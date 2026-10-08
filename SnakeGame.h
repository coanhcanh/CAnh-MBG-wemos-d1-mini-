#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

// --- KHAI BÁO NGUYÊN MẪU HÀM TRƯỚC ---
void spawnFood();

// --- BIẾN RIÊNG CHO GAME RẮN SĂN MỒI ---
const int SNAKE_MAX_LEN = 60;
int snakeX[SNAKE_MAX_LEN];
int snakeY[SNAKE_MAX_LEN];
int snakeLen;
int dirX, dirY; 
int foodX, foodY;
bool isBigFood = false; 
unsigned long lastSnakeMove = 0;
int snakeSpeed = 200;   
bool isGameOver = false;
int snakeScore = 0;

int snakeState = 0; 
int speedSelection = 1; 
int gameMode = 0;       

void initSnakeGame() {
  snakeState = 0;       
  speedSelection = 1;
  gameMode = 0;
  delay(200);     
}

void startActualGame() {
  snakeLen = 3;
  snakeX[0] = 32; snakeY[0] = 36;
  snakeX[1] = 28; snakeY[1] = 36;
  snakeX[2] = 24; snakeY[2] = 36;
  
  dirX = 4; 
  dirY = 0;
  snakeScore = 0;
  isGameOver = false;
  isBigFood = false;

  if (speedSelection == 0) snakeSpeed = 300;      
  else if (speedSelection == 1) snakeSpeed = 180; 
  else snakeSpeed = 100;                          

  spawnFood();
}

void spawnFood() {
  bool valid = false;
  while (!valid) {
    foodX = (rand() % 31) * 4;
    foodY = (rand() % 12) * 4 + 16;
    
    valid = true;
    for (int i = 0; i < snakeLen; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        valid = false;
        break;
      }
    }
  }
  isBigFood = (rand() % 4 == 0);
}

void runSnakeGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  if (snakeState == 0) {
    if (up) {
      speedSelection--;
      if (speedSelection < 0) speedSelection = 2;
      tone(buzzerPin, 800, 30);
      delay(150);
    }
    else if (down) {
      speedSelection++;
      if (speedSelection > 2) speedSelection = 0;
      tone(buzzerPin, 800, 30);
      delay(150);
    }
    else if (aBtn) {
      tone(buzzerPin, 1500, 80);
      snakeState = 1; 
      delay(300);     
      return;
    }
    else if (bBtn) {
      tone(buzzerPin, 500, 80);
      inGame = false; 
      delay(200);
      return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 2);
    display.println(F("CHON TOC DO RAN"));
    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

    String speeds[3] = {"1. Cham (Slow)", "2. Vua (Normal)", "3. Nhanh (Fast)"};
    int y = 20;
    for (int i = 0; i < 3; i++) {
      if (i == speedSelection) {
        display.setCursor(4, y); display.print(F(">"));
        display.setCursor(14, y); display.println(speeds[i]);
      } else {
        display.setCursor(14, y); display.println(speeds[i]);
      }
      y += 12;
    }
    display.setCursor(0, 56);
    display.print(F("U/D:Chon A:Tiep tuc"));
    display.display();
    return;
  }

  if (snakeState == 1) {
    if (up || down) {
      gameMode = 1 - gameMode; 
      tone(buzzerPin, 800, 30);
      delay(150);
    }
    else if (aBtn) {
      tone(buzzerPin, 1500, 80);
      snakeState = 2; 
      startActualGame();
      delay(200);
      return;
    }
    else if (bBtn) {
      tone(buzzerPin, 500, 80);
      snakeState = 0; 
      delay(200);
      return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 2);
    display.println(F("CHON CHE DO CHOI"));
    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

    String modes[2] = {"1. Xuyen tuong", "2. Tuong chan"};
    int y = 24;
    for (int i = 0; i < 2; i++) {
      if (i == gameMode) {
        display.setCursor(4, y); display.print(F(">"));
        display.setCursor(14, y); display.println(modes[i]);
      } else {
        display.setCursor(14, y); display.println(modes[i]);
      }
      y += 16;
    }
    display.setCursor(0, 56);
    display.print(F("U/D:Chon A:Bat dau"));
    display.display();
    return;
  }

  if (bBtn) {
    tone(buzzerPin, 500, 80);
    snakeState = 1; 
    return;
  }

  if (isGameOver) {
    if (up || down || left || right || aBtn) {
      startActualGame(); 
    }
    return;
  }

  if (up && dirY == 0) { dirX = 0; dirY = -4; }
  else if (down && dirY == 0) { dirX = 0; dirY = 4; }
  else if (left && dirX == 0) { dirX = -4; dirY = 0; }
  else if (right && dirX == 0) { dirX = 4; dirY = 0; }

  if (millis() - lastSnakeMove > snakeSpeed) {
    lastSnakeMove = millis();

    for (int i = snakeLen - 1; i > 0; i--) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }

    snakeX[0] += dirX;
    snakeY[0] += dirY;

    if (gameMode == 0) {
      if (snakeX[0] < 0) snakeX[0] = 124;
      else if (snakeX[0] >= 128) snakeX[0] = 0;

      if (snakeY[0] < 12) snakeY[0] = 60;
      else if (snakeY[0] >= 64) snakeY[0] = 12;
    } 
    else {
      if (snakeX[0] < 0 || snakeX[0] >= 128 || snakeY[0] < 12 || snakeY[0] >= 64) {
        isGameOver = true;
        tone(buzzerPin, 200, 300);
      }
    }

    for (int i = 1; i < snakeLen; i++) {
      if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
        isGameOver = true;
        tone(buzzerPin, 200, 300);
      }
    }

    int foodSize = isBigFood ? 6 : 3;
    if (snakeX[0] >= foodX && snakeX[0] < foodX + foodSize &&
        snakeY[0] >= foodY && snakeY[0] < foodY + foodSize) {
      
      int points = isBigFood ? 30 : 10;
      snakeScore += points;
      tone(buzzerPin, isBigFood ? 2200 : 1800, 60);
      
      if (snakeLen < SNAKE_MAX_LEN) snakeLen++;
      spawnFood();
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Score:"));
  display.print(snakeScore);
  
  display.setCursor(72, 0);
  display.print(gameMode == 0 ? F("[Xuyen]") : F("[Chan]"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  if (isBigFood) display.fillRect(foodX, foodY, 6, 6, SSD1306_WHITE);
  else display.fillRect(foodX, foodY, 3, 3, SSD1306_WHITE);

  for (int i = 0; i < snakeLen; i++) {
    display.fillRect(snakeX[i], snakeY[i], 3, 3, SSD1306_WHITE);
  }

  if (isGameOver) {
    display.setTextSize(1); // Đặt lại size mặc định trước để vẽ khung
    display.fillRect(2, 13, 124, 50, SSD1306_BLACK);  // Khung nền đen to hơn
    display.drawRect(2, 11, 124, 52, SSD1306_WHITE);  // Viền khung trắng

    // Dòng 1: Chữ GAME OVER! size 2
    display.setTextSize(2);                         
    display.setCursor(8, 22);                       // Căn chỉnh chữ size 2 ở giữa khung
    display.print(F("GAME OVER!"));

    // Dòng 2: Chữ hướng dẫn size 1 bên dưới
    display.setTextSize(1);                         
    display.setCursor(14, 46);                      // Đặt dòng chữ "Bấm phím chơi lại" phía dưới
    display.print(F("Bam phim choi lai"));
  }

  display.display();
}

#endif