//Quản lý menu, danh sách trò chơi và hiệu ứng âm thanh khởi động

#ifndef FLAPPY_GAME_H
#define FLAPPY_GAME_H

// --- BIẾN QUẢN LÝ FLAPPY BIRD ---
float birdY = 32.0;       // Tọa độ Y của chú chim
float birdVy = 0.0;       // Vận tốc rơi/nhảy
int birdX = 28;           // Tọa độ X cố định của chim
int flappyScore = 0;      // Điểm số
bool flappyGameOver = false;

// Cột chướng ngại vật (Pipe)
int pipeX = 128;          // Vị trí X của cột
int pipeWidth = 14;       // Độ rộng cột
int gapHeight = 34;       // Mở rộng khoảng hở lớn hơn nữa cho dễ bay
int topPipeH = 15;        // Chiều cao ống trên
bool passedPipe = false;  // Đã tính điểm qua ống này chưa
int pipeSpeed = 2;        // Tốc độ di chuyển của cột

unsigned long lastFlappyTick = 0;
const int flappyInterval = 30; // Khoảng thời gian mỗi khung hình (ms)

void initFlappyGame() {
  birdY = 32.0;
  birdVy = 0.0;
  flappyScore = 0;
  flappyGameOver = false;
  pipeX = 128;
  topPipeH = 10 + (rand() % 20); // Chiều cao ngẫu nhiên
  passedPipe = false;
  pipeSpeed = 2;
  delay(200);
}

void resetFlappyGame() {
  initFlappyGame();
}

void runFlappyGame(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame) {
  // Nút B: Thoát về Menu chính
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inGame = false; 
    delay(200);
    return;
  }

  // Nếu Game Over, bấm phím bất kỳ để chơi lại
  if (flappyGameOver) {
    if (up || down || left || right || aBtn) {
      tone(buzzerPin, 1200, 50);
      resetFlappyGame();
    }
    return;
  }

  // Bước nhảy ngắn và nhẹ nhàng hơn rất nhiều (-2.3)
  if (up || aBtn) {
    birdVy = -2.3; 
    tone(buzzerPin, 1000, 25); 
  }

  // Cập nhật khung hình theo thời gian thực
  if (millis() - lastFlappyTick > flappyInterval) {
    lastFlappyTick = millis();

    // Trọng lực nhẹ tương ứng với bước nhảy ngắn (0.25)
    birdY += birdVy;
    birdVy += 0.25;

    // Di chuyển cột sang trái
    pipeX -= pipeSpeed;

    // Khi cột trôi qua màn hình, tạo cột mới ở bên phải
    if (pipeX < -pipeWidth) {
      pipeX = 128;
      topPipeH = 10 + (rand() % 22); 
      passedPipe = false;
      
      // Tăng tốc độ dần khi điểm cao hơn
      if (flappyScore > 0 && flappyScore % 5 == 0 && pipeSpeed < 4) {
        pipeSpeed = 3;
      }
    }

    // Tính điểm khi chim vượt qua cột
    if (!passedPipe && pipeX + pipeWidth < birdX) {
      flappyScore++;
      passedPipe = true;
      tone(buzzerPin, 1600, 40); // Tiếng ăn điểm
    }

    // --- KIỂM TRA VA CHẠM ---
    int birdW = 6;
    int birdH = 6;
    int bottomPipeY = topPipeH + gapHeight;

    // ĐÃ BỎ giới hạn chạm nóc và chạm đáy màn hình (thoải mái bay lượn)

    // Chỉ tính Game Over khi va chạm trực tiếp với ống nước trên hoặc dưới
    if (birdX + birdW > pipeX && birdX < pipeX + pipeWidth) {
      if (birdY < topPipeH || birdY + birdH > bottomPipeY) {
        flappyGameOver = true;
        tone(buzzerPin, 200, 300);
      }
    }
  }

  // --- VẼ ĐỒ HỌA LÊN MÀN HÌNH OLED ---
  display.clearDisplay();

  // Hiển thị Điểm số lớn ở giữa phía trên
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(60, 2);
  display.print(flappyScore);

  // Vẽ Ống nước trên
  display.fillRect(pipeX, 0, pipeWidth, topPipeH, SSD1306_WHITE);
  display.drawRect(pipeX - 2, topPipeH - 4, pipeWidth + 4, 4, SSD1306_WHITE);

  // Vẽ Ống nước dưới
  int bottomPipeY = topPipeH + gapHeight;
  display.fillRect(pipeX, bottomPipeY, pipeWidth, 64 - bottomPipeY, SSD1306_WHITE);
  display.drawRect(pipeX - 2, bottomPipeY, pipeWidth + 4, 4, SSD1306_WHITE);

  // Vẽ chú chim
  display.fillRect(birdX, (int)birdY, 6, 6, SSD1306_WHITE);
  display.drawPixel(birdX + 4, (int)birdY + 1, SSD1306_BLACK); // Mắt chim

  // Hộp Game Over
  if (flappyGameOver) {
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