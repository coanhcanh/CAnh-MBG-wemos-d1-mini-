#ifndef SONG_JINGLE_BELLS_H
#define SONG_JINGLE_BELLS_H

#ifndef NOTE_C4
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#endif

struct JingleNote {
  int frequency;
  int duration;
};

// Giai điệu Jingle Bells được tinh chỉnh thời lượng ngắn hơn để nhạc nhanh hơn
const JingleNote jingleMelody[] = {
  // Jingle bells, jingle bells, jingle all the way
  {NOTE_E4, 180}, {NOTE_E4, 180}, {NOTE_E4, 360},
  {NOTE_E4, 180}, {NOTE_E4, 180}, {NOTE_E4, 360},
  {NOTE_E4, 180}, {NOTE_G4, 180}, {NOTE_C4, 280}, {NOTE_D4, 100}, {NOTE_E4, 560},
  
  // Oh, what fun it is to ride in a one-horse open sleigh
  {NOTE_F4, 180}, {NOTE_F4, 180}, {NOTE_F4, 280}, {NOTE_F4, 100},
  {NOTE_F4, 180}, {NOTE_E4, 180}, {NOTE_E4, 180}, {NOTE_E4, 90}, {NOTE_E4, 90},
  {NOTE_E4, 180}, {NOTE_D4, 180}, {NOTE_D4, 180}, {NOTE_E4, 180}, {NOTE_D4, 360}, {NOTE_G4, 360},
  
  // Jingle bells, jingle bells, jingle all the way
  {NOTE_E4, 180}, {NOTE_E4, 180}, {NOTE_E4, 360},
  {NOTE_E4, 180}, {NOTE_E4, 180}, {NOTE_E4, 360},
  {NOTE_E4, 180}, {NOTE_G4, 180}, {NOTE_C4, 280}, {NOTE_D4, 100}, {NOTE_E4, 560},
  
  // Oh, what fun it is to ride in a one-horse open sleigh
  {NOTE_F4, 180}, {NOTE_F4, 180}, {NOTE_F4, 280}, {NOTE_F4, 100},
  {NOTE_F4, 180}, {NOTE_E4, 180}, {NOTE_E4, 180}, {NOTE_E4, 90}, {NOTE_E4, 90},
  {NOTE_G4, 180}, {NOTE_G4, 180}, {NOTE_F4, 180}, {NOTE_D4, 180}, {NOTE_C4, 720}
};

const int jingleTotalNotes = sizeof(jingleMelody) / sizeof(jingleMelody[0]);

int jingleNoteIndex = 0;
unsigned long jingleLastTime = 0;
bool jingleIsNotePlaying = false;

void updateJingleBells(int buzzerPin, bool isPlaying) {
  if (!isPlaying) {
    noTone(buzzerPin);
    jingleIsNotePlaying = false;
    return;
  }

  unsigned long currentMillis = millis();
  
  if (!jingleIsNotePlaying) {
    tone(buzzerPin, jingleMelody[jingleNoteIndex].frequency);
    jingleLastTime = currentMillis;
    jingleIsNotePlaying = true;
  } else {
    int currentDuration = jingleMelody[jingleNoteIndex].duration;
    if (currentMillis - jingleLastTime >= currentDuration) {
      noTone(buzzerPin); 
      
      if (currentMillis - jingleLastTime >= currentDuration * 1.25) {
        jingleNoteIndex++;
        if (jingleNoteIndex >= jingleTotalNotes) {
          jingleNoteIndex = 0;
        }
        jingleIsNotePlaying = false;
      }
    }
  }
}

void resetJingleBells() {
  jingleNoteIndex = 0;
  jingleIsNotePlaying = false;
}

#endif