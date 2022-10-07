#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <DNSServer.h>

// WiFi related constants
const char* SSID = "bkpvdsply";
const char* WIFI_PASSWORD = "bkpvdsply";
const IPAddress IP(192, 168, 0, 1);

// Server objects
DNSServer dns;
WebServer server(80);

const int LED_COUNT = 32;
byte image[360 * LED_COUNT][3];

void parseCsv();

// Setup function
void setup() {
    // Initialize filesystem 
    LittleFS.begin();
    parseCsv();
    // Initialize WiFi network
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID, WIFI_PASSWORD);
    // Initialize DNS
    dns.start(53, "*", IP);
    // Initialize web server
    server.enableCORS(true);

    server.on("/", HTTP_GET, [&]() {
        File file = LittleFS.open("/index.html", "r");
        String content;
        while (file.available()) {
            content += (char) file.read();
        }
        file.close();
        server.send(200, "text/html", content);
    });

    server.on("/", HTTP_POST, [&]() {
        String content;
        bool hasArgs = server.hasArg("angle");
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasArgs) {
                break;
            }
            hasArgs = server.hasArg("led-" + String(i) + "-r");
        }
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasArgs) {
                break;
            }
            hasArgs = server.hasArg("led-" + String(i) + "-g");
        }
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasArgs) {
                break;
            }
            hasArgs = server.hasArg("led-" + String(i) + "-b");
        }
        if (hasArgs) {
            content += "Valid data";
            int angle = server.arg("angle").toInt();
            File file;
            if (angle == 0) {
                file = LittleFS.open("/data/image.csv", "w");
                file.print("R:, G:, B:\n");
            } else {
                file = LittleFS.open("/data/image.csv", "a");
            }
            for (int i = 0; i < LED_COUNT; i++) {
                byte r = server.arg("led-" + String(i) + "-r").toInt();
                byte g = server.arg("led-" + String(i) + "-g").toInt();
                byte b = server.arg("led-" + String(i) + "-b").toInt();
                file.print(String(r) + ", " + String(g) + ", " + String(b) + "\n");
                image[(angle * LED_COUNT) + i][0] = r;
                image[(angle * LED_COUNT) + i][1] = g;
                image[(angle * LED_COUNT) + i][2] = b;
            }
            file.close();
        } else {
            content += "Invalid data";
        }
        server.send(200, "text/plain", content);
    });

    server.serveStatic("/data/image.csv", LittleFS, "/data/image.csv");

    server.serveStatic("/scripts/index.js", LittleFS, "/scripts/index.js");

    server.serveStatic("/styles/water.min.css", LittleFS, "/styles/water.min.css");

    server.on(UriBraces("/angle/{}"), [&]() {
        int angle = server.pathArg(0).toInt();
        String content = "R:, G:, B:\n";
        for (int i = 0; i < LED_COUNT; i++) {
            byte r = image[(angle * LED_COUNT) + i][0];
            byte g = image[(angle * LED_COUNT) + i][1];
            byte b = image[(angle * LED_COUNT) + i][2];
            content += String(r) + ", " + String(g) + ", " + String(b) + "\n";
        }
        server.send(200, "text/plain", content);
    });

    server.onNotFound([&]() {
        server.send(404, "text/plain", "Resource not found");
    });

    server.begin();
}

// Main function
void loop() {
    dns.processNextRequest();
    server.handleClient();
}

void parseCsv() {
    File file = LittleFS.open("/data/image.csv", "r");
    for (int i = -1; i < 360 * LED_COUNT; i++) {
        String line;
        if (!file.available()) {
            break;
        }
        line = file.readStringUntil('\n');
        if (i > -1) {
            image[i][0] = line.substring(0, line.indexOf(',')).toInt();
            line.remove(0, line.indexOf(',') + 1);
            image[i][1] = line.substring(0, line.indexOf(',')).toInt();
            line.remove(0, line.indexOf(',') + 1);
            image[i][2] = line.toInt();
        }
    }
    file.close();
}