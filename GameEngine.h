//Định nghĩa cấu trúc chuẩn cho các trò chơi.

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <Adafruit_SSD1306.h>

// Định nghĩa cấu trúc cho mỗi trò chơi trong hệ thống
struct GameItem {
  const char* name;                  // Tên hiển thị trên menu
  void (*initFunc)();                // Hàm khởi tạo game
  void (*runFunc)(bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, Adafruit_SSD1306 &display, int buzzerPin, bool &inGame); // Hàm chạy game
};

#endif