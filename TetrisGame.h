#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

const int TETRIS_WIDTH = 10;
const int TETRIS_HEIGHT = 20;
byte tetrisBoard[TETRIS_HEIGHT][TETRIS_WIDTH] = {0};

const byte tetrisShapes[7][4][4] = {
  { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} }, // I
  { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} }, // O
  { {0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0} }, // T
  { {0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0} }, // S
  { {0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0} }, // Z
  { {0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0} }, // J
  { {0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0} }  // L
};

int tX = 3, tY = 0;
int tShape = 0;
int nextTetrisShape = 0; 
byte currentPiece[4][4];

int tetrisScore = 0;
bool tetrisGameOver = false;
unsigned long lastTetrisDrop = 0;
unsigned long tetrisDropInterval = 400;

bool canMoveTetris(int x, int y, byte piece[4][4]);

void spawnTetrisPiece() {
  tX = 3;
  tY = -1;
  tShape = nextTetrisShape;             
  nextTetrisShape = random(0, 7);       
  
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      currentPiece[r][c] = tetrisShapes[tShape][r][c];
    }
  }

  if (!canMoveTetris(tX, tY, currentPiece)) {
    memset(tetrisBoard, 0, sizeof(tetrisBoard));
    tetrisGameOver = true;
  }
}

bool canMoveTetris(int x, int y, byte piece[4][4]) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (piece[r][c]) {
        int nx = x + c;
        int ny = y + r;
        if (nx < 0 || nx >= TETRIS_WIDTH || ny >= TETRIS_HEIGHT) return false;
        if (ny >= 0 && tetrisBoard[ny][nx]) return false;
      }
    }
  }
  return true;
}

void initTetrisGame() {
  memset(tetrisBoard, 0, sizeof(tetrisBoard));
  tetrisScore = 0;
  tetrisGameOver = false;
  tetrisDropInterval = 400;
  
  tShape = random(0, 7);
  nextTetrisShape = random(0, 7);
  tX = 3;
  tY = -1;
  
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      currentPiece[r][c] = tetrisShapes[tShape][r][c];
    }
  }

  if (!canMoveTetris(tX, tY, currentPiece)) {
    tetrisGameOver = true;
  }
  delay(200);
}

void runTetrisGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inGame = false;
    delay(200);
    return;
  }

  if (tetrisGameOver) {
    if (up || down || left || right || aBtn) {
      tone(buzzerPin, 1200, 50);
      initTetrisGame();
    }
    return;
  }

  static bool lState = false, rState = false, aState = false;

  if (left && !lState) {
    if (canMoveTetris(tX - 1, tY, currentPiece)) { tX--; tone(buzzerPin, 600, 20); }
  }
  lState = left;

  if (right && !rState) {
    if (canMoveTetris(tX + 1, tY, currentPiece)) { tX++; tone(buzzerPin, 600, 20); }
  }
  rState = right;

  if (down) {
    if (canMoveTetris(tX, tY + 1, currentPiece)) { tY++; }
  }

  // Nút A để xoay khối gạch 90 độ
  if (aBtn && !aState) {
    byte tempPiece[4][4];
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        tempPiece[c][3 - r] = currentPiece[r][c];
      }
    }
    if (canMoveTetris(tX, tY, tempPiece)) {
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          currentPiece[r][c] = tempPiece[r][c];
        }
      }
      tone(buzzerPin, 900, 30);
    }
  }
  aState = aBtn;

  // Tự động rơi
  if (millis() - lastTetrisDrop > tetrisDropInterval) {
    lastTetrisDrop = millis();
    if (canMoveTetris(tX, tY + 1, currentPiece)) {
      tY++;
    } else {
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          if (currentPiece[r][c]) {
            int ny = tY + r;
            if (ny >= 0) tetrisBoard[ny][tX + c] = 1;
          }
        }
      }
      for (int r = TETRIS_HEIGHT - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < TETRIS_WIDTH; c++) {
          if (!tetrisBoard[r][c]) { full = false; break; }
        }
        if (full) {
          tetrisScore += 100;
          tone(buzzerPin, 1500, 50);
          for (int mr = r; mr > 0; mr--) {
            for (int c = 0; c < TETRIS_WIDTH; c++) {
              tetrisBoard[mr][c] = tetrisBoard[mr - 1][c];
            }
          }
            for (int c = 0; c < TETRIS_WIDTH; c++) tetrisBoard[0][c] = 0;
          r++;
        }
      }
      spawnTetrisPiece();
    }
  }

  // Vẽ giao diện game xếp hình
  display.clearDisplay();
  
  // Kích thước mỗi ô gạch vuông: 3x3 pixel (Bảng game rộng 10 ô = 30 pixel, cao 20 ô = 60 pixel)
  int blockSize = 3;
  int boardX = 35; // Căn giữa màn hình 128x64
  int boardY = 2;

  // 1. Vẽ khung chứa bảng game
  display.drawRect(boardX - 1, boardY - 1, (TETRIS_WIDTH * blockSize) + 2, (TETRIS_HEIGHT * blockSize) + 2, SSD1306_WHITE);
  
  // Vẽ các khối đã nằm cố định trên bảng
  for (int r = 0; r < TETRIS_HEIGHT; r++) {
    for (int c = 0; c < TETRIS_WIDTH; c++) {
      if (tetrisBoard[r][c]) {
        // Dùng kích thước 3x3 pixel vuông vức (có thể chỉnh fillRect thành 2x2 nếu muốn tạo đường viền hở giữa các ô)
        display.fillRect(boardX + (c * blockSize), boardY + (r * blockSize), blockSize, blockSize, SSD1306_WHITE);
      }
    }
  }

  // Vẽ khối đang rơi với kích thước vuông 3x3
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (currentPiece[r][c]) {
        int dx = tX + c;
        int dy = tY + r;
        if (dy >= 0) {
          display.fillRect(boardX + (dx * blockSize), boardY + (dy * blockSize), blockSize, blockSize, SSD1306_WHITE);
        }
      }
    }
  }

  // 2. Hiển thị điểm số ở bên trái
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print(F("SCR"));
  display.setCursor(2, 12);
  display.print(tetrisScore);

  // 3. Hiển thị ô "NEXT" ở bên phải màn hình
  display.drawRect(98, 0, 28, 28, SSD1306_WHITE);
  display.setCursor(100, 2);
  display.print(F("NEXT"));
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (tetrisShapes[nextTetrisShape][r][c]) {
        display.fillRect(102 + (c * 2), 12 + (r * 2), 2, 2, SSD1306_WHITE);
      }
    }
  }

  // Màn hình Game Over
  if (tetrisGameOver) {
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