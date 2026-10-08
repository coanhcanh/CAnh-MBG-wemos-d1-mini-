#ifndef BAN_PHIM_AO_H
#define BAN_PHIM_AO_H

#include <Adafruit_SSD1306.h>

// 4 hàng ký tự: Chữ thường, Chữ hoa, Chữ số, Ký tự đặc biệt
const char* rowLists[4] = {
  "abcdefghijklmnopqrstuvwxyz",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
  "0123456789",
  "~!@#$%^&*()_+-={}[]|\\:;\"'<>?,./"
};
const int rowLengths[4] = {26, 26, 10, 31};
const char* rowLabels[4] = {"abc", "ABC", "123", "SYM"};

int selectedRow = 0;               // Hàng hiện tại đang chọn (0 đến 3)
int charCursors[4] = {0, 0, 0, 0}; // Lưu vị trí con trỏ riêng cho từng hàng

// Khởi tạo hoặc reset lại bàn phím khi bắt đầu mở
void initVirtualKeyboard() {
  selectedRow = 0;
  for (int i = 0; i < 4; i++) {
    charCursors[i] = 0;
  }
}

// Xử lý logic và hiển thị bàn phím ảo
// Giá trị trả về: 0 (tiếp tục nhập), 1 (bấm Up để xác nhận/kết nối), -1 (bấm B khi rỗng để thoát)
int runVirtualKeyboard(Adafruit_SSD1306 &display, int buzzerPin, bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, String &enteredPassword) {
  
  if (right) {
    charCursors[selectedRow]++;
    if (charCursors[selectedRow] >= rowLengths[selectedRow]) {
      charCursors[selectedRow] = 0;
    }
    
    // Đồng bộ vị trí giữa hàng chữ thường (0) và chữ hoa (1)
    if (selectedRow == 0 || selectedRow == 1) {
      charCursors[0] = charCursors[selectedRow];
      charCursors[1] = charCursors[selectedRow];
    }

    tone(buzzerPin, 800, 20);
    delay(120);
  }
  
  if (left) {
    charCursors[selectedRow]--;
    if (charCursors[selectedRow] < 0) {
      charCursors[selectedRow] = rowLengths[selectedRow] - 1;
    }
    
    // Đồng bộ vị trí giữa hàng chữ thường (0) và chữ hoa (1)
    if (selectedRow == 0 || selectedRow == 1) {
      charCursors[0] = charCursors[selectedRow];
      charCursors[1] = charCursors[selectedRow];
    }

    tone(buzzerPin, 800, 20);
    delay(120);
  }

  // Nút Down (d) để chuyển đổi qua lại giữa 4 hàng
  if (down) {
    selectedRow = (selectedRow + 1) % 4;
    tone(buzzerPin, 1000, 40);
    delay(200);
  }

  // Nút Up (u) để xác nhận (tiến hành kết nối)
  if (up) {
    tone(buzzerPin, 1500, 80);
    delay(200);
    return 1; 
  }

  // Nút A để thêm ký tự đang chọn vào mật khẩu
  if (aBtn) {
    char c = rowLists[selectedRow][charCursors[selectedRow]];
    enteredPassword += c;
    tone(buzzerPin, 1200, 40);
    delay(200);
  }

  // Nút B để xóa lùi hoặc thoát
  if (bBtn) {
    if (enteredPassword.length() > 0) {
      enteredPassword.remove(enteredPassword.length() - 1);
      tone(buzzerPin, 600, 50);
      delay(200);
    } else {
      tone(buzzerPin, 500, 50);
      delay(200);
      return -1; // Tín hiệu quay lại danh sách WiFi
    }
  }

  // Vẽ khung giao diện bàn phím lên màn hình OLED
  display.drawRect(0, 37, 128, 17, SSD1306_WHITE);
  display.setCursor(4, 42);
  display.print(rowLabels[selectedRow]);
  display.print(F(":[ "));
  
  int curPos = charCursors[selectedRow];
  int len = rowLengths[selectedRow];
  const char* activeList = rowLists[selectedRow];

  for (int offset = -2; offset <= 2; offset++) {
    int idx = curPos + offset;
    while (idx < 0) idx += len;
    while (idx >= len) idx -= len;
    
    display.print(activeList[idx]);
    display.print(" ");
  }
  display.print(F("]"));

  return 0; // Tiếp tục trạng thái nhập bình thường
}

#endif