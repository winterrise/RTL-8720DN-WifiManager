


// WiFi/AP 參數
#define AP_SSID "BW-16"
#define AP_PASSWORD "12345678"
#define AP_IP IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)
#define AP_CHANNEL 1

// 儲存 WiFi 設定
struct WiFiConfig {
  char ssid[32];
  char password[64];
  char ha_ip[16];
  char custom1[32];
  char custom2[32];
  char custom3[32];
} wifiConfig;

