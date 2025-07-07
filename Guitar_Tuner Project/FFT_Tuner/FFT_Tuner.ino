#include <arduinoFFT.h>
extern "C" {
  #include "user_interface.h" // Disable Wi-Fi
}

// === Constants ===
#define SENSOR_PIN A0
#define SAMPLE_SIZE 2048              // Must be power of 2
#define SAMPLE_FREQ 5000.0           // Fixed sample frequency in Hz
#define SAMPLE_PERIOD_US (1000000 / SAMPLE_FREQ)
#define AMPLITUDE_THRESHOLD 4         // Ignore weak signals
#define Tuning_Delay 200              //delay between each run of the code

// === Variables ===
double samples[SAMPLE_SIZE];          // Real part
double fft[SAMPLE_SIZE];              // Imaginary part
ArduinoFFT<double> FFT(samples, fft, SAMPLE_SIZE, SAMPLE_FREQ);

void setup() {
  wifi_set_opmode(NULL_MODE);         // Disable Wi-Fi to reduce analog noise
  Serial.begin(115200);
  while (!Serial);
  Serial.println("ESP8266 Frequency Detector Starting...");
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

  // === Amplitude filter ===
  double sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) sum += abs(samples[i]);
  if ((sum / SAMPLE_SIZE) < AMPLITUDE_THRESHOLD) {
    Serial.println("No strong signal detected.");
    delay(Tuning_Delay);
    return;
  }

  // === FFT ===
  FFT.windowing(FFTWindow::Blackman_Harris, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  // === Find peak magnitude index (ignoring DC) ===
  int i_max = 1;
  double maxMag = fft[1];
  for (int i = 2; i < SAMPLE_SIZE / 2; i++) {
    if (fft[i] > maxMag) {
      i_max = i;
      maxMag = fft[i];
    }
  }

  // === Interpolate to refine frequency estimate ===
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

  Serial.print("Detected Frequency: ");
  Serial.print(interpolatedFreq, 2);
  Serial.println(" Hz");

  delay(Tuning_Delay);
}