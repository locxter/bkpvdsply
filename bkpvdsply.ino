#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include <DNSServer.h>

// WiFi related constants
const String SSID = "bkpvdsply";
const String PASSWORD = "bkpvdsply";
const IPAddress IP(192, 168, 0, 1);

// Server objects
DNSServer dnsServer;
ESP8266WebServer webServer(80);

byte image[12960][3];

void parseCsv() {
    File file = LittleFS.open("/data/image.csv", "r");
    for (int i = -1; i < 12960; i++) {
        if (!file.available()) {
            break;
        }
        String buffer = file.readStringUntil('\n');
        if (i > -1) {
            image[i][0] = buffer.substring(0, buffer.indexOf(',')).toInt();
            buffer.remove(0, buffer.indexOf(',') + 1);
            image[i][1] = buffer.substring(0, buffer.indexOf(',')).toInt();
            buffer.remove(0, buffer.indexOf(',') + 1);
            image[i][2] = buffer.toInt();
        }
    }
    file.close();
}

unsigned long test() {
    int angle = 359;
    int i = 0;
    File file = LittleFS.open("/data/image.csv", "r");
    unsigned long start = millis();
    while (file.available()) {
        String buffer = file.readStringUntil('\n');
        if (i > 0 && i > angle * 36 && i < ((angle + 1) * 36) + 1) {
            int r = buffer.substring(0, buffer.indexOf(',')).toInt();
            buffer.remove(0, buffer.indexOf(',') + 1);
            int g = buffer.substring(0, buffer.indexOf(',')).toInt();
            buffer.remove(0, buffer.indexOf(',') + 1);
            int b = buffer.toInt();
        }
        i++;
    }
    file.close();
    return millis() - start;
}

// Setup function
void setup() {
    // Initialize filesystem 
    LittleFS.begin();
    // Initialize WiFi network
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID, PASSWORD);
    // Initialize DNS
    dnsServer.start(53, "*", IP);
    // Initialize web server
    webServer.enableCORS(true);
    webServer.on("/", HTTP_GET, [&]() {
        File file = LittleFS.open("/index.html", "r");
        String content;
        while (file.available()) {
            content += (char) file.read();
        }
        file.close();
        webServer.send(200, "text/html", content);
    });
    webServer.on("/", HTTP_POST, [&]() {
        String content;
        if (webServer.hasArg("angle") && webServer.hasArg("led-0-r") && webServer.hasArg("led-0-g") && webServer.hasArg("led-0-b") && webServer.hasArg("led-1-r") && webServer.hasArg("led-1-g") && webServer.hasArg("led-1-b") && webServer.hasArg("led-2-r") && webServer.hasArg("led-2-g") && webServer.hasArg("led-2-b") && webServer.hasArg("led-3-r") && webServer.hasArg("led-3-g") && webServer.hasArg("led-3-b") && webServer.hasArg("led-4-r") && webServer.hasArg("led-4-g") && webServer.hasArg("led-4-b") && webServer.hasArg("led-5-r") && webServer.hasArg("led-5-g") && webServer.hasArg("led-5-b") && webServer.hasArg("led-6-r") && webServer.hasArg("led-6-g") && webServer.hasArg("led-6-b") && webServer.hasArg("led-7-r") && webServer.hasArg("led-7-g") && webServer.hasArg("led-7-b") && webServer.hasArg("led-8-r") && webServer.hasArg("led-8-g") && webServer.hasArg("led-8-b") && webServer.hasArg("led-9-r") && webServer.hasArg("led-9-g") && webServer.hasArg("led-9-b") && webServer.hasArg("led-10-r") && webServer.hasArg("led-10-g") && webServer.hasArg("led-10-b") && webServer.hasArg("led-11-r") && webServer.hasArg("led-11-g") && webServer.hasArg("led-11-b") && webServer.hasArg("led-12-r") && webServer.hasArg("led-12-g") && webServer.hasArg("led-12-b") && webServer.hasArg("led-13-r") && webServer.hasArg("led-13-g") && webServer.hasArg("led-13-b") && webServer.hasArg("led-14-r") && webServer.hasArg("led-14-g") && webServer.hasArg("led-14-b") && webServer.hasArg("led-15-r") && webServer.hasArg("led-15-g") && webServer.hasArg("led-15-b") && webServer.hasArg("led-16-r") && webServer.hasArg("led-16-g") && webServer.hasArg("led-16-b") && webServer.hasArg("led-17-r") && webServer.hasArg("led-17-g") && webServer.hasArg("led-17-b") && webServer.hasArg("led-18-r") && webServer.hasArg("led-18-g") && webServer.hasArg("led-18-b") && webServer.hasArg("led-19-r") && webServer.hasArg("led-19-g") && webServer.hasArg("led-19-b") && webServer.hasArg("led-20-r") && webServer.hasArg("led-20-g") && webServer.hasArg("led-20-b") && webServer.hasArg("led-21-r") && webServer.hasArg("led-21-g") && webServer.hasArg("led-21-b") && webServer.hasArg("led-22-r") && webServer.hasArg("led-22-g") && webServer.hasArg("led-22-b") && webServer.hasArg("led-23-r") && webServer.hasArg("led-23-g") && webServer.hasArg("led-23-b") && webServer.hasArg("led-24-r") && webServer.hasArg("led-24-g") && webServer.hasArg("led-24-b") && webServer.hasArg("led-25-r") && webServer.hasArg("led-25-g") && webServer.hasArg("led-25-b") && webServer.hasArg("led-26-r") && webServer.hasArg("led-26-g") && webServer.hasArg("led-26-b") && webServer.hasArg("led-27-r") && webServer.hasArg("led-27-g") && webServer.hasArg("led-27-b") && webServer.hasArg("led-28-r") && webServer.hasArg("led-28-g") && webServer.hasArg("led-28-b") && webServer.hasArg("led-29-r") && webServer.hasArg("led-29-g") && webServer.hasArg("led-29-b") && webServer.hasArg("led-30-r") && webServer.hasArg("led-30-g") && webServer.hasArg("led-30-b") && webServer.hasArg("led-31-r") && webServer.hasArg("led-31-g") && webServer.hasArg("led-31-b") && webServer.hasArg("led-32-r") && webServer.hasArg("led-32-g") && webServer.hasArg("led-32-b") && webServer.hasArg("led-33-r") && webServer.hasArg("led-33-g") && webServer.hasArg("led-33-b") && webServer.hasArg("led-34-r") && webServer.hasArg("led-34-g") && webServer.hasArg("led-34-b") && webServer.hasArg("led-35-r") && webServer.hasArg("led-35-g") && webServer.hasArg("led-35-b")) {
            content += "Valid data";
            int angle = webServer.arg("angle").toInt();
            File file;
            if (angle == 0) {
                file = LittleFS.open("/data/image.csv", "w");
                file.print("R:, G:, B:\n");
            } else {
                file = LittleFS.open("/data/image.csv", "a");
            }
            for (int i = 0; i < 36; i++) {
                int r = webServer.arg("led-" + String(i) + "-r").toInt();
                int g = webServer.arg("led-" + String(i) + "-g").toInt();
                int b = webServer.arg("led-" + String(i) + "-b").toInt();
                file.print(String(r) + ", " + String(g) + ", " + String(b) + "\n");
            }
            file.close();
        } else {
            content += "Invalid data";
        }
        webServer.send(200, "text/plain", content);
    });
    webServer.serveStatic("/data/image.csv", LittleFS, "/data/image.csv");
    webServer.serveStatic("/scripts/index.js", LittleFS, "/scripts/index.js");
    webServer.serveStatic("/styles/water.min.css", LittleFS, "/styles/water.min.css");

    webServer.on("/time", [&]() {
        String content = "Time: " + String(test());
        webServer.send(200, "text/plain", content);
    });

    webServer.on(UriBraces("/angle/{}"), [&]() {
        int angle = webServer.pathArg(0).toInt();
        int i = 0;
        File file = LittleFS.open("/data/image.csv", "r");
        String content;
        while (file.available()) {
            String buffer = file.readStringUntil('\n');
            if (i > 0 && i > angle * 36 && i < ((angle + 1) * 36) + 1) {
                content += "Original: " + buffer + "\n";
                int r = buffer.substring(0, buffer.indexOf(',')).toInt();
                buffer.remove(0, buffer.indexOf(',') + 1);
                int g = buffer.substring(0, buffer.indexOf(',')).toInt();
                buffer.remove(0, buffer.indexOf(',') + 1);
                int b = buffer.toInt();
                content += "Parsed: " + String(r) + ", " + String(g) + ", " + String(b) + "\n";
            }
            i++;
        }
        file.close();
        webServer.send(200, "text/plain", content);
    });

    webServer.onNotFound([&]() {
        webServer.send(404, "text/plain", "Resource not found");
    });
    webServer.begin();
}

// Main function
void loop() {
    dnsServer.processNextRequest();
    webServer.handleClient();
}