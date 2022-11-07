#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <DNSServer.h>

// LED and WiFi related constants
const int NEOPIXELS_PIN = 5;
const int LED_COUNT = 32;
const char* SSID = "bkpvdsply";
const char* WIFI_PASSWORD = "bkpvdsply";
const IPAddress IP(192, 168, 0, 1);

// MPU, NeoPixel and server objects
MPU6050 mpu(Wire);
Adafruit_NeoPixel neopixels(LED_COUNT * 4, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);
DNSServer dns;
WebServer server(80);

// Other variables
byte image[360 * LED_COUNT][3];
unsigned long currentMicros = 0;
unsigned long revolutionPeriod = 0;
unsigned long lastRotation = 0;
int angle = 0;
int virtualAngle = 0;
int lastVirtualAngle = -1;
bool isReceiving = false;
bool isPaused = true;

// Function to load the image from the filesystem into memory
void loadImage();

// Setup function
void setup() {
    // Initialize MPU
    Wire.begin();
    mpu.begin();
    mpu.calcGyroOffsets();
    // Initiliaze NeoPixels
    neopixels.begin();
    neopixels.setBrightness(128);
    neopixels.clear();
    neopixels.show();
    // Initialize filesystem
    LittleFS.begin();
    loadImage();
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
        bool hasArgs = server.hasArg("angle");
        // Check for necessary arguments
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
        // Extract and store the received data
        if (hasArgs) {
            int angle = server.arg("angle").toInt();
            File file;
            if (angle == 0) {
                neopixels.clear();
                neopixels.show();
                isReceiving = true;
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
            if (angle == 359) {
                isReceiving = false;
            }
            server.send(200);
        } else {
            server.send(404, "text/plain", "Invalid request");
        }
    });
    server.serveStatic("/data/image.csv", LittleFS, "/data/image.csv");
    server.serveStatic("/scripts/index.js", LittleFS, "/scripts/index.js");
    server.serveStatic("/styles/dark.min.css", LittleFS, "/styles/dark.min.css");
    server.onNotFound([&]() {
        server.send(404, "text/plain", "Resource not found");
    });
    server.begin();
    lastRotation = micros();
}

// Main function
void loop() {
    if (!isReceiving) {
        mpu.update();
        angle = ((int) round(mpu.getAccAngleX()) + 360) % 360;
        currentMicros = micros();
        // Calculate revolution period and update other data on every rotation when the user is pedalling
        if ((angle >= 353 || angle <= 7) && currentMicros - lastRotation >= 150000 && currentMicros - lastRotation <= 1500000) {
            if (isPaused) {
                isPaused = false;
            }
            revolutionPeriod = currentMicros - lastRotation;
            lastRotation = currentMicros;
            virtualAngle = 0;
            lastVirtualAngle = -1;
        }
        // Pause display when user stops pedalling
        if (currentMicros - lastRotation > 1500000) {
            if (!isPaused) {
                neopixels.clear();
                neopixels.show();
                isPaused = true;
            }
            lastRotation = currentMicros;
        }
        if (!isPaused) {
            virtualAngle = (int) round((currentMicros - lastRotation) / (revolutionPeriod / 360.0)) % 360;
            // Display an angle of the image
            if (virtualAngle != lastVirtualAngle) {
                for (int i = 0; i < LED_COUNT; i++) {
                    byte r = image[(virtualAngle * LED_COUNT) + i][0];
                    byte g = image[(virtualAngle * LED_COUNT) + i][1];
                    byte b = image[(virtualAngle * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i, neopixels.Color(r, g, b));
                    r = image[(((virtualAngle + 90) % 360) * LED_COUNT) + i][0];
                    g = image[(((virtualAngle + 90) % 360) * LED_COUNT) + i][1];
                    b = image[(((virtualAngle + 90) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + LED_COUNT, neopixels.Color(r, g, b));
                    r = image[(((virtualAngle + 180) % 360) * LED_COUNT) + i][0];
                    g = image[(((virtualAngle + 180) % 360) * LED_COUNT) + i][1];
                    b = image[(((virtualAngle + 180) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + (LED_COUNT * 2), neopixels.Color(r, g, b));
                    r = image[(((virtualAngle + 270) % 360) * LED_COUNT) + i][0];
                    g = image[(((virtualAngle + 270) % 360) * LED_COUNT) + i][1];
                    b = image[(((virtualAngle + 270) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + (LED_COUNT * 3), neopixels.Color(r, g, b));
                }
                neopixels.show();
                lastVirtualAngle = virtualAngle;
            }
        }
    }
    dns.processNextRequest();
    server.handleClient();
}

// Function to load the image from the filesystem into memory
void loadImage() {
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
