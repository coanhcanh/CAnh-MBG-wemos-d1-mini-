#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GameEngine.h"
#include "SnakeGame.h"  // Thêm file game mới ở đây
#include "Settings.h"   // File Cài đặt
#include "DinoGame.h"   //game khủng long
#include "BounceGame.h"   //game quả bóng
#include "FlappyGame.h"   //game flappy bird
#include "CarGame.h"  // Thêm game đua xe
#include "TetrisGame.h"  //game xếp hình
#include "ContraGame.h" // Thêm game Contra
#include "PacmanGame.h" //game Pacman
#include "PlaneGame.h"  //game bắn máy bay

#include "MusicMenu.h" // Thêm menu nhạc

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int PIN_UP    = D5;  
const int PIN_LEFT  = D6;  
const int PIN_DOWN  = D4;  
const int PIN_RIGHT = D7;  
const int PIN_A     = D3;  
const int PIN_B     = 3;   
const int PIN_BUZZER = D8;

unsigned long globalBLastPressTime = 0;
int globalBPressCount = 0;
bool globalBWasPressed = false;

// Hàm kiểm tra bấm phím B 3 lần liên tiếp (khoảng cách dưới 0.5 giây)
bool checkGlobalExitGame(bool bBtn, int buzzerPin) {
  unsigned long now = millis();
  if (bBtn) {
    if (!globalBWasPressed) {
      globalBWasPressed = true;
      if (now - globalBLastPressTime < 500) { 
        globalBPressCount++;
      } else {
        globalBPressCount = 1; 
      }
      globalBLastPressTime = now;
      tone(buzzerPin, 600, 50); // Tiếng kêu nhỏ mỗi lần bấm phím B
    }
  } else {
    globalBWasPressed = false;
  }

  if (globalBPressCount >= 3) {
    globalBPressCount = 0; 
    tone(buzzerPin, 1000, 100); // Tiếng kêu xác nhận thoát game
    return true; 
  }
  return false;
}

// --- DANH SÁCH CÁC TRÒ CHƠI VÀ CÀI ĐẶT ---
GameItem gameList[] = {
  { "1. Ran San Moi", initSnakeGame, runSnakeGame },
  { "2. Dino Runner", initDinoGame, runDinoGame },
  { "3. Bounce Classic", initBounceGame, runBounceGame },
  { "4. Flappy Bird", initFlappyGame, runFlappyGame },
  { "5. Dua Xe Oto", initCarGame, runCarGame },
  { "6. Xep Hinh Tetris", initTetrisGame, runTetrisGame },
  { "7. Contra Soldier", initContraGame, runContraGame },
  { "8. Ban May Bay", initPlaneGame, runPlaneGame },
  { "9. Pacman", initPacmanGame, runPacmanGame },
  { "10. Cai Dat", [](){}, [](bool u, bool d, bool l, bool r, bool a, bool b, Adafruit_SSD1306 &disp, int buz, bool &ing){} }
};
const int totalItems = sizeof(gameList) / sizeof(gameList[0]);

int currentSelection = 0; 
bool inGame = false;      
bool inSettings = false;  
bool inMusicMenu = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150; 

// --- ĐỊNH NGHĨA TẦN SỐ CÁC NỐT NHẠC (Mario Theme) ---
#define NOTE_E7  2637
#define NOTE_C7  2093
#define NOTE_G7  3136
#define NOTE_G6  1568
void playMarioStartup() {
  int notes[] = { NOTE_E7, NOTE_E7, 0, NOTE_E7, 0, NOTE_C7, NOTE_E7, 0, NOTE_G7, 0, NOTE_G6 };
  int durations[] = { 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120 };

  for (int i = 0; i < 11; i++) {
    if (notes[i] == 0) {
      noTone(PIN_BUZZER);
    } else {
      tone(PIN_BUZZER, notes[i], durations[i]);
    }
    delay(durations[i] * 1.3);
  }
  noTone(PIN_BUZZER);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Không tìm thấy màn hình OLED!"));
    for(;;);
  }

  // --- HIỂN THỊ MÀN HÌNH CHÀO MỪNG ĐÃ ĐƯỢC KHÔI PHỤC ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(16,16);
  display.println(F("Cong Anh"));
  display.setTextSize(1);
  display.setCursor(31,40);
  display.println(F("mobile game"));
  display.display();
  
  // Phát nhạc nền Mario khi khởi động
  playMarioStartup();
  
  delay(500); // Dừng lại một chút trước khi vào menu
}

void loop() {
  unsigned long currentTime = millis();

  bool btnUp = (digitalRead(PIN_UP) == LOW);
  bool btnDown = (digitalRead(PIN_DOWN) == LOW);
  bool btnLeft = (digitalRead(PIN_LEFT) == LOW);
  bool btnRight = (digitalRead(PIN_RIGHT) == LOW);
  bool btnA = (digitalRead(PIN_A) == LOW);
  bool btnB = (digitalRead(PIN_B) == LOW);

  // 1. Đang ở Menu Game chính
  if (!inGame && !inSettings && !inMusicMenu) {
    if (currentTime - lastDebounceTime > debounceDelay) {
      if (btnUp) {
        currentSelection--;
        if (currentSelection < 0) currentSelection = totalItems - 1;
        tone(PIN_BUZZER, 800, 30);
        lastDebounceTime = currentTime;
      } 
      else if (btnDown) {
        currentSelection++;
        if (currentSelection >= totalItems) currentSelection = 0;
        tone(PIN_BUZZER, 800, 30);
        lastDebounceTime = currentTime;
      } 
      // Bấm LEFT hoặc RIGHT để chuyển sang Menu Nhạc
      else if (btnLeft || btnRight) {
        tone(PIN_BUZZER, 1000, 50);
        inMusicMenu = true; 
        lastDebounceTime = currentTime;
      }
      else if (btnA) {
        tone(PIN_BUZZER, 1500, 80);
        lastDebounceTime = currentTime;
        
        if (currentSelection == totalItems - 1) { 
          inSettings = true; 
        } 
        else {
          inGame = true;     
          gameList[currentSelection].initFunc(); 
        }
      }
    }
    drawMenuUI();
  } 
  // 2. Đang ở trong Menu Nhạc
  else if (inMusicMenu) {
    if (currentTime - lastDebounceTime > debounceDelay) {
      // Truyền đủ tất cả các nút vào hàm runMusicMenu để tự xử lý trong file MusicMenu.h
      runMusicMenu(display, PIN_BUZZER, btnUp, btnDown, btnLeft, btnRight, btnA, btnB, inMusicMenu);
      lastDebounceTime = currentTime;
    }
  }
  // 3. Đang ở Cài đặt
  else if (inSettings) {
    if (currentTime - lastDebounceTime > debounceDelay) {
      runSettings(display, PIN_BUZZER, btnUp, btnDown, btnB, inSettings);
      lastDebounceTime = currentTime;
    }
  }
  // 4. Đang chơi Game
  else {
    gameList[currentSelection].runFunc(btnUp, btnDown, btnLeft, btnRight, btnA, btnB, display, PIN_BUZZER, inGame);
  }
}

void drawMenuUI() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 0);
  display.println(F("== CHON TRO CHOI =="));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  int startIndex = 0;
  if (currentSelection >= 3) {
    startIndex = currentSelection - 2;
  }
  
  int yPos = 13;
  for (int i = startIndex; i < startIndex + 4 && i < totalItems; i++) {
    if (i == currentSelection) {
      display.setCursor(2, yPos); 
      display.print(F(">"));
      display.setCursor(12, yPos); 
      display.println(gameList[i].name);
    } else {
      display.setCursor(12, yPos); 
      display.println(gameList[i].name);
    }
    yPos += 10;
  }

  display.setCursor(0, 57);
  display.print(F("U/D:Chon  A:Vao"));
  display.display();
}