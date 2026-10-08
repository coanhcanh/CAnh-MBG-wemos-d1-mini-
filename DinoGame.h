#ifndef DINO_GAME_H
#define DINO_GAME_H

// --- MẢNG BITMAP KHỦNG LONG CHUẨN (16x16 pixel) ---
const unsigned char PROGMEM dino_bmp[] = {
  0b00000000, 0b01111110,
  0b00000000, 0b11111111,
  0b00000000, 0b11111111,
  0b00000000, 0b11011111,
  0b00000000, 0b11111111,
  0b00000000, 0b01111000,
  0b00000011, 0b11111100,
  0b00001111, 0b11111100,
  0b00011111, 0b11110000,
  0b00111111, 0b11100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11000000,
  0b00011110, 0b00000000,
  0b00001100, 0b11000000,
  0b00001100, 0b11000000,
  0b00001100, 0b01100000
};

// --- MẢNG BITMAP CÂY XƯƠNG RỒNG (8x10 pixel) ---
const unsigned char PROGMEM cactus_bmp[] = {
  0b00110000,
  0b11110000,
  0b10111000,
  0b00110100,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000
};

// --- BIẾN QUẢN LÝ GAME DINO ---
int dinoY = 40;          
int dinoVy = 0;          
bool isJumping = false;  
int dinoScore = 0;       
bool dinoGameOver = false;

// Chướng ngại vật (Cây xương rồng 8x10)
int obstacleX = 128;     
int obstacleY = 46;      // Mặt đất ở y = 56, cao 10 -> 56 - 10 = 46
int obstacleW = 8;       
int obstacleH = 10;      
int gameSpeed = 3;       

unsigned long lastDinoUpdate = 0;
int dinoFrameRate = 30;  

void initDinoGame() {
  dinoScore = 0;
  dinoY = 40;
  dinoVy = 0;
  isJumping = false;
  dinoGameOver = false;
  obstacleX = 128;
  gameSpeed = 3;
  delay(200);
}

void resetDino() {
  dinoScore = 0;
  dinoY = 40;
  dinoVy = 0;
  isJumping = false;
  dinoGameOver = false;
  obstacleX = 128;
  gameSpeed = 3;
}

void runDinoGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inGame = false; 
    delay(200);
    return;
  }

  if (dinoGameOver) {
    if (up || down || aBtn) {
      tone(buzzerPin, 1200, 50);
      resetDino();
    }
    return;
  }

  if ((up || aBtn) && !isJumping) {
    dinoVy = -7;         
    isJumping = true;
    tone(buzzerPin, 1000, 40); 
  }

  if (millis() - lastDinoUpdate > dinoFrameRate) {
    lastDinoUpdate = millis();

    dinoY += dinoVy;
    dinoVy += 1;          

    if (dinoY >= 40) {
      dinoY = 40;
      isJumping = false;
      dinoVy = 0;
    }

    obstacleX -= gameSpeed;
    if (obstacleX < -15) {
      obstacleX = 128 + (rand() % 50); 
      dinoScore += 10;                 
      
      if (dinoScore % 50 == 0 && gameSpeed < 6) {
        gameSpeed++;
      }
    }

    // Kiểm tra va chạm theo kích thước mới
    int dinoX = 16;
    int dinoW = 14;
    int dinoH = 16;

    if (dinoX + dinoW > obstacleX && dinoX < obstacleX + obstacleW &&
        dinoY + dinoH > obstacleY && dinoY < obstacleY + obstacleH) {
      dinoGameOver = true;
      tone(buzzerPin, 200, 300); 
    }
  }

  // --- VẼ ĐỒ HỌA ---
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("Score: "));
  display.print(dinoScore);

  // Đường mặt đất
  display.drawLine(0, 56, 128, 56, SSD1306_WHITE);
  display.drawPixel(10, 58, SSD1306_WHITE);
  display.drawPixel(45, 59, SSD1306_WHITE);
  display.drawPixel(85, 58, SSD1306_WHITE);
  display.drawPixel(110, 59, SSD1306_WHITE);

  // Vẽ khủng long
  display.drawBitmap(16, dinoY, dino_bmp, 16, 16, SSD1306_WHITE);

  // Vẽ cây xương rồng (8x10)
  display.drawBitmap(obstacleX, obstacleY, cactus_bmp, 8, 10, SSD1306_WHITE);

  // Game Over Box
  if (dinoGameOver) {
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