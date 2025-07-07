#define ANALOG_PIN A0

// Detection config
#define NUM_CYCLES 15              // Collect 15 good cycles
#define PEAK_THRESHOLD 22        // ADC threshold to detect a peak
#define IGNORE_TIME 1000UL        // Ignore for 1000 µs after peak (debounce)
#define PERIOD_MIN 2500UL         // Accepts frequencies up to 400 Hz
#define PERIOD_MAX 15000UL        // Reject periods below ~66 Hz
#define TIMEOUT 800000UL          // Abort if it takes too long (800ms)

// Simple array sorter for median
void sortArray(unsigned long arr[], int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (arr[i] > arr[j]) {
        unsigned long temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Optimized Guitar Tuner");
}

void loop() {
  unsigned long periods[NUM_CYCLES];
  int cycleCount = 0;
  unsigned long lastPeakTime = 0;
  unsigned long lastAcceptedPeakTime = 0;
  int lastVal = 0;
  unsigned long startTime = micros();

  // Wait for signal above threshold
  while (analogRead(ANALOG_PIN) < PEAK_THRESHOLD) {
    if (micros() - startTime > TIMEOUT) {
      Serial.println("No signal detected.");
      delay(300);
      return;
    }
  }

  // Start peak detection loop
  while (cycleCount < NUM_CYCLES) {
    int val = analogRead(ANALOG_PIN);
    unsigned long now = micros();

    if (val >= PEAK_THRESHOLD && lastVal < PEAK_THRESHOLD) {
      if (now - lastAcceptedPeakTime >= IGNORE_TIME) {
        if (lastPeakTime > 0) {
          unsigned long period = now - lastPeakTime;

          if (period >= PERIOD_MIN && period <= PERIOD_MAX) {
            periods[cycleCount++] = period;
          }
        }
        lastPeakTime = now;
        lastAcceptedPeakTime = now;
      }
    }

    lastVal = val;

    if (micros() - startTime > TIMEOUT) {
      Serial.println("Timed out collecting peaks.");
      return;
    }
  }

  // Calculate median period
  sortArray(periods, NUM_CYCLES);
  unsigned long medianPeriod = periods[NUM_CYCLES / 2];
  float frequency = 1000000.0 / medianPeriod;

  // Output raw frequency
  Serial.print("Frequency: ");
  Serial.print(frequency, 2);
  Serial.println(" Hz");

  delay(300);  // Fast enough for live tuning feedback
}
