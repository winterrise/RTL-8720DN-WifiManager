#ifndef BW16WIFIMANAGER_H
#define BW16WIFIMANAGER_H

#include <WiFi.h>
#include <Preferences.h>

class BW16WiFiManager {
public:
    struct WiFiConfig {
        char ssid[32];
        char password[64];
        char ha_ip[16];
        char custom1[32];
        char custom2[32];
        char custom3[32];
    };

    BW16WiFiManager(int ledR, int ledG, int ledB);
    void begin();
    void handleClient();
    void setLEDColor(bool r, bool g, bool b);
    void saveConfig();
    void loadConfig();
    bool connectToWiFi();
    void printWifiStatus();
    WiFiConfig wifiConfig;

private:
    int _ledR, _ledG, _ledB;
    WiFiServer* server;
    Preferences preferences;
    int status;
    void sendHTML(WiFiClient &client);
    void parseAndSaveConfig(String postData);
    String urlDecode(String str);
};

#endif // BW16WIFIMANAGER_H 