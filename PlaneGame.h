#ifndef PLANE_GAME_H
#define PLANE_GAME_H

#include <Adafruit_SSD1306.h>

namespace PlaneGameNs {
  // Kích thước màn hình (Nằm ngang)
  const int SCREEN_WIDTH = 128;
  const int SCREEN_HEIGHT = 64;

  // Cấu trúc máy bay người chơi (Chiều ngang: X từ trái sang phải, Y từ trên xuống dưới)
  int playerX = 10;
  int playerY = 28;
  const int playerWidth = 12;
  const int playerHeight = 8;

  // Cấu trúc Đạn của người chơi (Bắn ngang sang phải)
  const int MAX_BULLETS = 5;
  struct Bullet {
    int x, y;
    bool active;
  };
  Bullet bullets[MAX_BULLETS];
  unsigned long lastBulletTime = 0;
  const int bulletCooldown = 180; // Tốc độ bắn

  // Cấu trúc Kẻ địch (Xuất phát từ bên phải bay sang trái)
  const int MAX_ENEMIES = 4;
  struct PlaneEnemy {
    int x, y;
    int width, height;
    bool active;
    int speed;
  };
  PlaneEnemy enemies[MAX_ENEMIES];
  unsigned long lastEnemySpawn = 0;
  int enemySpawnInterval = 1300;

  // Biến trạng thái trò chơi
  int planeScore = 0;
  int playerLives = 3;
  bool planeGameOver = false;
  bool restartReady = false;

  // Khởi tạo lại trò chơi
  void initPlaneGame() {
    playerX = 10;
    playerY = 28;
    planeScore = 0;
    playerLives = 3;
    planeGameOver = false;
    restartReady = false;

    for (int i = 0; i < MAX_BULLETS; i++) {
      bullets[i].active = false;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
      enemies[i].active = false;
    }
  }

  // Hàm chạy game Bắn máy bay ngang
  void runPlaneGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
    
    // 1. Thoát về menu bằng nút B
    if (bBtn) {
      tone(buzzerPin, 500, 80);
      inGame = false;
      delay(200);
      return;
    }

    // 2. Xử lý màn hình Game Over (có cơ chế chờ nhả phím)
    if (planeGameOver) {
      if (!up && !down && !left && !right && !aBtn && !bBtn) {
        restartReady = true;
      }

      if (restartReady && (up || down || left || right || aBtn)) {
        tone(buzzerPin, 1200, 50);
        initPlaneGame();
      }
      
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(12, 16);
      display.print(F("GAME OVER"));
      
      display.setTextSize(1);
      display.setCursor(24, 42);
      display.print(F("Score: "));
      display.print(planeScore);
      
      display.setCursor(14, 54);
      display.print(F("Bam phim choi lai"));
      display.display();
      return;
    }

    // 3. Điều khiển di chuyển 4 hướng (Up, Down, Left, Right)
    if (up) {
      playerY -= 3;
      if (playerY < 10) playerY = 10; // Chừa chỗ cho thanh trạng thái phía trên (cao 10px)
    }
    if (down) {
      playerY += 3;
      if (playerY > SCREEN_HEIGHT - playerHeight - 2) playerY = SCREEN_HEIGHT - playerHeight - 2;
    }
    if (left) {
      playerX -= 3;
      if (playerX < 2) playerX = 2;
    }
    if (right) {
      playerX += 3;
      if (playerX > SCREEN_WIDTH - playerWidth - 2) playerX = SCREEN_WIDTH - playerWidth - 2;
    }

    // 4. Xử lý bắn đạn bằng nút A (Bắn sang phải)
    if (aBtn && (millis() - lastBulletTime > bulletCooldown)) {
      lastBulletTime = millis();
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
          bullets[i].x = playerX + playerWidth;
          bullets[i].y = playerY + (playerHeight / 2) - 1;
          bullets[i].active = true;
          tone(buzzerPin, 1400, 15); // Âm thanh phát đạn
          break;
        }
      }
    }

    // 5. Cập nhật vị trí Đạn bay sang phải
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        bullets[i].x += 6;
        if (bullets[i].x > SCREEN_WIDTH) {
          bullets[i].active = false;
        }
      }
    }

    // 6. Sinh kẻ địch từ bên phải màn hình bay sang trái
    if (millis() - lastEnemySpawn > enemySpawnInterval) {
      lastEnemySpawn = millis();
      for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
          enemies[i].width = 10;
          enemies[i].height = 8;
          enemies[i].x = SCREEN_WIDTH + 5;
          enemies[i].y = random(12, SCREEN_HEIGHT - 12);
          // Tốc độ địch tăng dần theo điểm số
          enemies[i].speed = 1 + (planeScore / 350);
          if (enemies[i].speed > 4) enemies[i].speed = 4;
          enemies[i].active = true;
          break;
        }
      }
    }

    // 7. Cập nhật vị trí Kẻ địch và kiểm tra va chạm
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active) {
        enemies[i].x -= enemies[i].speed;

        // Địch bay qua mép trái màn hình (Người chơi bị lọt địch -> Trừ mạng)
        if (enemies[i].x + enemies[i].width < 0) {
          enemies[i].active = false;
          playerLives--;
          tone(buzzerPin, 200, 100);
          if (playerLives <= 0) {
            planeGameOver = true;
            restartReady = false;
            tone(buzzerPin, 150, 300);
          }
        }

        // Va chạm giữa Kẻ địch và Phi thuyền người chơi
        if (enemies[i].x < playerX + playerWidth &&
            enemies[i].x + enemies[i].width > playerX &&
            enemies[i].y < playerY + playerHeight &&
            enemies[i].y + enemies[i].height > playerY) {
          enemies[i].active = false;
          playerLives--;
          tone(buzzerPin, 150, 200);
          if (playerLives <= 0) {
            planeGameOver = true;
            restartReady = false;
          }
        }

        // Va chạm giữa Đạn của người chơi và Kẻ địch
        for (int j = 0; j < MAX_BULLETS; j++) {
          if (bullets[j].active && enemies[i].active) {
            if (bullets[j].x >= enemies[i].x &&
                bullets[j].x <= enemies[i].x + enemies[i].width &&
                bullets[j].y >= enemies[i].y &&
                bullets[j].y <= enemies[i].y + enemies[i].height) {
              
              bullets[j].active = false;
              enemies[i].active = false;
              planeScore += 50;
              tone(buzzerPin, 700, 40); // Tiếng nổ tiêu diệt địch
            }
          }
        }
      }
    }

    // 8. Vẽ đồ họa màn hình chơi
    display.clearDisplay();

    // Thanh trạng thái phía trên (Điểm số & Mạng sống)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 1);
    display.print(F("SCORE:"));
    display.print(planeScore);

    display.setCursor(88, 1);
    display.print(F("LV:"));
    display.print(playerLives);

    // Đường phân cách ngăn giữa thanh trạng thái và vùng chiến đấu
    display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

    // --- THIẾT KẾ LẠI HÌNH DÁNG PHI THUYỀN (Hướn ngang sang phải) ---
    // Mũi nhọn phía trước
    display.drawPixel(playerX + 11, playerY + 3, SSD1306_WHITE);
    display.drawPixel(playerX + 11, playerY + 4, SSD1306_WHITE);
    // Thân chính phi thuyền
    display.fillRect(playerX + 4, playerY + 2, 7, 4, SSD1306_WHITE);
    // Mũi cabin phía trước thân
    display.fillRect(playerX + 8, playerY + 3, 2, 2, SSD1306_WHITE);
    // Cánh trên & Cánh dưới xòe rộng sắc sảo
    display.drawPixel(playerX + 3, playerY + 1, SSD1306_WHITE);
    display.drawPixel(playerX + 2, playerY, SSD1306_WHITE);
    display.drawPixel(playerX + 3, playerY + 6, SSD1306_WHITE);
    display.drawPixel(playerX + 2, playerY + 7, SSD1306_WHITE);
    // Động cơ phản lực phía sau
    display.fillRect(playerX, playerY + 3, 2, 2, SSD1306_WHITE);

    // Vẽ đạn (Đường đạn ngang nhỏ gọn)
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        display.fillRect(bullets[i].x, bullets[i].y, 3, 1, SSD1306_WHITE);
      }
    }

    // --- THIẾT KẾ LẠI HÌNH DÁNG KẺ ĐỊCH (Phi thuyền địch bay hướng sang trái) ---
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active) {
        // Thân máy bay địch
        display.fillRect(enemies[i].x + 2, enemies[i].y + 2, 6, 4, SSD1306_WHITE);
        // Mũi nhọn hướng về bên trái
        display.drawPixel(enemies[i].x, enemies[i].y + 3, SSD1306_WHITE);
        display.drawPixel(enemies[i].x + 1, enemies[i].y + 3, SSD1306_WHITE);
        display.drawPixel(enemies[i].x, enemies[i].y + 4, SSD1306_WHITE);
        display.drawPixel(enemies[i].x + 1, enemies[i].y + 4, SSD1306_WHITE);
        // Cánh địch
        display.drawPixel(enemies[i].x + 4, enemies[i].y + 1, SSD1306_WHITE);
        display.drawPixel(enemies[i].x + 4, enemies[i].y + 6, SSD1306_WHITE);
      }
    }

    display.display();
  }
}

// Wrapper Functions tương thích với mảng gameList
void initPlaneGame() {
  PlaneGameNs::initPlaneGame();
}

void runPlaneGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  PlaneGameNs::runPlaneGame(up, down, left, right, aBtn, bBtn, display, buzzerPin, inGame);
}

#endif