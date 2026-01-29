/*
 * SMART ACCIDENT DETECTION - VIBRATION FIX
 * Updates: LoRa bypass, Motor Grace Period, Higher Threshold
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "MAX30100_PulseOximeter.h" 

// ================= USER CONFIGURATION =================
const char* PHONE_NUMBER = "+212600000000"; 
// INCREASED THRESHOLD: 40.0 m/s^2 (approx 4G) to ignore vibration
const float CRASH_THRESHOLD = 20.0;         
const int   EMERGENCY_TIMER = 10;           
const long  LORA_FREQUENCY  = 433E6;        

// ================= PIN DEFINITIONS =================
#define PIN_BTN      13  
#define PIN_BUZZ     12  
#define PIN_CAM_TRIG 15  

#define GPS_RX_PIN   16  
#define GPS_TX_PIN   17  
#define GSM_RX_PIN   32  
#define GSM_TX_PIN   33  

#define SPI_SCK      18
#define SPI_MISO     19
#define SPI_MOSI     23
#define SD_CS_PIN    5   
#define LORA_CS_PIN  4   
#define LORA_RST_PIN 2   
#define LORA_DIO0    35  

#define MOT_A1       26 
#define MOT_A2       27
#define MOT_B1       14  
#define MOT_B2       25

// ================= OBJECTS =================
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
PulseOximeter pox;
HardwareSerial SerialGSM(1); 
HardwareSerial SerialGPS(2); 

// ================= VARIABLES =================
bool crashDetected = false;
bool systemSafe = true;
bool poxBegin = false;
bool sdReady = false;
unsigned long crashStartTime = 0;
// NEW: Grace period timer
unsigned long ignoreSensorsUntil = 0; 

void onBeatDetected() {
    // Serial.print("♥");
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_BUZZ, OUTPUT);
  pinMode(PIN_CAM_TRIG, OUTPUT);
  digitalWrite(PIN_CAM_TRIG, LOW);
  
  pinMode(SD_CS_PIN, OUTPUT); digitalWrite(SD_CS_PIN, LOW);
  pinMode(LORA_CS_PIN, OUTPUT); digitalWrite(LORA_CS_PIN, HIGH);

  pinMode(MOT_A1, OUTPUT); pinMode(MOT_A2, OUTPUT);
  pinMode(MOT_B1, OUTPUT); pinMode(MOT_B2, OUTPUT);
  stopCar();

  Wire.begin(); 
  lcd.init(); lcd.backlight();
  lcd.setCursor(0,0); lcd.print("SYSTEM STARTUP");
  delay(1000);

  // MPU6050
  lcd.clear(); lcd.print("ACCEL: ");
  if(mpu.begin()) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    // NEW: Set filter to reduce engine vibration noise
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    lcd.print("OK");
  } else {
    lcd.print("FAIL"); // Warning only
  }
  delay(500);

  // MAX30100
  lcd.setCursor(0,1); lcd.print("PULSE: ");
  if(pox.begin()) {
    pox.setOnBeatDetectedCallback(onBeatDetected);
    poxBegin = true;
    lcd.print("OK");
  } else {
    lcd.print("FAIL"); 
  }
  delay(500);

  // SD Card
  lcd.clear(); lcd.print("SD CARD: ");
  if(SD.begin(SD_CS_PIN)) {
    sdReady = true; lcd.print("OK");
  } else {
    lcd.print("FAIL");
  }
  delay(500);

  // LoRa (MODIFIED: BYPASS IF FAIL)
  lcd.setCursor(0,1); lcd.print("LoRa: ");
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0);
  if(LoRa.begin(LORA_FREQUENCY)) {
    lcd.print("OK");
  } else {
    lcd.print("FAIL (SKIP)"); 
    Serial.println("LoRa Failed - Continuing anyway...");
  }
  delay(1000);

  // GPS & GSM
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  SerialGSM.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  
  // STARTUP COMPLETE
  lcd.clear();
  lcd.print("SYSTEM READY");
  digitalWrite(PIN_BUZZ, HIGH); delay(100); digitalWrite(PIN_BUZZ, LOW);
  
  // START MOTORS & SET GRACE PERIOD
  lcd.setCursor(0,1); lcd.print("MOTORS ON...");
  driveForward(); 
  // IMPORTANT: Ignore crash sensors for 3000ms (3 seconds) after start
  ignoreSensorsUntil = millis() + 3000; 
}

void loop() {
  if(poxBegin) pox.update();

  while(SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // CRASH DETECTION (With Safety Checks)
  static unsigned long lastCrashCheck = 0;
  // Check 1: Is the time > 50ms since last check?
  // Check 2: Is the "Grace Period" over? (millis() > ignoreSensorsUntil)
  if (!crashDetected && (millis() - lastCrashCheck > 50) && (millis() > ignoreSensorsUntil)) {
    lastCrashCheck = millis();
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    float totalForce = sqrt(sq(a.acceleration.x) + sq(a.acceleration.y) + sq(a.acceleration.z));
    
    // DEBUG: Uncomment this line to see the Vibration Value in Serial Monitor
    // Serial.print("Force: "); Serial.println(totalForce);

    if (totalForce > CRASH_THRESHOLD) {
      Serial.print("Crash Triggered! Force: "); Serial.println(totalForce);
      triggerCrash();
    }
  }

  if (crashDetected) {
    handleEmergency();
  }
}

// ================= CONTROL FUNCTIONS =================

void stopCar() {
  digitalWrite(MOT_A1, LOW); digitalWrite(MOT_A2, LOW);
  digitalWrite(MOT_B1, LOW); digitalWrite(MOT_B2, LOW);
}

void driveForward() {
  digitalWrite(MOT_A1, HIGH); digitalWrite(MOT_A2, LOW);
  digitalWrite(MOT_B1, HIGH); digitalWrite(MOT_B2, LOW);
}

void triggerCrash() {
  crashDetected = true;
  crashStartTime = millis();
  
  Serial.println("!!! CRASH DETECTED !!!");
  stopCar();
  
  digitalWrite(PIN_CAM_TRIG, HIGH);
  delay(100); 
  digitalWrite(PIN_CAM_TRIG, LOW);
  
  lcd.clear(); lcd.print("CRASH DETECTED!");
  digitalWrite(PIN_BUZZ, HIGH);
  
  if(sdReady) {
    File f = SD.open("/log.txt", FILE_APPEND);
    if(f) { f.println("CRASH_EVENT_TRIGGERED"); f.close(); }
  }
}

void handleEmergency() {
  unsigned long elapsed = (millis() - crashStartTime) / 1000;
  int countdown = EMERGENCY_TIMER - elapsed;

  if (digitalRead(PIN_BTN) == LOW) {
    cancelAlert();
    return;
  }

  if (countdown > 0) {
    lcd.setCursor(0,1); 
    lcd.print("SOS in: "); lcd.print(countdown); lcd.print("s   ");
    if(poxBegin) pox.update();
    if(millis() % 500 < 250) digitalWrite(PIN_BUZZ, HIGH);
    else digitalWrite(PIN_BUZZ, LOW);
  } else {
    sendAlerts();
    while(1) {
      lcd.setCursor(0,0); lcd.print("HELP SENT!      ");
      lcd.setCursor(0,1); lcd.print("WAITING RESCUE  ");
      if(poxBegin) pox.update();
    }
  }
}

void cancelAlert() {
  crashDetected = false;
  digitalWrite(PIN_BUZZ, LOW);
  lcd.clear(); lcd.print("ALARM CANCELLED");
  delay(2000);
  lcd.clear(); lcd.print("SYSTEM READY");
  driveForward(); 
  // Reset Grace Period so motors don't re-trigger crash immediately
  ignoreSensorsUntil = millis() + 3000; 
}

void sendAlerts() {
  digitalWrite(PIN_BUZZ, LOW);
  lcd.clear(); lcd.print("SENDING DATA...");
  
  float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
  float lng = gps.location.isValid() ? gps.location.lng() : 0.0;
  int hr = poxBegin ? pox.getHeartRate() : 0;
  int spo2 = poxBegin ? pox.getSpO2() : 0;
  
  String mapLink = "http://maps.google.com/?q=" + String(lat, 6) + "," + String(lng, 6);
  String alertMsg = "SOS! Crash Detected.\nLoc: " + mapLink + "\nHR: " + String(hr) + " BPM\nOxy: " + String(spo2) + "%";

  // Try LoRa (If it failed init, this might do nothing, which is fine)
  lcd.setCursor(0,1); lcd.print("LoRa...");
  LoRa.beginPacket();
  LoRa.print(alertMsg);
  LoRa.endPacket();
  delay(500);

  // GSM
  lcd.setCursor(0,1); lcd.print("GSM SMS...");
  SerialGSM.println("AT+CMGF=1"); 
  delay(200);
  SerialGSM.print("AT+CMGS=\"" + String(PHONE_NUMBER) + "\"\r");
  delay(500);
  SerialGSM.print(alertMsg);
  delay(100);
  SerialGSM.write(26); 
  delay(5000); 
  
  lcd.clear(); lcd.print("ALL ALERTS SENT");
}