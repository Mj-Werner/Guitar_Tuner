#include <arduinoFFT.h>
extern "C" {
#include "user_interface.h"  // Disable Wi-Fi on ESP8266
}
#include <Note_Mapping.h>

/*
Sample Chart for 2048 samples
Delay between samples| sampling freq | time taken
        120                 6145          .33
        150                 5188          .39
        200                 4120          .50

Sample Chart for 1024 samples
Delay between samples| sampling freq | time taken
        120                 6144          .17
        200                 4120          .25
        300                 2917          .35

Sample Chart for 512 samples
Delay between samples| sampling freq | time taken
        300                 2917          .18
        400                 2258          .23
        500                 1843          .28

Sample Chart for 248 samples
Delay between samples| sampling freq | time taken
        500                 1843          .14
        550                 1687          .15
*/


// Constants
#define SENSOR_PIN A0     // Analog input from piezo
#define SAMPLE_SIZE 1024  // Must be power of 2
#define SAMPLE_Delay 120  //Refer to chart for Sample Freq
#define num_AVG 3         //Number of times to average to get the SAMPLE_FREQ

//Variables
double SAMPLE_FREQ;           // Sampling frequency in Hz
double samples[SAMPLE_SIZE];  // Real part
double fft[SAMPLE_SIZE];      // Imaginary part

// FFT object
ArduinoFFT<double> FFT; 

// Optional: scaling modes for printing
//#define SCL_INDEX 0x00
//#define SCL_TIME 0x01
//#define SCL_FREQUENCY 0x02

void setup() {
  wifi_set_opmode(NULL_MODE);  // Disable Wi-Fi to reduce analog noise
  Serial.begin(115200);
  while (!Serial)
    ;

  //Get the sample FREQ to be used for this upload
  SAMPLE_FREQ = getSampleFreq(num_AVG);
  FFT = ArduinoFFT<double>(samples, fft, SAMPLE_SIZE, SAMPLE_FREQ);
}

void loop() {
  //Read in the raw data
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    samples[i] = analogRead(SENSOR_PIN);  // Raw ADC input
    fft[i] = 0;
    //Serial.println(samples[i]);
    delayMicroseconds(SAMPLE_Delay);
  }
  //FFT = ArduinoFFT<double>(samples, fft, SAMPLE_SIZE, SAMPLE_FREQ);
  /*
  // Compute average signal amplitude
  double sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    sum += abs(samples[i]);
  }
  double avgSignal = sum / SAMPLE_SIZE;

  // Ignore low-energy (noise-only) signals
  if (avgSignal < 5) {
    Serial.println("No strong signal detected.");
    delay(300);
    return;
  }
*/
  // Proceed with FFT
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  double freq = FFT.majorPeak();
  if (freq > 0.0) {
    Serial.print("Detected Frequency: ");
    Serial.print(freq, 2);
    Serial.println(" Hz");
  } else {
    Serial.println("No dominant frequency found.");
  }

  //delay(300);
}

//Will get the sampling Frequency for the current upload
double getSampleFreq(int numAverage) {
  float samplingFrequency[numAverage];
  for (int j = 0; j < 3; j++) {
    unsigned long startTime = micros();
    for (int i = 0; i < SAMPLE_SIZE; i++) {
      samples[i] = analogRead(SENSOR_PIN);  // Raw ADC input
      fft[i] = 0;
      delayMicroseconds(SAMPLE_Delay);
    }
    unsigned long endTime = micros();
    unsigned long totalTime = endTime - startTime;
    samplingFrequency[j] = (float)SAMPLE_SIZE / (totalTime / 1e6);
    delay(300);
  }
  return (samplingFrequency[0] + samplingFrequency[1] + samplingFrequency[2]) / numAverage;
}