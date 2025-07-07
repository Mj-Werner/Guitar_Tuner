#include <arduinoFFT.h>
#include "Note_Mapping.h"

// === Constants ===
#define SENSOR_PIN A0
#define SAMPLE_SIZE 2048                    // Use 1024 or 2048 for balance
#define SAMPLE_FREQ 10000.0                  // Sampling frequency in Hz up to 40Kh
#define SAMPLE_PERIOD_US (1000000 / SAMPLE_FREQ)
#define AMPLITUDE_THRESHOLD 4               // Ignore weak signals
#define Tuning_Delay 300                    // Delay between each run

// === Variables ===
double samples[SAMPLE_SIZE];                // Real part
double fft[SAMPLE_SIZE];                    // Imaginary part
ArduinoFFT<double> FFT(samples, fft, SAMPLE_SIZE, SAMPLE_FREQ);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Nano ESP32 Frequency Detector Starting...");
}

void loop() {
  // === Sample with fixed timing ===
  unsigned long t0 = micros();
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    while ((micros() - t0) < i * SAMPLE_PERIOD_US);
    samples[i] = analogRead(SENSOR_PIN);
    fft[i] = 0;
  }

  // === Remove DC offset ===
  double mean = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) mean += samples[i];
  mean /= SAMPLE_SIZE;
  for (int i = 0; i < SAMPLE_SIZE; i++) samples[i] -= mean;

  // === Pre-emphasis filter to reduce harmonics ===
  for (int i = 1; i < SAMPLE_SIZE; i++) {
    samples[i] = 0.8 * samples[i - 1] + 0.2 * samples[i];
  }

  // === Amplitude filter ===
  double sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) sum += abs(samples[i]);
  if ((sum / SAMPLE_SIZE) < AMPLITUDE_THRESHOLD) {
    Serial.println("No strong signal detected.");
    delay(Tuning_Delay);
    return;
  }

  // === FFT processing ===
  FFT.windowing(FFTWindow::Blackman_Harris, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  // === Normalize FFT magnitudes ===
  double maxVal = 0;
  for (int i = 1; i < SAMPLE_SIZE / 2; i++) {
    if (fft[i] > maxVal) maxVal = fft[i];
  }
  if (maxVal > 0) {
    for (int i = 1; i < SAMPLE_SIZE / 2; i++) {
      fft[i] /= maxVal;
    }
  }

  // === Weight the spectrum to reduce harmonics ===
  for (int i = 1; i < SAMPLE_SIZE / 2; i++) {
    double weight = 1.0 - (double)i / (SAMPLE_SIZE / 2);  // Linear falloff
    fft[i] *= weight;
  }

  // === Peak detection ===
  int i_max = 1;
  double maxMag = fft[1];
  for (int i = 2; i < SAMPLE_SIZE / 2; i++) {
    if (fft[i] > maxMag) {
      i_max = i;
      maxMag = fft[i];
    }
  }

  // === Interpolation ===
  double delta = 0;
  if (i_max > 0 && i_max < SAMPLE_SIZE / 2 - 1) {
    double magL = fft[i_max - 1];
    double magC = fft[i_max];
    double magR = fft[i_max + 1];
    double denom = (magL - 2 * magC + magR);
    if (denom != 0)
      delta = 0.5 * (magL - magR) / denom;
  }

  double interpolatedFreq = (i_max + delta) * SAMPLE_FREQ / SAMPLE_SIZE;

  
  Serial.println(interpolatedFreq);
  //printNoteInfo(interpolatedFreq, getClosestNote(interpolatedFreq));

  delay(Tuning_Delay);
}
