#include <WiFi.h>
#include <Preferences.h>
#include "wificonfig.h"



// 建立 WiFiServer 實例
WiFiServer* server = nullptr;

// 建立 Preferences 實例
Preferences preferences;



// 函式宣告
void setupWiFiAP();
void setupWiFiStation();
void handleClient();
void saveConfig();
void loadConfig();
bool connectToWiFi();
void printWifiStatus();
void sendHTML(WiFiClient &client);
void parseAndSaveConfig(String postData);
String urlDecode(String str);

int status = WL_IDLE_STATUS;  // WiFi 狀態指示器

void setup() {
  // 初始化序列埠
  Serial.begin(115200);
  while (!Serial) {
    ; // 等待序列埠連接
  }
  
  // 初始化 Preferences
  preferences.begin("wifi-config", false);
  
  // 載入設定
  loadConfig();
  
  // 檢查是否有儲存的 WiFi 設定
  if (strlen(wifiConfig.ssid) > 0 && strlen(wifiConfig.password) > 0) {
    // 嘗試連線到已儲存的 WiFi
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(wifiConfig.ssid);
      status = WiFi.begin(wifiConfig.ssid, wifiConfig.password);
      delay(10000);  // 等待 10 秒
    }
    // 連線成功後，不啟動 HTTP server
  } else {
    Serial.println("No saved WiFi config");
    // 啟動 AP 模式（正確順序）
    char channel[2] = "1";
    WiFi.config(AP_IP, AP_GATEWAY, AP_SUBNET); // 先設定IP
    bool ap_status = WiFi.apbegin((char*)AP_SSID, (char*)AP_PASSWORD, channel, 0); // 再啟動AP
    if (!ap_status) {
      Serial.println("Failed to start AP mode");
    } else {
      Serial.println("AP Mode started");
      Serial.print("AP IP address: ");
      Serial.println(WiFi.localIP());
      server = new WiFiServer(80);
      server->begin();
      Serial.println("HTTP server started");
      printWifiStatus();
    }
  }
}

void loop() {
  if (server) handleClient();
  delay(1000);
}

void printWifiStatus() {
  // 顯示連接的網路 SSID
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  // 顯示 WiFi IP 位址
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // 顯示接收到的訊號強度
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// 載入設定
void loadConfig() {
  preferences.getString("ssid", wifiConfig.ssid, sizeof(wifiConfig.ssid));
  preferences.getString("password", wifiConfig.password, sizeof(wifiConfig.password));
  preferences.getString("ha_ip", wifiConfig.ha_ip, sizeof(wifiConfig.ha_ip));
  preferences.getString("custom1", wifiConfig.custom1, sizeof(wifiConfig.custom1));
  preferences.getString("custom2", wifiConfig.custom2, sizeof(wifiConfig.custom2));
  preferences.getString("custom3", wifiConfig.custom3, sizeof(wifiConfig.custom3));
}

// 儲存設定
void saveConfig() {
  preferences.putString("ssid", wifiConfig.ssid);
  preferences.putString("password", wifiConfig.password);
  preferences.putString("ha_ip", wifiConfig.ha_ip);
  preferences.putString("custom1", wifiConfig.custom1);
  preferences.putString("custom2", wifiConfig.custom2);
  preferences.putString("custom3", wifiConfig.custom3);
}

// 連線到 WiFi
bool connectToWiFi() {
  int status = WL_IDLE_STATUS;
  int attempts = 0;
  
  while (status != WL_CONNECTED && attempts < 3) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifiConfig.ssid);
    status = WiFi.begin(wifiConfig.ssid, wifiConfig.password);
    delay(10000);  // 等待 10 秒
    attempts++;
  }
  
  return (status == WL_CONNECTED);
}

// 設定 AP 模式
void setupWiFiAP() {
  // 初始化 WiFi
  WiFi.begin(NULL, NULL);
  delay(2000);
  
  // 設定 IP 位址
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_ip, gateway, subnet);
  
  // 啟動 AP
  char channel[2] = "1";
  bool status = WiFi.apbegin((char*)AP_SSID, (char*)AP_PASSWORD, channel, 0);
  
  if (status) {
    server->begin();
    Serial.println("AP Mode started");
    printWifiStatus();
  } else {
    Serial.println("Failed to start AP mode");
  }
}

// 設定 Station 模式
void setupWiFiStation() {
  // 初始化 WiFi
  WiFi.begin(NULL, NULL);
  delay(2000);
  
  // 設定 IP 位址
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_ip, gateway, subnet);
  
  // 啟動 AP
  char channel[2] = "1";
  bool status = WiFi.apbegin((char*)AP_SSID, (char*)AP_PASSWORD, channel, 0);
  
  if (status) {
    server->begin();
    Serial.println("Station Mode started");
    printWifiStatus();
  } else {
    Serial.println("Failed to start Station mode");
  }
}

// 發送 HTML 內容
void sendHTML(WiFiClient &client) {
  // 發送 HTTP 回應標頭
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  // 發送 HTML 內容
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<title>BW-16 WiFi Manager</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; margin: 20px; font-size: 3em; }");
  client.println("h1 { color: #333; }");
  client.println("form { max-width: 500px; }");
  client.println("select, input { width: 100%; padding: 8px; margin: 5px 0; }");
  client.println("input[type=submit] { background-color: #4CAF50; color: white; padding: 10px; border: none; cursor: pointer; }");
  client.println("</style>");
  client.println("<script>");
  client.println("function confirmSubmit() {");
  client.println("  var ssid = document.getElementById('ssid').value;");
  client.println("  var password = document.getElementsByName('password')[0].value;");
  client.println("  var ha_ip = document.getElementsByName('ha_ip')[0].value;");
  client.println("  var custom1 = document.getElementsByName('custom1')[0].value;");
  client.println("  var custom2 = document.getElementsByName('custom2')[0].value;");
  client.println("  var custom3 = document.getElementsByName('custom3')[0].value;");
  client.println("  var msg = '請確認您的設定：\\n' +");
  client.println("    'SSID: ' + ssid + '\\n' +");
  client.println("    'Password: ' + password + '\\n' +");
  client.println("    'Home Assistant IP: ' + ha_ip + '\\n' +");
  client.println("    'Custom1: ' + custom1 + '\\n' +");
  client.println("    'Custom2: ' + custom2 + '\\n' +");
  client.println("    'Custom3: ' + custom3 + '\\n' +");
  client.println("    '\\n確定要送出嗎？';");
  client.println("  return confirm(msg);");
  client.println("}");
  client.println("</script>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>BW-16 WiFi Manager</h1>");
  client.println("<form action='/save' method='post' onsubmit='return confirmSubmit()'>");
  client.println("<select name='ssid' id='ssid'>");
  
  // 掃描 WiFi 網路
  Serial.println("Scanning WiFi networks");
  int n = WiFi.scanNetworks();
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks");
  
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    client.print("<option value='");
    client.print(ssid);
    client.print("'>");
    client.print(ssid);
    client.println("</option>");
  }
  
  client.println("</select><br><br>");
  client.println("Password: <input type='password' name='password'><br><br>");
  client.println("Home Assistant IP: <input type='text' name='ha_ip'><br><br>");
  client.println("Custom Field 1: <input type='text' name='custom1'><br><br>");
  client.println("Custom Field 2: <input type='text' name='custom2'><br><br>");
  client.println("Custom Field 3: <input type='text' name='custom3'><br><br>");
  client.println("<input type='submit' value='Save'>");
  client.println("</form>");
  client.println("</body>");
  client.println("</html>");
}

// 解析並儲存設定
void parseAndSaveConfig(String postData) {
  int idx;
  String key, value;
  while ((idx = postData.indexOf('&')) != -1) {
    String pair = postData.substring(0, idx);
    int eqIdx = pair.indexOf('=');
    if (eqIdx != -1) {
      key = pair.substring(0, eqIdx);
      value = urlDecode(pair.substring(eqIdx + 1));
      if (key == "ssid") strncpy(wifiConfig.ssid, value.c_str(), sizeof(wifiConfig.ssid));
      else if (key == "password") strncpy(wifiConfig.password, value.c_str(), sizeof(wifiConfig.password));
      else if (key == "ha_ip") strncpy(wifiConfig.ha_ip, value.c_str(), sizeof(wifiConfig.ha_ip));
      else if (key == "custom1") strncpy(wifiConfig.custom1, value.c_str(), sizeof(wifiConfig.custom1));
      else if (key == "custom2") strncpy(wifiConfig.custom2, value.c_str(), sizeof(wifiConfig.custom2));
      else if (key == "custom3") strncpy(wifiConfig.custom3, value.c_str(), sizeof(wifiConfig.custom3));
    }
    postData = postData.substring(idx + 1);
  }
  int eqIdx = postData.indexOf('=');
  if (eqIdx != -1) {
    key = postData.substring(0, eqIdx);
    value = urlDecode(postData.substring(eqIdx + 1));
    if (key == "ssid") strncpy(wifiConfig.ssid, value.c_str(), sizeof(wifiConfig.ssid));
    else if (key == "password") strncpy(wifiConfig.password, value.c_str(), sizeof(wifiConfig.password));
    else if (key == "ha_ip") strncpy(wifiConfig.ha_ip, value.c_str(), sizeof(wifiConfig.ha_ip));
    else if (key == "custom1") strncpy(wifiConfig.custom1, value.c_str(), sizeof(wifiConfig.custom1));
    else if (key == "custom2") strncpy(wifiConfig.custom2, value.c_str(), sizeof(wifiConfig.custom2));
    else if (key == "custom3") strncpy(wifiConfig.custom3, value.c_str(), sizeof(wifiConfig.custom3));
  }
  saveConfig();
}

// URL decode function
String urlDecode(String str) {
  String decodedString = "";
  char c, code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == '%') {
      if (i + 2 < str.length()) {
        code0 = str.charAt(i + 1);
        code1 = str.charAt(i + 2);
        if (code0 >= '0' && code0 <= '9') c = (code0 - '0') << 4;
        else if (code0 >= 'A' && code0 <= 'F') c = (code0 - 'A' + 10) << 4;
        else if (code0 >= 'a' && code0 <= 'f') c = (code0 - 'a' + 10) << 4;
        if (code1 >= '0' && code1 <= '9') c += code1 - '0';
        else if (code1 >= 'A' && code1 <= 'F') c += code1 - 'A' + 10;
        else if (code1 >= 'a' && code1 <= 'f') c += code1 - 'a' + 10;
        i += 2;
      }
    }
    decodedString += c;
  }
  return decodedString;
}

// 處理客戶端請求
void handleClient() {
  if (server == nullptr) return;
  WiFiClient client = server->available();
  if (client) {
    Serial.println("New client");
    String reqLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\r') continue;
        if (c == '\n') break;
        reqLine += c;
      }
    }
    if (reqLine.startsWith("GET /generate_204") ||
        reqLine.startsWith("GET /hotspot-detect.html") ||
        reqLine.startsWith("GET /ncsi.txt") ||
        reqLine.startsWith("GET /favicon.ico")) {
      // 導向到主頁
      client.println("HTTP/1.1 302 Found");
      client.println("Location: /");
      client.println("Connection: close");
      client.println();
      client.stop();
      Serial.println("Captive portal redirect sent");
      return;
    }
    if (reqLine.startsWith("POST /save")) {
      // 讀取 header 到空行
      while (client.connected() && client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r" || line == "\n" || line == "") break;
      }
      // 讀取 POST 資料
      String postData = "";
      while (client.connected() && client.available()) {
        char c = client.read();
        postData += c;
      }
      parseAndSaveConfig(postData);
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<html><body><h1>設定已儲存！</h1><a href='/'>返回</a></body></html>");
    } else {
      sendHTML(client);
    }
    delay(100);
    client.stop();
    Serial.println("Client disconnected");
  }
}
