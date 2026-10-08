#ifndef SONG_HAPPY_BIRTHDAY_H
#define SONG_HAPPY_BIRTHDAY_H

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

struct BirthdayNote {
  int frequency;
  int duration;
};

// Giai điệu Happy Birthday đầy đủ chi tiết và được đẩy nhanh tốc độ
const BirthdayNote birthdayMelody[] = {
  // Happy birthday to you (Dòng 1)
  {NOTE_C4, 180}, {NOTE_C4, 100}, {NOTE_D4, 280}, {NOTE_C4, 280}, {NOTE_F4, 280}, {NOTE_E4, 560},
  
  // Happy birthday to you (Dòng 2)
  {NOTE_C4, 180}, {NOTE_C4, 100}, {NOTE_D4, 280}, {NOTE_C4, 280}, {NOTE_G4, 280}, {NOTE_F4, 560},
  
  // Happy birthday dear [Name] (Dòng 3)
  {NOTE_C4, 180}, {NOTE_C4, 100}, {NOTE_C5, 280}, {NOTE_A4, 280}, {NOTE_F4, 280}, {NOTE_E4, 280}, {NOTE_D4, 560},
  
  // Happy birthday to you (Dòng 4 kết thúc)
  {NOTE_A4, 180}, {NOTE_A4, 100}, {NOTE_F4, 280}, {NOTE_G4, 280}, {NOTE_F4, 700}
};

const int birthdayTotalNotes = sizeof(birthdayMelody) / sizeof(birthdayMelody[0]);

int birthdayNoteIndex = 0;
unsigned long birthdayLastTime = 0;
bool birthdayIsNotePlaying = false;

// Hàm cập nhật nhạc Happy Birthday chạy ngầm không chặn
void updateHappyBirthday(int buzzerPin, bool isPlaying) {
  if (!isPlaying) {
    noTone(buzzerPin);
    birthdayIsNotePlaying = false;
    return;
  }

  unsigned long currentMillis = millis();
  
  if (!birthdayIsNotePlaying) {
    tone(buzzerPin, birthdayMelody[birthdayNoteIndex].frequency);
    birthdayLastTime = currentMillis;
    birthdayIsNotePlaying = true;
  } else {
    int currentDuration = birthdayMelody[birthdayNoteIndex].duration;
    if (currentMillis - birthdayLastTime >= currentDuration) {
      noTone(buzzerPin); // Tắt âm ngắn giữa các nốt
      
      if (currentMillis - birthdayLastTime >= currentDuration * 1.25) {
        birthdayNoteIndex++;
        if (birthdayNoteIndex >= birthdayTotalNotes) {
          birthdayNoteIndex = 0; // Lặp lại từ đầu khi chạy hết bài
        }
        birthdayIsNotePlaying = false;
      }
    }
  }
}

// Hàm reset tiến trình bài hát
void resetHappyBirthday() {
  birthdayNoteIndex = 0;
  birthdayIsNotePlaying = false;
}

#endif