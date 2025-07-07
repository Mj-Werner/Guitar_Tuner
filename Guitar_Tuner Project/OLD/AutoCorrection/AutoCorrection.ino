#define ANALOG_PIN A0
#define SAMPLE_COUNT 512              // Number of samples to capture
#define SAMPLING_RATE 5000            // Hz
#define MIN_FREQ 70                   // Lowest expected frequency (E2 ≈ 82 Hz)
#define MAX_FREQ 350                  // Highest expected frequency (E4 ≈ 330 Hz)

int16_t samples[SAMPLE_COUNT];

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Auto-Correlation Guitar Tuner");
}

void loop() {
  // Step 1: Sample the signal
  unsigned long sampleIntervalMicros = 1000000UL / SAMPLING_RATE;
  unsigned long lastSampleTime = micros();

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    while (micros() - lastSampleTime < sampleIntervalMicros);
    lastSampleTime += sampleIntervalMicros;

    samples[i] = analogRead(ANALOG_PIN) - 512;  // Center around 0
  }

  // Step 2: Auto-correlation
  int bestLag = 0;
  long bestCorrelation = 0;

  int minLag = SAMPLING_RATE / MAX_FREQ;
  int maxLag = SAMPLING_RATE / MIN_FREQ;

  for (int lag = minLag; lag <= maxLag; lag++) {
    long sum = 0;
    for (int i = 0; i < SAMPLE_COUNT - lag; i++) {
      sum += (long)samples[i] * samples[i + lag];
    }

    if (sum > bestCorrelation) {
      bestCorrelation = sum;
      bestLag = lag;
    }
  }

  // Step 3: Convert lag to frequency
  if (bestLag != 0) {
    float frequency = (float)SAMPLING_RATE / bestLag;
    Serial.print("Detected Frequency: ");
    Serial.print(frequency, 2);
    Serial.println(" Hz");
  } else {
    Serial.println("No stable frequency detected.");
  }

  delay(500);  // Wait before next detection
}
