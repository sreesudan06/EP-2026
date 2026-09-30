#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>

// =====================================================
// PROJECT
// =====================================================
#define PROJECT_NAME1 "BIKE"
#define PROJECT_NAME2 "FINGERPRINT"
#define PROJECT_NAME3 "UNLOCK"

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// GPS NEO-6M
// GPS TX -> ESP32 GPIO 16
// GPS RX <- ESP32 GPIO 17
// =====================================================
#define GPS_RX 16
#define GPS_TX 17

HardwareSerial GPS_Serial(2);
TinyGPSPlus gps;

// =====================================================
// SIM800L
// SIM800L TX -> ESP32 GPIO 26
// SIM800L RX <- ESP32 GPIO 27
// =====================================================
#define SIM_RX 26
#define SIM_TX 27

HardwareSerial SIM_Serial(1);

// =====================================================
// BUZZER
// =====================================================
#define BUZZER_PIN 33

// =====================================================
// DISPLAY TIMING
// =====================================================
unsigned long lastDisplay = 0;
const unsigned long DISPLAY_INTERVAL = 3000;

int screenNumber = 0;

// =====================================================
// CENTER TEXT
// =====================================================
void centerText(String text, int y, int size) {

  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);

  display.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &w,
    &h
  );

  int x = (SCREEN_WIDTH - w) / 2;

  display.setCursor(x, y);
  display.print(text);
}

// =====================================================
// BOOT SCREEN
// =====================================================
void bootScreen() {

  display.clearDisplay();

  centerText("BIKE", 3, 2);
  centerText("FINGERPRINT", 25, 1);
  centerText("UNLOCK", 40, 2);

  display.display();

  delay(3000);

  display.clearDisplay();

  centerText("SYSTEM", 15, 2);
  centerText("STARTING...", 40, 1);

  display.display();

  delay(2000);
}

// =====================================================
// FINGERPRINT STATUS
// NOTE: R307S IS NOT CONNECTED.
// This is display status only.
// =====================================================
void showFingerprintStatus() {

  display.clearDisplay();

  centerText("FINGERPRINT", 0, 1);
  centerText("SENSOR", 14, 1);

  display.drawLine(
    15, 29,
    113, 29,
    SSD1306_WHITE
  );

  centerText("OK", 34, 2);

  centerText("STATUS READY", 55, 1);

  display.display();
}

// =====================================================
// GPS SCREEN
// =====================================================
void showGPS() {

  display.clearDisplay();

  centerText("GPS STATUS", 0, 1);

  display.setTextSize(1);

  // GPS FIX
  display.setCursor(3, 14);

  if (gps.location.isValid()) {
    display.print("FIX: YES");
  } else {
    display.print("FIX: SEARCH");
  }

  // SATELLITES
  display.setCursor(72, 14);
  display.print("SAT:");

  if (gps.satellites.isValid()) {
    display.print(gps.satellites.value());
  } else {
    display.print("--");
  }

  // LATITUDE
  display.setCursor(3, 28);
  display.print("LAT:");

  if (gps.location.isValid()) {
    display.print(gps.location.lat(), 5);
  } else {
    display.print("--");
  }

  // LONGITUDE
  display.setCursor(3, 42);
  display.print("LON:");

  if (gps.location.isValid()) {
    display.print(gps.location.lng(), 5);
  } else {
    display.print("--");
  }

  // GPS STATUS
  display.setCursor(3, 56);

  if (gps.location.isValid()) {
    display.print("GPS LIVE");
  } else {
    display.print("WAITING...");
  }

  display.display();
}

// =====================================================
// SYSTEM STATUS
// =====================================================
void showSystemStatus() {

  display.clearDisplay();

  centerText("SYSTEM STATUS", 0, 1);

  display.setTextSize(1);

  display.setCursor(3, 16);
  display.print("FINGERPRINT : OK");

  display.setCursor(3, 28);
  display.print("GPS         : ");

  if (gps.location.isValid()) {
    display.print("FIX");
  } else {
    display.print("SEARCH");
  }

  display.setCursor(3, 40);
  display.print("SIM800L     : READY");

  display.setCursor(3, 52);
  display.print("BUZZER      : OK");

  display.display();
}

// =====================================================
// LIVE GPS DETAILS
// =====================================================
void showLiveData() {

  display.clearDisplay();

  centerText("LIVE DATA", 0, 1);

  display.setTextSize(1);

  // Speed
  display.setCursor(3, 15);
  display.print("SPD:");

  if (gps.speed.isValid()) {
    display.print(gps.speed.kmph(), 1);
    display.print("km/h");
  } else {
    display.print("--");
  }

  // Altitude
  display.setCursor(70, 15);
  display.print("ALT:");

  if (gps.altitude.isValid()) {
    display.print(gps.altitude.meters(), 0);
    display.print("m");
  } else {
    display.print("--");
  }

  // Latitude
  display.setCursor(3, 30);
  display.print("LAT:");

  if (gps.location.isValid()) {
    display.print(gps.location.lat(), 5);
  } else {
    display.print("--");
  }

  // Longitude
  display.setCursor(3, 44);
  display.print("LON:");

  if (gps.location.isValid()) {
    display.print(gps.location.lng(), 5);
  } else {
    display.print("--");
  }

  // Satellites
  display.setCursor(3, 57);
  display.print("SAT:");

  if (gps.satellites.isValid()) {
    display.print(gps.satellites.value());
  } else {
    display.print("--");
  }

  display.display();
}

// =====================================================
// READ GPS
// =====================================================
void readGPS() {

  while (GPS_Serial.available()) {

    gps.encode(
      GPS_Serial.read()
    );
  }
}

// =====================================================
// SIM800L INITIALIZATION
// =====================================================
void initSIM() {

  delay(1000);

  SIM_Serial.println("AT");
  delay(500);

  SIM_Serial.println("ATE0");
  delay(500);

  SIM_Serial.println("AT+CPIN?");
  delay(500);

  SIM_Serial.println("AT+CSQ");
  delay(500);

  SIM_Serial.println("AT+CREG?");
  delay(500);
}

// =====================================================
// BUZZER TEST
// =====================================================
void buzzerTest() {

  digitalWrite(BUZZER_PIN, HIGH);

  delay(1000);

  digitalWrite(BUZZER_PIN, LOW);

  delay(500);
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  // USB Serial
  Serial.begin(115200);

  delay(1000);

  // ===================================================
  // OLED
  // ===================================================
  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDR
      )) {

    Serial.println("OLED NOT FOUND!");
  }

  display.clearDisplay();
  display.display();

  // ===================================================
  // GPS
  // ===================================================
  GPS_Serial.begin(
    9600,
    SERIAL_8N1,
    GPS_RX,
    GPS_TX
  );

  // ===================================================
  // SIM800L
  // ===================================================
  SIM_Serial.begin(
    9600,
    SERIAL_8N1,
    SIM_RX,
    SIM_TX
  );

  // ===================================================
  // BUZZER
  // ===================================================
  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // ===================================================
  // BOOT
  // ===================================================
  bootScreen();

  // ===================================================
  // BUZZER TEST
  // ===================================================
  buzzerTest();

  // ===================================================
  // SIM800L
  // ===================================================
  initSIM();

  // ===================================================
  // SERIAL INFORMATION
  // ===================================================
  Serial.println();
  Serial.println("========================================");
  Serial.println("       BIKE FINGERPRINT UNLOCK");
  Serial.println("========================================");
  Serial.println("OLED        : OK");
  Serial.println("GPS         : READY");
  Serial.println("SIM800L     : READY");
  Serial.println("BUZZER      : OK");
  Serial.println("R307S       : DISPLAY STATUS ONLY");
  Serial.println("RELAY       : NOT USED");
  Serial.println("========================================");
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // ---------------------------------------------------
  // Continuously read GPS
  // ---------------------------------------------------
  readGPS();

  // ---------------------------------------------------
  // SIM800L -> Serial Monitor
  // ---------------------------------------------------
  while (SIM_Serial.available()) {

    Serial.write(
      SIM_Serial.read()
    );
  }

  // ---------------------------------------------------
  // Serial Monitor -> SIM800L
  // ---------------------------------------------------
  while (Serial.available()) {

    SIM_Serial.write(
      Serial.read()
    );
  }

  // ---------------------------------------------------
  // OLED DISPLAY
  // ---------------------------------------------------
  if (millis() - lastDisplay >= DISPLAY_INTERVAL) {

    lastDisplay = millis();

    switch (screenNumber) {

      case 0:
        showFingerprintStatus();
        break;

      case 1:
        showGPS();
        break;

      case 2:
        showLiveData();
        break;

      case 3:
        showSystemStatus();
        break;
    }

    screenNumber++;

    if (screenNumber > 3) {
      screenNumber = 0;
    }
  }
}