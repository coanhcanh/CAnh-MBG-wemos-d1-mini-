#ifndef PACMAN_GAME_H
#define PACMAN_GAME_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Kích thước lưới bản đồ (Khớp chuẩn màn hình OLED 128x64: Dùng 28 cột cho mê cung, chừa 4 cột bên phải làm bảng điểm)
#define PAC_W 32
#define PAC_H 16
#define TILE_S 4

// Trạng thái ô: 0 = Trống, 1 = Tường, 2 = Chấm thức ăn nhỏ, 3 = Viên sức mạnh (Power Pellet)
static byte pacMap[PAC_H][PAC_W];

static int pacX, pacY;         // Tọa độ Pacman
static int pacDirX, pacDirY;   // Hướng di chuyển hiện tại
static int pacNDirX, pacNDirY; // Hướng dự kiến theo nút bấm
static bool pacMouthOpen;      // Trạng thái miệng Pacman (đóng/mở)

// 2 con Ma
static int pacG1X, pacG1Y, pacG1DirX, pacG1DirY;
static int pacG2X, pacG2Y, pacG2DirX, pacG2DirY;

static unsigned long pacPowerTimer; // Bộ đếm thời gian sức mạnh (3 giây)
static bool pacIsPowered;

static int pacScore;
static int pacLives;
static bool pacIsGameOver;
static bool pacIsGameWin;
static unsigned long pacLastTick;
const unsigned long pacTickInterval = 140; // Tốc độ game

// Khởi tạo bản đồ mê cung chuẩn theo bản vẽ của bạn
void initPacmanGame() {
  pacScore = 0;
  pacLives = 3;
  pacIsGameOver = false;
  pacIsGameWin = false;
  pacIsPowered = false;
  pacMouthOpen = false;
  
  // Vị trí khởi đầu Pacman (Chấm đỏ ở phía dưới chuồng ma)
  pacX = 14;
  pacY = 12;
  pacDirX = 0; pacDirY = 0;
  pacNDirX = 0; pacNDirY = 0;

  // Vị trí khởi đầu 2 con ma (Hai chấm xanh dương bên trong chuồng)
  pacG1X = 12; pacG1Y = 9; pacG1DirX = -1; pacG1DirY = 0;
  pacG2X = 16; pacG2Y = 9; pacG2DirX = 1;  pacG2DirY = 0;

  // Xây dựng mê cung theo sơ đồ pixel mẫu (giới hạn trong 28 cột đầu, cột 28-31 dùng cho bảng điểm)
  for (int y = 0; y < PAC_H; y++) {
    for (int x = 0; x < PAC_W; x++) {
      // 1. Viền tường ngoài bao quanh khu vực mê cung (từ cột 0 đến 27)
      if (x == 0 || x == 27 || y == 0 || y == PAC_H - 1) {
        pacMap[y][x] = 1;
      }
      // Khu vực bảng thông tin bên phải (Cột 28 đến 31): Đánh dấu trống để vẽ UI riêng
      else if (x >= 28) {
        pacMap[y][x] = 0;
      }
      // 2. Chuồng ma ở chính giữa (Đã mở rộng rộng cửa phía trên từ cột 12 đến 15 để ma dễ dàng đi ra)
      else if ((x >= 10 && x <= 17) && (y >= 7 && y <= 11)) {
        if (y == 7 && (x >= 12 && x <= 15)) pacMap[y][x] = 0; // Cửa chuồng phía trên rộng rãi (4 ô)
        else if (x == 10 || x == 17 || y == 7 || y == 11) pacMap[y][x] = 1; // Khung chuồng
        else pacMap[y][x] = 0; // Không gian bên trong chuồng
      }
      // 3. Các khối chướng ngại vật đối xứng hai bên trái/phải và các thanh chắn trên/dưới theo bản vẽ
      else if (
        ((x >= 2 && x <= 8 || x >= 19 && x <= 25) && y == 2) ||
        (x >= 10 && x <= 17 && y == 2) ||
        (x >= 10 && x <= 17 && y == 5) ||
        (x >= 10 && x <= 17 && y == 13) ||
        ((x >= 2 && x <= 8 || x >= 19 && x <= 25) && y == 13) ||
        
        (x == 2 && (y >= 4 && y <= 11)) ||
        (x == 25 && (y >= 4 && y <= 11)) ||
        
        ((x >= 4 && x <= 6) && y == 4) || (x == 5 && (y >= 4 && y <= 6)) ||
        ((x >= 21 && x <= 23) && y == 4) || (x == 22 && (y >= 4 && y <= 6)) ||
        ((x >= 4 && x <= 6) && y == 9) || (x == 5 && (y >= 9 && y <= 11)) ||
        ((x >= 21 && x <= 23) && y == 9) || (x == 22 && (y >= 9 && y <= 11))
      ) {
        pacMap[y][x] = 1;
      } else {
        pacMap[y][x] = 2; // Mặc định các ô trống còn lại chứa thức ăn nhỏ
      }
    }
  }

  // 4. Đặt 4 viên sức mạnh ở 4 góc bản đồ
  pacMap[1][1] = 3;
  pacMap[1][26] = 3;
  pacMap[PAC_H - 2][1] = 3;
  pacMap[PAC_H - 2][26] = 3;

  // Dọn sạch ô xuất phát của Pacman và Ma để tránh bị kẹt tường
  pacMap[12][14] = 0;
  pacMap[9][12] = 0;
  pacMap[9][16] = 0;
  // Dọn thêm khoảng trống ngay lối ra cửa chuồng để ma dễ thoát ra ngoài
  pacMap[6][13] = 0;
  pacMap[6][14] = 0;

  pacLastTick = millis();
}

bool isPacWall(int x, int y) {
  if (x < 0 || x >= 28 || y < 0 || y >= PAC_H) return true;
  return pacMap[y][x] == 1;
}

// Hàm di chuyển AI cho Ma ngẫu nhiên
void movePacGhost(int &gx, int &gy, int &gDx, int &gDy) {
  if (isPacWall(gx + gDx, gy + gDy) || random(0, 4) == 0) {
    int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    int r = random(0, 4);
    gDx = dirs[r][0];
    gDy = dirs[r][1];
  }
  if (!isPacWall(gx + gDx, gy + gDy)) {
    gx += gDx;
    gy += gDy;
  }
}

void runPacmanGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  if (bBtn) {
    tone(buzzerPin, 200, 80);
    inGame = false;
    return;
  }

  // Nhận phím điều hướng
  if (up)    { pacNDirX = 0;  pacNDirY = -1; }
  if (down)  { pacNDirX = 0;  pacNDirY = 1;  }
  if (left)  { pacNDirX = -1; pacNDirY = 0;  }
  if (right) { pacNDirX = 1;  pacNDirY = 0;  }

  if (millis() - pacLastTick > pacTickInterval) {
    pacLastTick = millis();

    if (!pacIsGameOver && !pacIsGameWin) {
      // Đổi hướng Pacman nếu hợp lệ
      if (!isPacWall(pacX + pacNDirX, pacY + pacNDirY)) {
        pacDirX = pacNDirX;
        pacDirY = pacNDirY;
      }
      // Tiến bước Pacman
      if (!isPacWall(pacX + pacDirX, pacY + pacDirY)) {
        pacX += pacDirX;
        pacY += pacDirY;
        pacMouthOpen = !pacMouthOpen;
      }

      // Kiểm tra ăn thức ăn
      if (pacMap[pacY][pacX] == 2) {
        pacMap[pacY][pacX] = 0;
        pacScore += 10;
        tone(buzzerPin, 900, 20);
      } else if (pacMap[pacY][pacX] == 3) {
        pacMap[pacY][pacX] = 0;
        pacScore += 50;
        pacIsPowered = true;
        pacPowerTimer = millis(); 
        tone(buzzerPin, 1200, 100);
      }

      // Hết thời gian sức mạnh sau 3 giây
      if (pacIsPowered && (millis() - pacPowerTimer > 3000)) {
        pacIsPowered = false;
      }

      // Di chuyển 2 con ma
      movePacGhost(pacG1X, pacG1Y, pacG1DirX, pacG1DirY);
      movePacGhost(pacG2X, pacG2Y, pacG2DirX, pacG2DirY);

      // Kiểm tra va chạm với ma
      if ((pacX == pacG1X && pacY == pacG1Y) || (pacX == pacG2X && pacY == pacG2Y)) {
        if (pacIsPowered) {
          pacScore += 200;
          tone(buzzerPin, 1500, 150);
          if (pacX == pacG1X && pacY == pacG1Y) { pacG1X = 12; pacG1Y = 9; }
          if (pacX == pacG2X && pacY == pacG2Y) { pacG2X = 16; pacG2Y = 9; }
        } else {
          pacLives--;
          tone(buzzerPin, 120, 250);
          if (pacLives <= 0) {
            pacIsGameOver = true;
          } else {
            pacX = 14; pacY = 12;
            pacDirX = 0; pacDirY = 0;
          }
        }
      }

      // Kiểm tra điều kiện thắng
      bool remainingFood = false;
      for (int y = 0; y < PAC_H; y++) {
        for (int x = 0; x < 28; x++) {
          if (pacMap[y][x] == 2 || pacMap[y][x] == 3) {
            remainingFood = true;
            break;
          }
        }
        if (remainingFood) break;
      }
      if (!remainingFood) {
        pacIsGameWin = true;
      }
    } else {
      if (aBtn) {
        initPacmanGame(); 
      }
    }
  }

  // --- VẼ ĐỒ HỌA LÊN MÀN HÌNH OLED 128x64 ---
  display.clearDisplay();

  // 1. Vẽ khung viền tường ngoài mê cung (Cột 0 đến 27) và các khối tường
  display.drawRect(0, 0, 28 * TILE_S, PAC_H * TILE_S, SSD1306_WHITE);

  for (int y = 0; y < PAC_H; y++) {
    for (int x = 0; x < 28; x++) {
      int rx = x * TILE_S;
      int ry = y * TILE_S;
      if (pacMap[y][x] == 1) {
        display.fillRect(rx, ry, TILE_S, TILE_S, SSD1306_WHITE); 
      } else if (pacMap[y][x] == 2) {
        display.drawPixel(rx + 1, ry + 1, SSD1306_WHITE);
      } else if (pacMap[y][x] == 3) {
        display.fillCircle(rx + 2, ry + 2, 2, SSD1306_WHITE);
      }
    }
  }

  // 2. Vẽ Pacman
  int pxCenter = pacX * TILE_S + 2;
  int pyCenter = pacY * TILE_S + 2;
  if (pacMouthOpen) {
    display.fillCircle(pxCenter, pyCenter, 3, SSD1306_WHITE);
    int cutX = pxCenter;
    int cutY = pyCenter;
    if (pacDirX == 1)  cutX += 2;
    if (pacDirX == -1) cutX -= 2;
    if (pacDirY == 1)  cutY += 2;
    if (pacDirY == -1) cutY -= 2;
    display.fillCircle(cutX, cutY, 2, SSD1306_BLACK);
  } else {
    display.fillCircle(pxCenter, pyCenter, 3, SSD1306_WHITE);
  }

  // 3. Vẽ 2 con Ma
  int g1Px = pacG1X * TILE_S + 2;
  int g1Py = pacG1Y * TILE_S + 2;
  int g2Px = pacG2X * TILE_S + 2;
  int g2Py = pacG2Y * TILE_S + 2;

  if (pacIsPowered) {
    display.fillCircle(g1Px, g1Py, 3, SSD1306_WHITE);
    display.fillCircle(g2Px, g2Py, 3, SSD1306_WHITE);
  } else {
    display.drawCircle(g1Px, g1Py, 3, SSD1306_WHITE);
    display.drawCircle(g2Px, g2Py, 3, SSD1306_WHITE);
    display.drawPixel(g1Px, g1Py, SSD1306_WHITE);
    display.drawPixel(g2Px, g2Py, SSD1306_WHITE);
  }

  // 4. Khu vực bảng điểm hiển thị ở vùng trống bên phải (Từ pixel X = 114 đến 128)
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(114, 2);
  display.print(F("SC"));
  display.setCursor(114, 12);
  display.print(pacScore);

  display.setCursor(114, 32);
  display.print(F("LV"));
  display.setCursor(114, 42);
  display.print(pacLives);

  // 5. Thông báo Thắng / Thua căn giữa đè lên màn hình
  if (pacIsGameOver) {
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
  } else if (pacIsGameWin) {
    display.setTextSize(1); // Đặt lại size mặc định trước để vẽ khung
    display.fillRect(2, 13, 124, 50, SSD1306_BLACK);  // Khung nền đen to hơn
    display.drawRect(2, 11, 124, 52, SSD1306_WHITE);  // Viền khung trắng

    // Dòng 1: Chữ GAME OVER! size 2
    display.setTextSize(2);                         
    display.setCursor(16, 22);                       // Căn chỉnh chữ size 2 ở giữa khung
    display.print(F("YOU WIN!"));

    // Dòng 2: Chữ hướng dẫn size 1 bên dưới
    display.setTextSize(1);                         
    display.setCursor(14, 46);                      // Đặt dòng chữ "Bấm phím chơi lại" phía dưới
    display.print(F("Bam phim choi lai"));
  }

  display.display();
}

#endif