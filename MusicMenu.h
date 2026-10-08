#ifndef MUSIC_MENU_H
#define MUSIC_MENU_H

#include <Adafruit_SSD1306.h>
#include "SongHappyBirthday.h"
#include "SongJingleBells.h"

struct MusicItem {
  const char* name;
  void (*updateFunc)(int buzzerPin, bool isPlaying);
  void (*resetFunc)();
};

void updateEmpty(int buzzerPin, bool isPlaying) {
  noTone(buzzerPin);
}
void resetEmpty() {}

MusicItem musicList[] = {
  {"1. Happy Birthday", updateHappyBirthday, resetHappyBirthday},
  {"2. Jingle Bells",   updateJingleBells,  resetJingleBells},
  {"3. Tat Nhac",       updateEmpty,          resetEmpty}
};

const int totalMusicItems = sizeof(musicList) / sizeof(musicList[0]);
int musicSelection = 0;

bool inPlayerScreen = false;
bool isPlaying = false;

// Thêm bool bBtn vào danh sách tham số nhận vào
void runMusicMenu(Adafruit_SSD1306 &display, int buzzerPin, bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, bool &inMusicMenu) {
  
  // A. NẾU ĐANG Ở MÀN HÌNH PLAYER CHI TIẾT
  if (inPlayerScreen) {
    musicList[musicSelection].updateFunc(buzzerPin, isPlaying);

    if (bBtn) {
      // Bấm nút B để thoát ra ngoài danh sách nhạc
      tone(buzzerPin, 1000, 50);
      isPlaying = false;
      noTone(buzzerPin);
      inPlayerScreen = false; // Thoát khỏi màn hình player về lại danh sách
      delay(200);
      return;
    }
    else if (left) {
      musicList[musicSelection].resetFunc();
      noTone(buzzerPin);
      musicSelection--;
      if (musicSelection < 0) musicSelection = totalMusicItems - 1;
      tone(buzzerPin, 800, 30);
      isPlaying = true;
      delay(200);
    } 
    else if (right) {
      musicList[musicSelection].resetFunc();
      noTone(buzzerPin);
      musicSelection++;
      if (musicSelection >= totalMusicItems) musicSelection = 0;
      tone(buzzerPin, 800, 30);
      isPlaying = true;
      delay(200);
    } 
    else if (aBtn) {
      tone(buzzerPin, 1200, 50);
      isPlaying = !isPlaying;
      if (!isPlaying) {
        noTone(buzzerPin);
      }
      delay(200);
    }

    // Vẽ giao diện màn hình Player
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, 2);
    display.print(musicList[musicSelection].name);
    display.drawLine(0, 14, 128, 14, SSD1306_WHITE);

    display.drawCircle(64, 38, 16, SSD1306_WHITE);
    if (isPlaying) {
      display.fillTriangle(60, 30, 60, 46, 72, 38, SSD1306_WHITE);
    } else {
      display.fillRect(59, 31, 3, 14, SSD1306_WHITE);
      display.fillRect(66, 31, 3, 14, SSD1306_WHITE);
    }

    display.setTextSize(2);
    display.setCursor(15, 30);
    display.print(F("<"));
    display.setCursor(102, 30);
    display.print(F(">"));

    display.setTextSize(1);
    display.setCursor(20, 55);
    if (isPlaying) {
      display.print(F("Status: Playing"));
    } else {
      display.print(F("Status: Paused"));
    }

    display.display();
    return;
  }

  // B. NẾU ĐANG Ở DANH SÁCH CHÍNH (MUSIC LIST)
  noTone(buzzerPin);
  isPlaying = false;

  if (up) {
    musicSelection--;
    if (musicSelection < 0) musicSelection = totalMusicItems - 1;
    tone(buzzerPin, 800, 30);
    delay(150);
  } else if (down) {
    musicSelection++;
    if (musicSelection >= totalMusicItems) musicSelection = 0;
    tone(buzzerPin, 800, 30);
    delay(150);
  } else if (left || right) {
    tone(buzzerPin, 1000, 50);
    inMusicMenu = false;
    delay(200);
    return;
  } else if (aBtn) {
    tone(buzzerPin, 1500, 80);
    inPlayerScreen = true;
    isPlaying = true;
    musicList[musicSelection].resetFunc();
    delay(200);
  }

  // Vẽ giao diện danh sách nhạc (với các tinh chỉnh căn lề của bạn)
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 0);
  display.println(F("== KHO TANG NHAC =="));
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  int visibleItems = 4;
  int startIndex = 0;
  if (musicSelection >= visibleItems) {
    startIndex = musicSelection - visibleItems + 1;
  }

  int startY = 12; 
  int rowHeight = 10; 

  for (int i = 0; i < visibleItems; i++) {
    int itemIndex = startIndex + i;
    if (itemIndex >= totalMusicItems) break;

    int yPos = startY + (i * rowHeight);

    if (itemIndex == musicSelection) {
      display.setCursor(2, yPos);
      display.print(F(">"));
      display.setCursor(12, yPos);
      display.print(musicList[itemIndex].name);
    } else {
      display.setCursor(12, yPos);
      display.print(musicList[itemIndex].name);
    }
  }

  display.setCursor(0, 57);
  display.print(F("L/R:Doi Menu A:Chon"));
  
  display.display();
}

#endif