#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GameEngine.h"
#include "SnakeGame.h"  
#include "Settings.h"   
#include "DinoGame.h"   
#include "BounceGame.h"   
#include "FlappyGame.h"   
#include "CarGame.h"  
#include "TetrisGame.h"  
#include "ContraGame.h" 
#include "PacmanGame.h" 
#include "PlaneGame.h"  

#include "MusicMenu.h" // Thêm menu nhạc
#include "Wifi.h"      // Thêm trang quản lý Wifi mới

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

// --- QUẢN LÝ 3 TRANG SONG SONG ---
// 0: Trang Menu Game (CAnh.ino)
// 1: Trang Kho Tàng Nhạc (MusicMenu.h)
// 2: Trang Quản Lý Wifi (Wifi.h)
int currentPage = 0; 

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
bool inMusicMenu = false; // Khai báo biến này để tương thích với MusicMenu.h

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150; 

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
  
  playMarioStartup();
  delay(500);
}

void loop() {
  unsigned long currentTime = millis();

  bool btnUp = (digitalRead(PIN_UP) == LOW);
  bool btnDown = (digitalRead(PIN_DOWN) == LOW);
  bool btnLeft = (digitalRead(PIN_LEFT) == LOW);
  bool btnRight = (digitalRead(PIN_RIGHT) == LOW);
  bool btnA = (digitalRead(PIN_A) == LOW);
  bool btnB = (digitalRead(PIN_B) == LOW);

  if (inGame) {
    gameList[currentSelection].runFunc(btnUp, btnDown, btnLeft, btnRight, btnA, btnB, display, PIN_BUZZER, inGame);
    return;
  }
  if (inSettings) {
    if (currentTime - lastDebounceTime > debounceDelay) {
      runSettings(display, PIN_BUZZER, btnUp, btnDown, btnB, inSettings);
      lastDebounceTime = currentTime;
    }
    return;
  }

  // --- QUẢN LÝ 3 TRANG CHÍNH BẰNG NÚT LEFT / RIGHT ---
  if (currentTime - lastDebounceTime > debounceDelay) {
    
    // TRANG 0: MENU TRÒ CHƠI
    if (currentPage == 0) {
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
      else if (btnRight) { // Sang trang Nhạc (Trang 1)
        tone(PIN_BUZZER, 1000, 50);
        currentPage = 1;
        lastDebounceTime = currentTime;
      }
      else if (btnLeft) { // Sang trang Wifi (Trang 2)
        tone(PIN_BUZZER, 1000, 50);
        currentPage = 2;
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
      drawMenuUI();
    }
    // TRANG 1: KHO TÀNG NHẠC
    else if (currentPage == 1) {
      if (btnRight) {
        tone(PIN_BUZZER, 1000, 50);
        currentPage = 2; // Sang trang Wifi
        lastDebounceTime = currentTime;
      } else if (btnLeft) {
        tone(PIN_BUZZER, 1000, 50);
        currentPage = 0; // Về trang Menu Game
        lastDebounceTime = currentTime;
      } else {
        runMusicMenu(display, PIN_BUZZER, btnUp, btnDown, btnLeft, btnRight, btnA, btnB, inMusicMenu);
      }
    }
    // TRANG 2: QUẢN LÝ WIFI
    else if (currentPage == 2) {
      runWifiMenu(display, PIN_BUZZER, btnUp, btnDown, btnLeft, btnRight, btnA, btnB, currentPage);
      lastDebounceTime = currentTime;
    }
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
  display.print(F("L/R: Doi Trang (1/3)"));
  display.display();
}