#ifndef CAR_GAME_H
#define CAR_GAME_H

// --- BIẾN QUẢN LÝ GAME ĐUA XE ---
int playerLane = 1;     // Làn hiện tại của xe người chơi (0: trên, 1: giữa, 2: dưới)
float playerX = 16.0;   // Tọa độ X cố định của xe người chơi
int carScore = 0;       // Điểm số
bool carGameOver = false;

// Cấu trúc xe chướng ngại vật
struct EnemyCar {
  int x;
  int lane; // 0, 1, 2
  bool active;
};

const int maxEnemies = 2;
EnemyCar enemies[maxEnemies];

int roadSpeed = 3;      // Tốc độ di chuyển của chướng ngại vật
unsigned long lastCarUpdate = 0;
const int carFrameRate = 30;

void initCarGame() {
  playerLane = 1;
  carScore = 0;
  carGameOver = false;
  roadSpeed = 3;

  // Khởi tạo xe địch ở các làn khác nhau
  enemies[0].x = 128;
  enemies[0].lane = 0;
  enemies[0].active = true;

  enemies[1].x = 180;
  enemies[1].lane = 2;
  enemies[1].active = true;

  delay(200);
}

void resetCarGame() {
  initCarGame();
}

void runCarGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  // Nút B: Thoát về Menu chính
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inGame = false;
    delay(200);
    return;
  }

  // Nếu Game Over, bấm phím bất kỳ để chơi lại
  if (carGameOver) {
    if (up || down || left || right || aBtn) {
      tone(buzzerPin, 1200, 50);
      resetCarGame();
    }
    return;
  }

  // Xử lý chuyển làn bằng nút UP và DOWN
  static bool lastUpState = false;
  static bool lastDownState = false;

  // Bấm nút UP để chuyển lên làn trên
  if (up && !lastUpState) {
    if (playerLane > 0) {
      playerLane--;
      tone(buzzerPin, 800, 20);
    }
  }
  lastUpState = up;

  // Bấm nút DOWN để chuyển xuống làn dưới
  if (down && !lastDownState) {
    if (playerLane < 2) {
      playerLane++;
      tone(buzzerPin, 800, 20);
    }
  }
  lastDownState = down;

  // Cập nhật khung hình game theo thời gian thực
  if (millis() - lastCarUpdate > carFrameRate) {
    lastCarUpdate = millis();

    // Tăng điểm theo thời gian sống sót
    carScore++;

    // Tăng tốc độ dần khi điểm cao hơn
    if (carScore > 0 && carScore % 200 == 0 && roadSpeed < 6) {
      roadSpeed++;
    }

    // Di chuyển các xe địch sang trái
    for (int i = 0; i < maxEnemies; i++) {
      if (enemies[i].active) {
        enemies[i].x -= roadSpeed;

        // Nếu xe chạy qua màn hình, tái sinh ở bên phải với làn ngẫu nhiên
        if (enemies[i].x < -12) {
          enemies[i].x = 128 + (rand() % 50);
          enemies[i].lane = rand() % 3;
        }

        // Kiểm tra va chạm (Xe người chơi ở X = 16, rộng 10, cao 6)
        if (enemies[i].x + 10 > playerX && enemies[i].x < playerX + 10) {
          if (enemies[i].lane == playerLane) {
            carGameOver = true;
            tone(buzzerPin, 200, 300);
          }
        }
      }
    }
  }

  // --- VẼ ĐỒ HỌA LÊN MÀN HÌNH OLED 128x64 ---
  display.clearDisplay();

  // Hiển thị điểm số ở góc trên
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print(F("Score: "));
  display.print(carScore);

  // Vẽ khung đường đua 3 làn (từ y = 10 đến y = 58)
  display.drawRect(0, 10, 128, 48, SSD1306_WHITE);
  
  // Vẽ vạch đứt khúc phân cách giữa 3 làn
  for (int x = 0; x < 128; x += 10) {
    display.drawPixel(x, 26, SSD1306_WHITE);
    display.drawPixel(x, 42, SSD1306_WHITE);
  }

  // Vẽ xe của người chơi
  int playerY = 17 + playerLane * 16;
  display.fillRect((int)playerX, playerY, 10, 6, SSD1306_WHITE);
  display.drawPixel((int)playerX + 2, playerY - 1, SSD1306_WHITE); // Bánh xe trên
  display.drawPixel((int)playerX + 7, playerY - 1, SSD1306_WHITE);
  display.drawPixel((int)playerX + 2, playerY + 6, SSD1306_WHITE); // Bánh xe dưới
  display.drawPixel((int)playerX + 7, playerY + 6, SSD1306_WHITE);

  // Vẽ các xe chướng ngại vật
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].active) {
      int enemyY = 17 + enemies[i].lane * 16;
      display.fillRect(enemies[i].x, enemyY, 10, 6, SSD1306_WHITE);
      display.drawPixel(enemies[i].x + 2, enemyY - 1, SSD1306_WHITE);
      display.drawPixel(enemies[i].x + 7, enemyY - 1, SSD1306_WHITE);
      display.drawPixel(enemies[i].x + 2, enemyY + 6, SSD1306_WHITE);
      display.drawPixel(enemies[i].x + 7, enemyY + 6, SSD1306_WHITE);
    }
  }

  // Hộp thông báo Game Over
  if (carGameOver) {
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