#ifndef BOUNCE_GAME_H
#define BOUNCE_GAME_H

// --- BIẾN QUẢN LÝ GAME BOUNCE VÔ TẬN (NHIỀU CHƯỚNG NGẠI VẬT & GAI) ---
int ballScreenX = 40;    // Vị trí X cố định của bóng trên màn hình
float ballXWorld = 40.0; // Tọa độ thực tế X của bóng trong thế giới
float ballY = 40.0;      // Vị trí Y của bóng
float ballVx = 0;        // Vận tốc ngang
float ballVy = 0;        // Vận tốc dọc
bool isBallJumping = false;
int bounceScore = 0;
bool bounceGameOver = false;

// Tọa độ thế giới cuộn màn hình
int worldScrollX = 0;    

// Cấu trúc Bục / Tường chặn 4 cạnh
struct Wall {
  int x; 
  int y; 
  int w; 
  int h; 
};

// Cấu trúc Gai nhọn đặt dưới đất
struct Spike {
  int x;
  int y;
};

// Tăng số lượng tường lên 7 cái
const int totalWalls = 7;
Wall walls[totalWalls] = {
  { 140, 32, 16, 16 },
  { 260, 24, 18, 24 },
  { 390, 34, 16, 14 },
  { 510, 20, 20, 28 },
  { 640, 30, 16, 18 },
  { 770, 26, 18, 22 },
  { 900, 32, 16, 16 }
};

// Tăng số lượng gai nhọn lên 7 cái
const int totalSpikes = 7;
Spike spikes[totalSpikes] = {
  { 200, 44 },
  { 330, 44 },
  { 450, 44 },
  { 580, 44 },
  { 710, 44 },
  { 840, 44 },
  { 970, 44 }
};

// Tăng số lượng vòng điểm (Rings) lên 6 cái
const int totalRings = 6;
int ringXPositions[totalRings] = { 170, 300, 430, 550, 680, 810 };
int ringYPositions[totalRings] = { 20, 14, 22, 12, 18, 16 };
bool ringCollected[totalRings] = { false, false, false, false, false, false };

unsigned long lastBounceUpdate = 0;
int bounceFrameRate = 30; 

void initBounceGame() {
  bounceScore = 0;
  ballScreenX = 40;
  ballXWorld = 40.0;
  ballY = 40.0;
  ballVx = 0;
  ballVy = 0;
  isBallJumping = false;
  bounceGameOver = false;
  worldScrollX = 0;
  
  // Thiết lập lại vị trí ban đầu đan xen dày đặc
  walls[0] = { 140, 32, 16, 16 }; spikes[0] = { 200, 44 };
  walls[1] = { 260, 24, 18, 24 }; spikes[1] = { 330, 44 };
  walls[2] = { 390, 34, 16, 14 }; spikes[2] = { 450, 44 };
  walls[3] = { 510, 20, 20, 28 }; spikes[3] = { 580, 44 };
  walls[4] = { 640, 30, 16, 18 }; spikes[4] = { 710, 44 };
  walls[5] = { 770, 26, 18, 22 }; spikes[5] = { 840, 44 };
  walls[6] = { 900, 32, 16, 16 }; spikes[6] = { 970, 44 };

  for(int i = 0; i < totalRings; i++) {
    ringCollected[i] = false;
  }
  delay(200);
}

void resetBounce() {
  initBounceGame();
}

void runBounceGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  // Nút B: Thoát về Menu chính
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inGame = false; 
    delay(200);
    return;
  }

  // Nếu Game Over, bấm nút bất kỳ để chơi lại
  if (bounceGameOver) {
    if (up || down || left || right || aBtn) {
      tone(buzzerPin, 1200, 50);
      resetBounce();
    }
    return;
  }

  // Điều khiển di chuyển ngang trái / phải
  ballVx = 0;
  if (right) {
    ballVx = 3.0;
    bounceScore += 1;  
  }
  if (left) {
    ballVx = -3.0;
  }

  // Nhảy lên khi bấm UP hoặc nút A
  if ((up || aBtn) && !isBallJumping) {
    ballVy = -6.2;       
    isBallJumping = true;
    tone(buzzerPin, 900, 30); 
  }

  // Bấm phím DOWN để rơi nhanh hơn
  if (down && isBallJumping) {
    ballVy += 1.5; 
  }

  // Cập nhật khung hình game theo thời gian thực
  if (millis() - lastBounceUpdate > bounceFrameRate) {
    lastBounceUpdate = millis();

    // --- XỬ LÝ VA CHẠM NGANG VỚI TƯỜNG (TRÁI / PHẢI) ---
    ballXWorld += ballVx;
    for(int i = 0; i < totalWalls; i++) {
      if (ballXWorld + 6 > walls[i].x && ballXWorld < walls[i].x + walls[i].w &&
          ballY + 6 > walls[i].y && ballY < walls[i].y + walls[i].h) {
        if (ballVx > 0) {
          ballXWorld = walls[i].x - 6; 
        } else if (ballVx < 0) {
          ballXWorld = walls[i].x + walls[i].w; 
        }
      }
    }

    // Đồng bộ thế giới cuộn theo vị trí bóng
    if (ballXWorld - worldScrollX > 50) {
      worldScrollX = ballXWorld - 50;
    } else if (ballXWorld - worldScrollX < 30) {
      worldScrollX = ballXWorld - 30;
      if (worldScrollX < 0) worldScrollX = 0;
    }
    ballScreenX = ballXWorld - worldScrollX;

    // --- XỬ LÝ VA CHẠM DỌC VỚI TƯỜNG (TRÊN / DƯỚI) ---
    ballY += ballVy;
    ballVy += 0.55; // Trọng lực

    float targetGroundY = 44.0; 
    bool onGroundOrWall = false;

    for(int i = 0; i < totalWalls; i++) {
      if (ballXWorld + 6 > walls[i].x && ballXWorld < walls[i].x + walls[i].w) {
        // 1. Chạm ĐỈNH tường
        if (ballVy >= 0 && ballY + 6 >= walls[i].y && ballY + 6 <= walls[i].y + 8) {
          targetGroundY = walls[i].y - 6;
          onGroundOrWall = true;
          break;
        }
        // 2. Chạm ĐÁY tường (gầm tường)
        if (ballVy < 0 && ballY <= walls[i].y + walls[i].h && ballY >= walls[i].y + walls[i].h - 6) {
          ballY = walls[i].y + walls[i].h;
          ballVy = 0.5; 
        }
      }
    }

    if (ballY >= targetGroundY) {
      ballY = targetGroundY;
      isBallJumping = false;
      ballVy = 0;
    } else {
      if (!onGroundOrWall && ballY < 44) {
        isBallJumping = true;
      }
    }

    // --- XỬ LÝ ĂN VÒNG ĐIỂM (RINGS) ---
    for(int i = 0; i < totalRings; i++) {
      if(!ringCollected[i]) {
        if (ballXWorld + 6 > ringXPositions[i] && ballXWorld < ringXPositions[i] + 10 &&
            ballY + 6 > ringYPositions[i] && ballY < ringYPositions[i] + 10) {
          ringCollected[i] = true;
          bounceScore += 50;
          tone(buzzerPin, 1800, 40); 
        }
      }
    }

    // --- XỬ LÝ VA CHẠM GAI NHỌN (SPIKES) ---
    for(int i = 0; i < totalSpikes; i++) {
      if (ballXWorld + 5 > spikes[i].x && ballXWorld < spikes[i].x + 8 &&
          ballY + 6 >= 42) {
        bounceGameOver = true;
        tone(buzzerPin, 200, 350); 
      }
    }

    // --- TÁI TẠO VẬT THỂ VÔ TẬN LIÊN TỤC ---
    int maxX = 0;
    for(int i = 0; i < totalWalls; i++) if(walls[i].x > maxX) maxX = walls[i].x;
    for(int i = 0; i < totalSpikes; i++) if(spikes[i].x > maxX) maxX = spikes[i].x;
    for(int i = 0; i < totalRings; i++) if(ringXPositions[i] > maxX) maxX = ringXPositions[i];

    for(int i = 0; i < totalWalls; i++) {
      if (walls[i].x + walls[i].w < worldScrollX - 20) {
        maxX += 90 + (rand() % 40);
        walls[i].x = maxX;
        walls[i].y = 18 + (rand() % 20); 
        walls[i].w = 14 + (rand() % 8);
        walls[i].h = 14 + (rand() % 12);
      }
    }

    for(int i = 0; i < totalSpikes; i++) {
      if (spikes[i].x + 8 < worldScrollX - 20) {
        maxX += 70 + (rand() % 30);
        spikes[i].x = maxX;
      }
    }

    for(int i = 0; i < totalRings; i++) {
      if (ringXPositions[i] + 10 < worldScrollX - 20) {
        maxX += 80 + (rand() % 35);
        ringXPositions[i] = maxX;
        ringYPositions[i] = 10 + (rand() % 20);
        ringCollected[i] = false; 
      }
    }
  }

  // --- VẼ ĐỒ HỌA LÊN MÀN HÌNH OLED 128x64 ---
  display.clearDisplay();

  // Hiển thị Điểm số
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("Score: "));
  display.print(bounceScore);

  // Đường mặt đất chính
  display.drawLine(0, 50, 128, 50, SSD1306_WHITE);

  // Vẽ các Bục / Tường chặn 4 cạnh
  for(int i = 0; i < totalWalls; i++) {
    int wallScreenX = walls[i].x - worldScrollX;
    if(wallScreenX >= -30 && wallScreenX <= 138) {
      display.fillRect(wallScreenX, walls[i].y, walls[i].w, walls[i].h, SSD1306_WHITE);
    }
  }

  // Vẽ các Gai nhọn
  for(int i = 0; i < totalSpikes; i++) {
    int spikeScreenX = spikes[i].x - worldScrollX;
    if(spikeScreenX >= -20 && spikeScreenX <= 138) {
      display.drawLine(spikeScreenX, 50, spikeScreenX + 4, 42, SSD1306_WHITE);
      display.drawLine(spikeScreenX + 4, 42, spikeScreenX + 8, 50, SSD1306_WHITE);
      display.drawLine(spikeScreenX, 50, spikeScreenX + 8, 50, SSD1306_WHITE);
    }
  }

  // Vẽ các Vòng điểm lơ lửng
  for(int i = 0; i < totalRings; i++) {
    if(!ringCollected[i]) {
      int ringScreenX = ringXPositions[i] - worldScrollX;
      if(ringScreenX >= -10 && ringScreenX <= 138) {
        display.drawCircle(ringScreenX + 4, ringYPositions[i] + 4, 5, SSD1306_WHITE);
      }
    }
  }

  // Vẽ Quả bóng đỏ
  display.fillCircle(ballScreenX + 3, (int)ballY + 3, 3, SSD1306_WHITE);
  display.drawPixel(ballScreenX + 2, (int)ballY + 2, SSD1306_BLACK); 

  // Bảng thông báo khi Game Over
  if (bounceGameOver) {
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