//Giao diện phần cài đặt trong hệ thống.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Adafruit_SSD1306.h>

void runSettings(Adafruit_SSD1306 &display, int buzzerPin, bool up, bool down, bool bBtn, bool &inSettings) {
  if (bBtn) {
    tone(buzzerPin, 500, 80);
    inSettings = false;
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.println(F("== CAI DAT =="));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(10, 28);
  display.println(F("Dang cap nhat..."));

  display.setCursor(0, 57);
  display.print(F("An [B] de ve Menu"));
  display.display();
}

#endif