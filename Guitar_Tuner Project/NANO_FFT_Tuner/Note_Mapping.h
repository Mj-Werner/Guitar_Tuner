#ifndef NOTE_MAPPING_h
#define NOTE_MAPPING_h

struct GuitarNote {
  const char* name;
  float frequency;
};

GuitarNote getClosestNote(float freq);
void printNoteInfo(float inputFreq, GuitarNote result);

#endif
