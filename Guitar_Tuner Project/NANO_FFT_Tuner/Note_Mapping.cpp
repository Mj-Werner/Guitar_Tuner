#include "Note_Mapping.h"
#include <Arduino.h>  // For Serial
#include <math.h>     // For fabs()
/*
+---------------------------------------------------------------------------------------------+
| 6th String | 5th String | 4th String | 3rd String | 2nd String | 1st String                |
|   E=82     |   A=110    |   D=147    |   G=196    |   B=247    |   E=330       Standard    |
| F=87       | A#=117     | D#=156     | G#=208     | C=262      | F=349                     |
| F#=93      | B=124      | E=165      | A=220      | C#=278     | F#=370                    |
| G=98       | C=131      | F=175      | A#=233     | D=294      | G=392                     |
| G#=104     | C#=139     | F#=185     | B=247      | D#=311     | G#=415                    |
| A=110      | D=147      | G=196      | C=262      | E=330      | A=440                     |
| A#=117     | D#=156     | G#=208     | C#=278     | F=349      | A#=466                    |
| B=124      | E=165      | A=220      | D=294      | F#=370     | B=494                     |
| C=131      | F=175      | A#=233     | D#=311     | G=392      | C=523                     |
| C#=139     | F#=185     | B=247      | E=330      | G#=415     | C#=554                    |
| D=147      | G=196      | C=262      | F=349      | A=440      | D=587                     |
| D#=156     | G#=208     | C#=278     | F#=370     | A#=466     | D#=622                    |
| E=165      | A=220      | D=294      | G=392      | B=494      | E=659                     |
+---------------------------------------------------------------------------------------------+

*/

const GuitarNote noteTable[] = {
  {"E2", 82.0},   {"F2", 87.0},   {"F#2", 93.0},  {"G2", 98.0},   {"G#2", 104.0}, {"A2", 110.0},
  {"A#2", 117.0}, {"B2", 124.0},  {"C3", 131.0},  {"C#3", 139.0}, {"D3", 147.0},  {"D#3", 156.0},
  {"E3", 165.0},  {"F3", 175.0},  {"F#3", 185.0}, {"G3", 196.0},  {"G#3", 208.0}, {"A3", 220.0},
  {"A#3", 233.0}, {"B3", 247.0},  {"C4", 262.0},  {"C#4", 278.0}, {"D4", 294.0},  {"D#4", 311.0},
  {"E4", 330.0},  {"F4", 349.0},  {"F#4", 370.0}, {"G4", 392.0},  {"G#4", 415.0}, {"A4", 440.0},
  {"A#4", 466.0}, {"B4", 494.0},  {"C5", 523.0},  {"C#5", 554.0}, {"D5", 587.0},  {"D#5", 622.0},
  {"E5", 659.0}
};

const int numNotes = sizeof(noteTable) / sizeof(noteTable[0]);

// Given a frequency, return the closest guitar note
GuitarNote getClosestNote(float freq) {
  float minDiff = 99999;
  const char* closestNote = "";
  for (int i = 0; i < numNotes; i++) {
    float diff = freq - noteTable[i].frequency;
    if (abs(diff) < abs(minDiff)) {
      minDiff = diff;
      closestNote = noteTable[i].name;
    }
  }
  return {closestNote, minDiff} ;
}

// === Print note tuning info ===
//result holds the {closest note as name, difference as Freq}
void printNoteInfo(float inputFreq, GuitarNote result) {
  Serial.print("Input Frequency: ");
  Serial.print(inputFreq, 2);
  Serial.print(" Hz -> Closest Note: ");
  Serial.print(result.name);
  Serial.print(" (");
  Serial.print((result.frequency >= 0 ? "+" : ""));
  Serial.print(result.frequency, 2);
  Serial.println(" Hz)");

//Tell what direction to tune
  if (result.frequency < -0.5)
    Serial.println("🡅 Tune UP");
  else if (result.frequency > 0.5)
    Serial.println("🡇 Tune DOWN");
  else
    Serial.println("✓ In Tune");
}
