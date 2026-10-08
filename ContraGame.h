#ifndef CONTRA_GAME_H
#define CONTRA_GAME_H

// --- THÔNG SỐ CƠ CHẾ CONTRA CHUẨN ---
float worldX = 0; // Vị trí cuộn camera của màn chơi
float cPlayerX = 20; // Vị trí tương đối trên màn hình
float cPlayerY = 40;
int cPlayerW = 6;
int cPlayerH = 12;
bool cIsJumping = false;
float cJumpSpeed = 0;
bool cIsCrouching = false;
int cFacingDir = 1; // 1: Phải, -1: Trái
int cAimDir = 0; // 0: Ngang, 1: Lên, -1: Xuống, 2: Chéo lên

int cScore = 0;
bool cGameOver = false;
bool cGameWin = false;
int cLives = 3;

// Loại vũ khí: 0: Normal, 1: Machine Gun, 2: Spread Gun
int cCurrentWeapon = 0; 

// Hệ thống đạn người chơi
struct CBullet {
  float x, y; // Tọa độ thế giới (world coordinate)
  float vx, vy;
  bool active;
};
const int maxCBullets = 8;
CBullet cBullets[maxCBullets];

// Hệ thống Kẻ địch & Boss
struct CEnemy {
  float x; // Tọa độ thế giới
  float y;
  int type; // 0: Lính đi bộ, 1: Ụ súng cố định, 2: Boss cuối màn
  int health;
  bool active;
};
const int maxCEnemies = 5;
CEnemy cEnemies[maxCEnemies];

unsigned long cLastShoot = 0;
unsigned long cLastUpdate = 0;
const int cFrameRate = 25;
const float mapLength = 1000.0; // Chiều dài màn chơi

void initContraGame() {
  worldX = 0;
  cPlayerX = 20;
  cPlayerY = 40;
  cIsJumping = false;
  cJumpSpeed = 0;
  cIsCrouching = false;
  cFacingDir = 1;
  cAimDir = 0;
  cScore = 0;
  cGameOver = false;
  cGameWin = false;
  cLives = 3;
  cCurrentWeapon = 0;
  cLastShoot = 0;

  for (int i = 0; i < maxCBullets; i++) cBullets[i].active = false;
  for (int i = 0; i < maxCEnemies; i++) cEnemies[i].active = false;

  // Tạo sẵn một số kẻ địch dọc đường đi
  for (int i = 0; i < 4; i++) {
    cEnemies[i].x = 150 + i * 200;
    cEnemies[i].y = 42;
    cEnemies[i].type = (i % 2 == 0) ? 0 : 1;
    cEnemies[i].health = 1;
    cEnemies[i].active = true;
  }
  // Boss cuối màn ở mốc 900
  cEnemies[4].x = 900;
  cEnemies[4].y = 32;
  cEnemies[4].type = 2; // Boss
  cEnemies[4].health = 15;
  cEnemies[4].active = true;

  delay(200);
}

void resetContraGame() {
  initContraGame();
}

void runContraGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  // Nút B: Thoát về Menu chính[cite: 1]
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inGame = false;
    delay(200);
    return;
  }

  // Nếu Game Over hoặc Win, bấm phím bất kỳ để chơi lại
  if (cGameOver || cGameWin) {
    if (up || down || left || right || aBtn) {
      tone(buzzerPin, 1200, 50);
      resetContraGame();
    }
    return;
  }

  if (millis() - cLastUpdate > cFrameRate) {
    cLastUpdate = millis();

    // --- XỬ LÝ TRẠNG THÁI & HƯỚNG NGẮM (Theo chuẩn GDD) ---
    if (down && !cIsJumping) {
      cIsCrouching = true;
      cAimDir = -1; // Ngắm xuống / Nằm ngồi thấp[cite: 1]
    } else {
      cIsCrouching = false;
      if (up) cAimDir = 1; // Ngắm lên[cite: 1]
      else cAimDir = 0;    // Ngang
    }

    // --- DI CHUYỂN TRÁI / PHẢI & CUỘN CAMERA ---[cite: 1]
    if (left) {
      cFacingDir = -1;
      cPlayerX -= 2.5;
      if (cPlayerX < 10) {
        cPlayerX = 10;
        worldX -= 2.5; // Cuộn ngược màn hình
        if (worldX < 0) worldX = 0;
      }
    }
    if (right) {
      cFacingDir = 1;
      cPlayerX += 2.5;
      // Camera cuộn theo hướng tiến của người chơi khi qua nửa màn hình
      if (cPlayerX > 50) {
        cPlayerX = 50;
        worldX += 2.5; 
        if (worldX > mapLength - 128) worldX = mapLength - 128;
      }
    }

    // --- NHẢY (Phím A) ---[cite: 1]
    if (aBtn && !cIsJumping && !cIsCrouching) {
      cIsJumping = true;
      cJumpSpeed = -5.8;
      tone(buzzerPin, 880, 25);
    }

    if (cIsJumping) {
      cPlayerY += cJumpSpeed;
      cJumpSpeed += 0.55; // Trọng lực
      if (cPlayerY >= 40) {
        cPlayerY = 40;
        cIsJumping = false;
      }
    }

    // --- BẮN ĐẠN (Phím B) ---[cite: 1]
    static bool lastBState = false;
    // Hỗ trợ bắn liên thanh nếu giữ nút B hoặc bấm liên tục tùy vũ khí
    if (bBtn && (millis() - cLastShoot > 150)) {
      for (int i = 0; i < maxCBullets; i++) {
        if (!cBullets[i].active) {
          cBullets[i].x = worldX + cPlayerX + (cFacingDir > 0 ? cPlayerW : 0);
          cBullets[i].y = cPlayerY + (cIsCrouching ? 6 : 4);
          
          // Xác định vận tốc đạn theo hướng ngắm (Bắn ngang, chéo, lên, xuống)[cite: 1]
          float speed = 6.0;
          if (cAimDir == 1) { // Lên
            cBullets[i].vx = cFacingDir * (speed * 0.7);
            cBullets[i].vy = -speed * 0.7;
          } else if (cAimDir == -1) { // Xuống/Thấp
            cBullets[i].vx = cFacingDir * speed;
            cBullets[i].vy = 0;
          } else { // Ngang
            cBullets[i].vx = cFacingDir * speed;
            cBullets[i].vy = 0;
          }

          cBullets[i].active = true;
          cLastShoot = millis();
          tone(buzzerPin, 1600, 15); // Âm thanh đạn nổ
          break;
        }
      }
    }

    // Cập nhật tọa độ đạn bay
    for (int i = 0; i < maxCBullets; i++) {
      if (cBullets[i].active) {
        cBullets[i].x += cBullets[i].vx;
        cBullets[i].y += cBullets[i].vy;
        // Hủy đạn khi ra ngoài màn hình tương đối
        if (cBullets[i].x < worldX || cBullets[i].x > worldX + 128 || cBullets[i].y < 0 || cBullets[i].y > 64) {
          cBullets[i].active = false;
        }
      }
    }

    // --- CẬP NHẬT KẺ ĐỊNH & VA CHẠM ---[cite: 1, 8]
    for (int i = 0; i < maxCEnemies; i++) {
      if (cEnemies[i].active) {
        // Lính đi bộ tự động di chuyển nhẹ lại gần
        if (cEnemies[i].type == 0) {
          if (cEnemies[i].x > worldX + cPlayerX) cEnemies[i].x -= 0.8;
        }

        // Kiểm tra đạn trúng địch[cite: 1]
        for (int b = 0; b < maxCBullets; b++) {
          if (cBullets[b].active && 
              cBullets[b].x >= cEnemies[i].x && cBullets[b].x <= cEnemies[i].x + (cEnemies[i].type == 2 ? 16 : 6) &&
              cBullets[b].y >= cEnemies[i].y && cBullets[b].y <= cEnemies[i].y + (cEnemies[i].type == 2 ? 16 : 10)) {
            cBullets[b].active = false;
            cEnemies[i].health--;
            tone(buzzerPin, 350, 30);

            if (cEnemies[i].health <= 0) {
              cEnemies[i].active = false;
              cScore += (cEnemies[i].type == 2 ? 500 : 100);
              if (cEnemies[i].type == 2) {
                cGameWin = true; // Tiêu diệt Boss -> Thắng màn chơi[cite: 1, 8]
              }
            }
          }
        }

        // Kiểm tra va chạm người chơi với kẻ địch (1 hit chết chuẩn bản kinh điển)[cite: 1, 9]
        float realPlayerWorldX = worldX + cPlayerX;
        if (realPlayerWorldX + cPlayerW > cEnemies[i].x && realPlayerWorldX < cEnemies[i].x + (cEnemies[i].type == 2 ? 16 : 6) &&
            cPlayerY + cPlayerH > cEnemies[i].y) {
          cLives--;
          tone(buzzerPin, 150, 400);
          if (cLives <= 0) {
            cGameOver = true;
          } else {
            // Hồi sinh ngắn tại chỗ / lùi lại checkpoint
            cPlayerX = 20;
            worldX = max(0.0f, worldX - 40.0f);
          }
        }
      }
    }

    // Kiểm tra hoàn thành màn nếu đi đến cuối bản đồ
    if (worldX >= mapLength - 128 && !cEnemies[4].active) {
      cGameWin = true;
    }
  }

  // --- VẼ ĐỒ HỌA GIAO DIỆN (OLED 128x64) ---
  display.clearDisplay();

  // 1. Thanh thông tin HUD trên cùng
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print(F("SC:"));
  display.print(cScore);
  display.setCursor(75, 2);
  display.print(F("LIV:"));
  display.print(cLives);

  // 2. Vẽ mặt đất & cấu trúc nền cuộn theo camera
  display.drawLine(0, 52, 128, 52, SSD1306_WHITE);
  int startGrid = (int)(-worldX) % 16;
  for (int x = startGrid; x < 128; x += 16) {
    display.drawPixel(x, 55, SSD1306_WHITE);
    display.drawPixel(x + 4, 57, SSD1306_WHITE);
  }

  // Thanh tiến độ màn chơi ở góc dưới hoặc trên
  int progressWidth = (int)((worldX / (mapLength - 128)) * 40);
  display.drawRect(85, 54, 40, 6, SSD1306_WHITE);
  display.fillRect(87, 56, constrain(progressWidth, 0, 36), 2, SSD1306_WHITE);

  // 3. Vẽ Nhân vật Contra (chi tiết tư thế đứng, ngồi, nhảy, súng)[cite: 1, 2]
  float drawY = cPlayerY + (cIsCrouching ? 4 : 0);
  int drawH = cIsCrouching ? 8 : cPlayerH;
  // Thân người
  display.fillRect(cPlayerX, drawY, cPlayerW, drawH, SSD1306_WHITE);
  // Đầu
  display.fillRect(cPlayerX + (cFacingDir > 0 ? 1 : 1), drawY - 3, 4, 3, SSD1306_WHITE);
  
  // Nòng súng hướng theo trạng thái ngắm[cite: 1, 4]
  if (cAimDir == 1) { // Hướng lên
    display.fillRect(cPlayerX + (cFacingDir > 0 ? 4 : -2), drawY - 5, 2, 4, SSD1306_WHITE);
  } else if (cAimDir == -1) { // Hướng xuống / Thấp
    display.fillRect(cPlayerX + (cFacingDir > 0 ? cPlayerW : -4), drawY + 4, 4, 2, SSD1306_WHITE);
  } else { // Ngang
    display.fillRect(cPlayerX + (cFacingDir > 0 ? cPlayerW : -4), drawY + 3, 4, 2, SSD1306_WHITE);
  }

  // 4. Vẽ Đạn người chơi[cite: 1, 5]
  for (int i = 0; i < maxCBullets; i++) {
    if (cBullets[i].active) {
      float screenBulletX = cBullets[i].x - worldX;
      if (screenBulletX >= 0 && screenBulletX <= 128) {
        display.fillRect(screenBulletX, cBullets[i].y, 3, 2, SSD1306_WHITE);
      }
    }
  }

  // 5. Vẽ Kẻ địch & Boss xuất hiện trong tầm nhìn màn hình[cite: 1, 7, 8]
  for (int i = 0; i < maxCEnemies; i++) {
    if (cEnemies[i].active) {
      float screenEnemyX = cEnemies[i].x - worldX;
      if (screenEnemyX >= -20 && screenEnemyX <= 148) {
        if (cEnemies[i].type == 0) {
          // Lính đi bộ[cite: 1, 7]
          display.fillRect(screenEnemyX, cEnemies[i].y, 6, 10, SSD1306_WHITE);
        } else if (cEnemies[i].type == 1) {
          // Ụ súng cố định[cite: 1, 6, 7]
          display.fillTriangle(screenEnemyX, cEnemies[i].y + 10, screenEnemyX + 8, cEnemies[i].y + 10, screenEnemyX + 4, cEnemies[i].y + 4, SSD1306_WHITE);
        } else if (cEnemies[i].type == 2) {
          // Boss cuối màn[cite: 1, 8]
          display.fillRect(screenEnemyX, cEnemies[i].y, 16, 16, SSD1306_WHITE);
          display.drawRect(screenEnemyX - 2, cEnemies[i].y - 4, 20, 3, SSD1306_WHITE);
          display.fillRect(screenEnemyX - 2, cEnemies[i].y - 4, (cEnemies[i].health * 20) / 15, 3, SSD1306_WHITE); // Thanh máu boss[cite: 1, 8]
        }
      }
    }
  }

  // 6. Bảng Game Over / Victory[cite: 1, 8, 9]
  if (cGameOver) {
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
    return;
  }

  if (cGameWin) {
    display.fillRect(14, 14, 100, 36, SSD1306_BLACK);
    display.drawRect(12, 12, 104, 40, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(26, 18);
    display.print(F("MISSION CLEAR!"));
    display.setCursor(20, 34);
    display.print(F("YOU WIN!"));
    display.display();
    return;
  }

  display.display();
}

#endif