#include "BW16WiFiManager.h"

BW16WiFiManager::BW16WiFiManager(int ledR, int ledG, int ledB)
    : _ledR(ledR), _ledG(ledG), _ledB(ledB), server(nullptr), status(WL_IDLE_STATUS) {}

void BW16WiFiManager::begin() {
    Serial.begin(115200);
    while (!Serial) {;}
    pinMode(_ledR, OUTPUT);
    pinMode(_ledG, OUTPUT);
    pinMode(_ledB, OUTPUT);
    setLEDColor(false, false, false);
    preferences.begin("wifi-config", false);
    loadConfig();
    if (strlen(wifiConfig.ssid) > 0 && strlen(wifiConfig.password) > 0) {
        while (status != WL_CONNECTED) {
            Serial.print("Attempting to connect to SSID: ");
            Serial.println(wifiConfig.ssid);
            status = WiFi.begin(wifiConfig.ssid, wifiConfig.password);
            delay(10000);
        }
        setLEDColor(false, true, false); // 綠色
    } else {
        Serial.println("No saved WiFi config");
        char channel[2] = "1";
        WiFi.config(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
        bool ap_status = WiFi.apbegin((char*)"BW-16", (char*)"12345678", channel, 0);
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
            setLEDColor(true, true, false); // 黃色
        }
    }
}

void BW16WiFiManager::handleClient() {
    if (!server) return;
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
            client.println("HTTP/1.1 302 Found");
            client.println("Location: /");
            client.println("Connection: close");
            client.println();
            client.stop();
            Serial.println("Captive portal redirect sent");
            return;
        }
        if (reqLine.startsWith("POST /save")) {
            while (client.connected() && client.available()) {
                String line = client.readStringUntil('\n');
                if (line == "\r" || line == "\n" || line == "") break;
            }
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

void BW16WiFiManager::setLEDColor(bool r, bool g, bool b) {
    digitalWrite(_ledR, r ? LOW : HIGH);
    digitalWrite(_ledG, g ? LOW : HIGH);
    digitalWrite(_ledB, b ? LOW : HIGH);
}

void BW16WiFiManager::saveConfig() {
    preferences.putString("ssid", wifiConfig.ssid);
    preferences.putString("password", wifiConfig.password);
    preferences.putString("ha_ip", wifiConfig.ha_ip);
    preferences.putString("custom1", wifiConfig.custom1);
    preferences.putString("custom2", wifiConfig.custom2);
    preferences.putString("custom3", wifiConfig.custom3);
}

void BW16WiFiManager::loadConfig() {
    preferences.getString("ssid", wifiConfig.ssid, sizeof(wifiConfig.ssid));
    preferences.getString("password", wifiConfig.password, sizeof(wifiConfig.password));
    preferences.getString("ha_ip", wifiConfig.ha_ip, sizeof(wifiConfig.ha_ip));
    preferences.getString("custom1", wifiConfig.custom1, sizeof(wifiConfig.custom1));
    preferences.getString("custom2", wifiConfig.custom2, sizeof(wifiConfig.custom2));
    preferences.getString("custom3", wifiConfig.custom3, sizeof(wifiConfig.custom3));
}

bool BW16WiFiManager::connectToWiFi() {
    int attempts = 0;
    while (status != WL_CONNECTED && attempts < 3) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(wifiConfig.ssid);
        status = WiFi.begin(wifiConfig.ssid, wifiConfig.password);
        delay(10000);
        attempts++;
    }
    return (status == WL_CONNECTED);
}

void BW16WiFiManager::printWifiStatus() {
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void BW16WiFiManager::sendHTML(WiFiClient &client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
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

void BW16WiFiManager::parseAndSaveConfig(String postData) {
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
    Serial.println("Restarting...");
    NVIC_SystemReset();
}

String BW16WiFiManager::urlDecode(String str) {
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