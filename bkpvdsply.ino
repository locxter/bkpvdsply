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
const int SLOT_COUNT = 2;
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
byte images[SLOT_COUNT][360 * LED_COUNT][3];
unsigned long currentMicros = 0;
unsigned long revolutionPeriod = 0;
unsigned long lastRotation = 0;
unsigned long lastSlotChange = 0;
unsigned long animationInterval = 1000000;
int brightness = 128;
int displayMode = 0;
int currentSlot = 0;
int angle = 0;
int virtualAngle = 0;
int lastVirtualAngle = -1;
bool isReceiving = false;
bool isPaused = true;

// Function to load settings from the filesystem into memory
void loadSettings();

// Function to load the images from the filesystem into memory
void loadImages();

// Setup function
void setup() {
    // Initialize MPU
    Wire.begin();
    mpu.begin();
    mpu.calcGyroOffsets();
    // Initialize filesystem
    LittleFS.begin();
    loadSettings();
    loadImages();
    // Initiliaze NeoPixels
    neopixels.begin();
    neopixels.setBrightness(brightness);
    neopixels.clear();
    neopixels.show();
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
        // Check for necessary arguments
        bool hasSettingsArgs = server.hasArg("brightness") && server.hasArg("display-mode") && server.hasArg("animation-interval");
        bool hasImageArgs = server.hasArg("slot") && server.hasArg("angle");
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasImageArgs) {
                break;
            }
            hasImageArgs = server.hasArg("led-" + String(i) + "-r");
        }
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasImageArgs) {
                break;
            }
            hasImageArgs = server.hasArg("led-" + String(i) + "-g");
        }
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasImageArgs) {
                break;
            }
            hasImageArgs = server.hasArg("led-" + String(i) + "-b");
        }
        // Extract and store the received data
        if (hasSettingsArgs) {
            File file = LittleFS.open("/data/settings.csv", "w");
            file.print("Key:, Value:\n");
            brightness = server.arg("brightness").toInt();
            displayMode = server.arg("display-mode").toInt();
            animationInterval = server.arg("animation-interval").toInt();
            neopixels.setBrightness(brightness);
            neopixels.show();
            if (displayMode < SLOT_COUNT) {
                currentSlot = displayMode;
            }
            file.print("brightness, " + String(brightness) + "\n");
            file.print("display-mode, " + String(displayMode) + "\n");
            file.print("animation-interval, " + String(animationInterval) + "\n");
            file.close();
            server.send(200);
        } else if (hasImageArgs) {
            int slot = server.arg("slot").toInt();
            int angle = server.arg("angle").toInt();
            String filename = "/data/image-" + String(slot) + ".csv";
            File file;
            if (angle == 0) {
                neopixels.clear();
                neopixels.show();
                isReceiving = true;
                file = LittleFS.open(filename, "w");
                file.print("R:, G:, B:\n");
            } else {
                file = LittleFS.open(filename, "a");
            }
            for (int i = 0; i < LED_COUNT; i++) {
                byte r = server.arg("led-" + String(i) + "-r").toInt();
                byte g = server.arg("led-" + String(i) + "-g").toInt();
                byte b = server.arg("led-" + String(i) + "-b").toInt();
                images[slot][(angle * LED_COUNT) + i][0] = r;
                images[slot][(angle * LED_COUNT) + i][1] = g;
                images[slot][(angle * LED_COUNT) + i][2] = b;
                file.print(String(r) + ", " + String(g) + ", " + String(b) + "\n");
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
    server.serveStatic("/data/image-0.csv", LittleFS, "/data/image-0.csv");
    server.serveStatic("/data/image-1.csv", LittleFS, "/data/image-1.csv");
    server.serveStatic("/data/settings.csv", LittleFS, "/data/settings.csv");
    server.serveStatic("/scripts/index.js", LittleFS, "/scripts/index.js");
    server.serveStatic("/styles/dark.min.css", LittleFS, "/styles/dark.min.css");
    server.onNotFound([&]() {
        server.send(404, "text/plain", "Resource not found");
    });
    server.begin();
    lastRotation = micros();
    lastSlotChange = micros();
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
            if (displayMode == SLOT_COUNT && currentMicros - lastSlotChange > animationInterval) {
                currentSlot = (currentSlot + 1) % SLOT_COUNT;
                lastSlotChange = currentMicros;
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
                    byte r = images[currentSlot][(virtualAngle * LED_COUNT) + i][0];
                    byte g = images[currentSlot][(virtualAngle * LED_COUNT) + i][1];
                    byte b = images[currentSlot][(virtualAngle * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i, neopixels.Color(r, g, b));
                    r = images[currentSlot][(((virtualAngle + 90) % 360) * LED_COUNT) + i][0];
                    g = images[currentSlot][(((virtualAngle + 90) % 360) * LED_COUNT) + i][1];
                    b = images[currentSlot][(((virtualAngle + 90) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + LED_COUNT, neopixels.Color(r, g, b));
                    r = images[currentSlot][(((virtualAngle + 180) % 360) * LED_COUNT) + i][0];
                    g = images[currentSlot][(((virtualAngle + 180) % 360) * LED_COUNT) + i][1];
                    b = images[currentSlot][(((virtualAngle + 180) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + (LED_COUNT * 2), neopixels.Color(r, g, b));
                    r = images[currentSlot][(((virtualAngle + 270) % 360) * LED_COUNT) + i][0];
                    g = images[currentSlot][(((virtualAngle + 270) % 360) * LED_COUNT) + i][1];
                    b = images[currentSlot][(((virtualAngle + 270) % 360) * LED_COUNT) + i][2];
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

// Function to load settings from the filesystem into memory
void loadSettings() {
    File file = LittleFS.open("/data/settings.csv", "r");
    while (file.available()) {
        String line = file.readStringUntil('\n');
        String key = line.substring(0, line.indexOf(','));
        line.remove(0, line.indexOf(',') + 1);
        unsigned long value = line.toInt();
        if (key == "brightness") {
            brightness = value;
        } else if (key == "display-mode") {
            displayMode = value;
            if (displayMode < SLOT_COUNT) {
                currentSlot = displayMode;
            }
        } else if (key == "animation-interval") {
            animationInterval = value;
        }
    }
    file.close();
}

// Function to load the images from the filesystem into memory
void loadImages() {
    for (int i = 0; i < SLOT_COUNT; i++) {
        String filename = "/data/image-" + String(i) + ".csv";
        File file = LittleFS.open(filename, "r");
        for (int j = -1; j < 360 * LED_COUNT; j++) {
            String line;
            if (!file.available()) {
                break;
            }
            line = file.readStringUntil('\n');
            if (j > -1) {
                images[i][j][0] = line.substring(0, line.indexOf(',')).toInt();
                line.remove(0, line.indexOf(',') + 1);
                images[i][j][1] = line.substring(0, line.indexOf(',')).toInt();
                line.remove(0, line.indexOf(',') + 1);
                images[i][j][2] = line.toInt();
            }
        }
        file.close();
    }
}
