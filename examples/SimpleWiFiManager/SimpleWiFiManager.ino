#include <BW16WiFiManager.h>

// LED腳位：紅12、綠11、藍10
BW16WiFiManager wifiManager(12, 11, 10);

void setup() {
  wifiManager.begin();
}

void loop() {
  wifiManager.handleClient();
  delay(1000);
} 
