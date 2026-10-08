#ifndef WIFI_H
#define WIFI_H

#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "BanPhimAo.h"

enum WifiMenuState {
  WIFI_LIST,
  WIFI_PASS_INPUT,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_WEB_DISPLAY
};

WifiMenuState wifiState = WIFI_LIST;
int ssidCount = 0;
int selectedSSIDIndex = 0;
String selectedSSIDName = "";
String enteredPassword = "";
unsigned long wifiActionTimer = 0;

unsigned long lastScrollTime = 0;
int textScrollOffset = 0;
int lastSelectedSSID = -1;

ESP8266WebServer server(80);
bool webServerInitialized = false;

// Hàm thiết lập Web Server và giao diện trang web
void setupWebRemote(Adafruit_SSD1306 &display) {
  server.on("/", HTTP_GET, [&display]() {
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP8266 OLED Remote</title>";
    html += "<style>";
    html += "body { font-family: Arial; text-align: center; background: #222; color: #fff; margin-top: 20px; }";
    html += "canvas { background: #000; border: 4px solid #444; image-rendering: pixelated; margin: 15px auto; display: block; box-shadow: 0 0 15px rgba(0,255,0,0.2); touch-action: none; cursor: crosshair; }";
    html += "input[type=text] { padding: 10px; width: 280px; font-size: 16px; border-radius: 5px; border: 1px solid #555; background: #333; color: #00FF66; font-family: monospace; margin-bottom: 12px; }";
    html += "button { padding: 10px 18px; font-size: 15px; color: white; border: none; border-radius: 5px; cursor: pointer; font-weight: bold; margin: 4px; }";
    html += "button:hover { opacity: 0.9; }";
    html += ".btn-draw { background: #4CAF50; }";
    html += ".btn-erase { background: #FF9800; }";
    html += ".btn-danger { background: #f44336; }";
    html += ".btn-sync { background: #2196F3; font-size: 18px; padding: 14px 28px; width: 280px; display: block; margin: 15px auto; }";
    html += ".active-tool { border: 3px solid #fff; box-shadow: 0 0 10px #fff; }";
    html += "div.toolbar { margin-bottom: 5px; }";
    html += "</style></head><body>";
    html += "<h2>ESP8266 OLED Remote Canvas</h2>";
    
    // Ô nhập chữ
    html += "<div>";
    html += "<input type='text' id='msg' placeholder='Nhap chu o day...' oninput='updateVirtualScreen()'><br>";
    html += "</div>";

    // Thanh công cụ
    html += "<div class='toolbar'>";
    html += "<button id='btnDraw' class='btn-draw active-tool' onclick='setTool(\"draw\")'>Vẽ</button>";
    html += "<button id='btnErase' class='btn-erase' onclick='setTool(\"erase\")'>Tẩy</button>";
    html += "<button class='btn-danger' onclick='clearScreen()'>Xóa sạch</button>";
    html += "</div>";

    // Màn hình ảo 128x64 trên web
    html += "<canvas id='screenCanvas' width='256' height='128'></canvas>";
    html += "<p>Chữ bắt đầu từ hàng trên cùng và ngắt dòng siêu tốc!</p>";
    
    html += "<div>";
    html += "<button class='btn-sync' onclick='syncScreen()'>Đồng bộ xuống OLED thật</button>";
    html += "</div>";

    html += "<script>";
    html += "const canvas = document.getElementById('screenCanvas');";
    html += "const ctx = canvas.getContext('2d');";
    html += "let pixels = Array(64).fill(0).map(() => Array(128).fill(0));";
    html += "let isDrawing = false;";
    html += "let currentTool = 'draw';";

    html += "function setTool(tool) {";
    html += "  currentTool = tool;";
    html += "  if(tool === 'draw') {";
    html += "    document.getElementById('btnDraw').classList.add('active-tool');";
    html += "    document.getElementById('btnErase').classList.remove('active-tool');";
    html += "  } else {";
    html += "    document.getElementById('btnErase').classList.add('active-tool');";
    html += "    document.getElementById('btnDraw').classList.remove('active-tool');";
    html += "  }";
    html += "}";

    // Thuật toán ngắt dòng theo từng ký tự
    html += "function wrapText(context, text, x, y, maxWidth, lineHeight) {";
    html += "  let line = '';";
    html += "  for(let i = 0; i < text.length; i++) {";
    html += "    let testLine = line + text[i];";
    html += "    let metrics = context.measureText(testLine);";
    html += "    if (metrics.width > maxWidth && line !== '') {";
    html += "      context.fillText(line, x, y);";
    html += "      y += lineHeight;";
    html += "      line = text[i];";
    html += "    } else {";
    html += "      line = testLine;";
    html += "    }";
    html += "  }";
    html += "  if (line !== '') {";
    html += "    context.fillText(line, x, y);";
    html += "  }";
    html += "}";

    // Hàm vẽ giả lập trên web
    html += "function drawTarget(targetCtx, scale) {";
    html += "  targetCtx.fillStyle = '#000'; targetCtx.fillRect(0, 0, targetCtx.canvas.width, targetCtx.canvas.height);";
    html += "  targetCtx.fillStyle = (scale === 2) ? '#00FF66' : '#FFF';";
    
    html += "  for(let y=0; y<64; y++) {";
    html += "    for(let x=0; x<128; x++) {";
    html += "      if(pixels[y][x]) { targetCtx.fillRect(x * scale, y * scale, scale, scale); }";
    html += "    }";
    html += "  }";

    html += "  let txt = document.getElementById('msg').value;";
    html += "  if(txt.length > 0) {";
    html += "    targetCtx.font = (scale === 2) ? '16px monospace' : '8px monospace';";
    html += "    targetCtx.fillStyle = (scale === 2) ? '#00FF66' : '#FFF';";
    html += "    let maxWidth = targetCtx.canvas.width - (scale === 2 ? 8 : 4);";
    html += "    let lineHeight = scale === 2 ? 16 : 8;";
    html += "    wrapText(targetCtx, txt, 0, scale === 2 ? 14 : 7, maxWidth, lineHeight);";
    html += "  }";
    html += "}";

    html += "function draw() { drawTarget(ctx, 2); }";
    html += "function updateVirtualScreen() { draw(); }";

    html += "function handleInteraction(e) {";
    html += "  const rect = canvas.getBoundingClientRect();";
    html += "  let clientX = e.clientX;";
    html += "  let clientY = e.clientY;";
    html += "  if(e.touches && e.touches.length > 0) {";
    html += "    clientX = e.touches[0].clientX;";
    html += "    clientY = e.touches[0].clientY;";
    html += "  }";
    html += "  const x = Math.floor((clientX - rect.left) / 2);";
    html += "  const y = Math.floor((clientY - rect.top) / 2);";
    html += "  if(x >= 0 && x < 128 && y >= 0 && y < 64) {";
    html += "    if(currentTool === 'draw') { pixels[y][x] = 1; }";
    html += "    else { pixels[y][x] = 0; }";
    html += "    draw();";
    html += "  }";
    html += "}";

    html += "canvas.addEventListener('mousedown', (e) => { isDrawing = true; handleInteraction(e); });";
    html += "canvas.addEventListener('mousemove', (e) => { if(isDrawing) handleInteraction(e); });";
    html += "window.addEventListener('mouseup', () => { isDrawing = false; });";

    html += "canvas.addEventListener('touchstart', (e) => { isDrawing = true; handleInteraction(e); e.preventDefault(); });";
    html += "canvas.addEventListener('touchmove', (e) => { if(isDrawing) handleInteraction(e); e.preventDefault(); });";
    html += "window.addEventListener('touchend', () => { isDrawing = false; });";

    html += "function clearScreen() {";
    html += "  pixels = Array(64).fill(0).map(() => Array(128).fill(0));";
    html += "  document.getElementById('msg').value = '';";
    html += "  draw();";
    html += "}";

    html += "function syncScreen() {";
    html += "  let offScreen = document.createElement('canvas');";
    html += "  offScreen.width = 128; offScreen.height = 64;";
    html += "  let offCtx = offScreen.getContext('2d');";
    html += "  drawTarget(offCtx, 1);";

    html += "  let imgData = offCtx.getImageData(0, 0, 128, 64);";
    html += "  let data = '';";
    html += "  for (let i = 0; i < imgData.data.length; i += 4) {";
    html += "    let avg = (imgData.data[i] + imgData.data[i+1] + imgData.data[i+2]) / 3;";
    html += "    data += (avg > 128) ? '1' : '0';";
    html += "  }";
    
    html += "  let syncBtn = document.querySelector('.btn-sync');";
    html += "  syncBtn.innerText = 'Đang đồng bộ...';";
    html += "  fetch('/syncOLED', { method: 'POST', body: data })";
    html += "    .then(response => {";
    html += "       syncBtn.innerText = 'Đã đồng bộ thành công!';";
    html += "       setTimeout(() => { syncBtn.innerText = 'Đồng bộ xuống OLED thật'; }, 2000);";
    html += "    }).catch(err => {";
    html += "       syncBtn.innerText = 'Lỗi đồng bộ!';";
    html += "    });";
    html += "}";

    html += "draw();";
    html += "</script></body></html>";
    
    server.send(200, "text/html", html);
  });

  server.on("/syncOLED", HTTP_POST, [&display]() {
    String payload = server.arg("plain");
    if (payload.length() >= 128 * 64) {
      display.clearDisplay();
      for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 128; x++) {
          int index = y * 128 + x;
          if (payload[index] == '1') {
            display.drawPixel(x, y, SSD1306_WHITE);
          }
        }
      }
      display.display();
      wifiState = WIFI_WEB_DISPLAY; 
    }
    server.send(200, "text/plain", "Success");
  });

  server.begin();
  webServerInitialized = true;
}

void runWifiMenu(Adafruit_SSD1306 &display, int buzzerPin, bool up, bool down, bool left, bool right, bool aBtn, bool bBtn, int &currentPage) {
  
  if (webServerInitialized) {
    server.handleClient();
  }

  // 1. DANH SÁCH WIFI (Chỉ bắt đầu quét khi người dùng vào menu WiFi)
  if (wifiState == WIFI_LIST) {
    if (right) {
      tone(buzzerPin, 1000, 50);
      currentPage = (currentPage + 1) % 3;
      delay(200);
      return;
    }
    if (left) {
      tone(buzzerPin, 1000, 50);
      currentPage = (currentPage - 1 + 3) % 3;
      delay(200);
      return;
    }

    if (bBtn) {
      tone(buzzerPin, 900, 50);
      ssidCount = 0; 
      selectedSSIDIndex = 0;
      delay(200);
      return;
    }

    if (ssidCount == 0) {
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      ssidCount = WiFi.scanNetworks();
      selectedSSIDIndex = 0;
      lastSelectedSSID = 0;
      textScrollOffset = 0;
    }

    if (up) {
      if (ssidCount > 0) {
        selectedSSIDIndex--;
        if (selectedSSIDIndex < 0) selectedSSIDIndex = ssidCount - 1;
        tone(buzzerPin, 800, 30);
        delay(150);
      }
    }
    if (down) {
      if (ssidCount > 0) {
        selectedSSIDIndex++;
        if (selectedSSIDIndex >= ssidCount) selectedSSIDIndex = 0;
        tone(buzzerPin, 800, 30);
        delay(150);
      }
    }

    if (selectedSSIDIndex != lastSelectedSSID) {
      lastSelectedSSID = selectedSSIDIndex;
      textScrollOffset = 0;
      lastScrollTime = millis();
    }

    if (millis() - lastScrollTime > 250) {
      lastScrollTime = millis();
      textScrollOffset++;
    }

    if (aBtn && ssidCount > 0) {
      tone(buzzerPin, 1200, 50);
      selectedSSIDName = WiFi.SSID(selectedSSIDIndex);
      enteredPassword = "";
      initVirtualKeyboard();
      wifiState = WIFI_PASS_INPUT;
      delay(200);
      return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.print(F("== QUAN LY WIFI =="));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    if (ssidCount == 0) {
      display.setCursor(12, 25);
      display.print(F("Dang quet mang..."));
    } else {
      int startIndex = 0;
      if (selectedSSIDIndex >= 3) startIndex = selectedSSIDIndex - 2;
      
      int yPos = 14;
      for (int i = startIndex; i < startIndex + 4 && i < ssidCount; i++) {
        String ssid = WiFi.SSID(i);
        int maxChars = 17;

        if (i == selectedSSIDIndex) {
          display.setCursor(2, yPos);
          display.print(F(">"));
          display.setCursor(12, yPos);

          if (ssid.length() > maxChars) {
            int maxOffset = ssid.length() - maxChars + 4;
            int currentPos = textScrollOffset % maxOffset;
            if (currentPos > ssid.length() - maxChars) {
              currentPos = ssid.length() - maxChars;
            }
            display.print(ssid.substring(currentPos, currentPos + maxChars));
          } else {
            display.print(ssid);
          }
        } else {
          display.setCursor(12, yPos);
          if (ssid.length() > maxChars) {
            display.print(ssid.substring(0, maxChars - 1) + "..");
          } else {
            display.print(ssid);
          }
        }
        yPos += 10;
      }
    }

    display.setCursor(0, 57);
    display.print(F("U/D:Chon A:Nhap B:Quet"));
    display.display();
  }
  
  // 2. TRẠNG THÁI NHẬP MẬT KHẨU
  else if (wifiState == WIFI_PASS_INPUT) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.print(F("== NHAP MAT KHAU =="));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(4, 13);
    display.print(F("SSID: "));
    display.print(selectedSSIDName.substring(0, 14));

    display.setCursor(4, 25);
    display.print(F("Pass: "));
    display.print(enteredPassword);

    int kbResult = runVirtualKeyboard(display, buzzerPin, up, down, left, right, aBtn, bBtn, enteredPassword);
    
    if (kbResult == 1) {
      wifiState = WIFI_CONNECTING;
      WiFi.begin(selectedSSIDName.c_str(), enteredPassword.c_str());
      wifiActionTimer = millis();
      return;
    } else if (kbResult == -1) {
      wifiState = WIFI_LIST;
      return;
    }

    display.setCursor(0, 57);
    display.print(F("L/R:Chon A:Them U:Ket D:Doi"));
    display.display();
  }
  
  // 3. ĐANG KẾT NỐI
  else if (wifiState == WIFI_CONNECTING) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.print(F("== KET NOI WIFI =="));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(14, 22);
    display.print(F("Dang ket noi..."));
    display.setCursor(14, 35);
    display.print(selectedSSIDName);

    if (WiFi.status() == WL_CONNECTED) {
      wifiState = WIFI_CONNECTED;
      tone(buzzerPin, 1800, 100);
      if (!webServerInitialized) {
        setupWebRemote(display);
      }
    } else if (millis() - wifiActionTimer > 8000) {
      wifiState = WIFI_LIST;
      tone(buzzerPin, 200, 200);
    }
    display.display();
  }
  
  // 4. ĐÃ KẾT NỐI THÀNH CÔNG
  else if (wifiState == WIFI_CONNECTED) {
    if (bBtn) {
      WiFi.disconnect();
      webServerInitialized = false;
      wifiState = WIFI_LIST;
      delay(200);
      return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 0);
    display.print(F("== TRANG THAI WIFI =="));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(10, 16);
    display.print(F("KET NOI THANH CONG!"));
    display.setCursor(10, 28);
    display.print(F("IP: ")); display.print(WiFi.localIP().toString());
    display.setCursor(10, 40);
    display.print(F("Mo trinh duyet ve hinh"));
    
    display.setCursor(0, 57);
    display.print(F("B: Ngat ket noi WiFi"));
    display.display();
  }

  // 5. TRẠNG THÁI HIỂN THỊ KẾT QUẢ TỪ WEB LÊN OLED THẬT
  else if (wifiState == WIFI_WEB_DISPLAY) {
    if (bBtn) {
      wifiState = WIFI_CONNECTED; 
      delay(200);
      return;
    }
  }
}

#endif