#define ANALOG_PIN A0

#define TOTAL_PEAKS 10
#define USE_LAST_N 8

#define PEAK_THRESHOLD 22
#define IGNORE_TIME 1000UL
#define PERIOD_MIN 2500UL
#define PERIOD_MAX 18000UL
#define TIMEOUT 600000UL

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
  Serial.println("Accurate Guitar Tuner (stabilized)");
}

void loop() {
  unsigned long periods[TOTAL_PEAKS];
  int peakCount = 0;
  int lastVal = 0;
  unsigned long lastPeakTime = 0;
  unsigned long lastAcceptedPeakTime = 0;
  unsigned long startTime = micros();

  // Wait for signal rise
  while (analogRead(ANALOG_PIN) < PEAK_THRESHOLD) {
    if (micros() - startTime > TIMEOUT) {
      Serial.println("No signal...");
      delay(300);
      return;
    }
  }

  // Optional: let pluck settle
  delayMicroseconds(15000);  // 15ms to avoid pluck transients

  while (peakCount < TOTAL_PEAKS) {
    int val = analogRead(ANALOG_PIN);
    //Serial.println(val);
    unsigned long now = micros();

    if (val >= PEAK_THRESHOLD && lastVal < PEAK_THRESHOLD) {
      if (now - lastAcceptedPeakTime >= IGNORE_TIME) {
        if (lastPeakTime > 0) {
          unsigned long period = now - lastPeakTime;
          if (period >= PERIOD_MIN && period <= PERIOD_MAX) {
            periods[peakCount++] = period;
          }
        }
        lastPeakTime = now;
        lastAcceptedPeakTime = now;
      }
    }

    lastVal = val;

    if (micros() - startTime > TIMEOUT) {
      Serial.println("Timed out...");
      return;
    }
  }

  // Use only the last N periods
  int startIndex = TOTAL_PEAKS - USE_LAST_N;
  sortArray(&periods[startIndex], USE_LAST_N);
  unsigned long medianPeriod = periods[startIndex + USE_LAST_N / 2];

  float frequency = 1000000.0 / medianPeriod;
 // Serial.print("Frequency: ");
  Serial.println(frequency);
 // Serial.println(" Hz");

  delay(300);
}
