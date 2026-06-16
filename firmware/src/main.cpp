#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

#define LED_PIN PC13
#define RELAY_PIN PA2
#define PCF8563_ADDR 0x51      // RTC I2C address
#define SHT30_ADDR 0x44         // Temperature sensor I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Time variables
int hours = 22;
int minutes = 9;
int seconds = 0;
int day = 19, month = 5, year = 2026;
unsigned long lastUpdate = 0;
float temperature = 0.0;
float humidity = 0.0;




// ========== BCD Conversion ==========
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}

byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}

// ========== PCF8563 RTC Functions ==========
void initRTC() {
  Wire.beginTransmission(PCF8563_ADDR);
  Wire.write(0x00); // Control register 1
  Wire.write(0x00); // Clear control bits, enable RTC
  Wire.endTransmission();
  
  // Check and clear VL (Voltage Low) bit
  Wire.beginTransmission(PCF8563_ADDR);
  Wire.write(0x02); // Seconds register
  Wire.endTransmission();
  
  Wire.requestFrom(PCF8563_ADDR, 1);
  byte seconds_byte = Wire.read();
  
  if (seconds_byte & 0x80) { // VL bit is set, need to reset it
    Wire.beginTransmission(PCF8563_ADDR);
    Wire.write(0x02); // Seconds register
    Wire.write(seconds_byte & 0x7F); // Clear VL bit
    Wire.endTransmission();
  }
}

void setTimeRTC(int h, int m, int s, int d, int mo, int y) {
  Wire.beginTransmission(PCF8563_ADDR);
  Wire.write(0x02); // Start at seconds register
  Wire.write(decToBcd(s) & 0x7F);      // Seconds (clear VL bit)
  Wire.write(decToBcd(m) & 0x7F);      // Minutes
  Wire.write(decToBcd(h) & 0x3F);      // Hours (24-hour format)
  Wire.write(decToBcd(d) & 0x3F);      // Day
  Wire.write(0x00);                     // Weekday (0-6, not used here)
  Wire.write(decToBcd(mo) & 0x1F);     // Month
  Wire.write(decToBcd(y - 2000));      // Year (0-99)
  Wire.endTransmission();
}

void readTimeFromRTC() {
  Wire.beginTransmission(PCF8563_ADDR);
  Wire.write(0x02); // Start reading from seconds register
  Wire.endTransmission();
  
  Wire.requestFrom(PCF8563_ADDR, 7); // Request 7 bytes
  if (Wire.available() >= 7) {
    seconds = bcdToDec(Wire.read() & 0x7F); // Remove VL bit
    minutes = bcdToDec(Wire.read() & 0x7F);
    hours = bcdToDec(Wire.read() & 0x3F);   // Remove hour format bit
    day = bcdToDec(Wire.read() & 0x3F);
    Wire.read(); // Skip weekday
    month = bcdToDec(Wire.read() & 0x1F);   // Remove century bit
    year = 2000 + bcdToDec(Wire.read());
  }
}

// ========== SHT30 Temperature/Humidity Functions ==========
void initSHT30() {
  Wire.beginTransmission(SHT30_ADDR);
  Wire.write(0x22); // Soft reset command high byte
  Wire.write(0x36); // Soft reset command low byte
  Wire.endTransmission();
  delay(10);
}

void readTemperatureHumidity() {
  // Trigger measurement: periodic mode with 0.5 mps
  Wire.beginTransmission(SHT30_ADDR);
  Wire.write(0x21); // Command high byte
  Wire.write(0x30); // Command low byte
  Wire.endTransmission();
  
  delay(20); // Wait for measurement
  
  Wire.requestFrom(SHT30_ADDR, 6); // Request 6 bytes (temp(2) + checksum + humidity(2) + checksum)
  if (Wire.available() >= 6) {
    uint16_t tempRaw = (Wire.read() << 8) | Wire.read();
    Wire.read(); // Skip checksum
    uint16_t humidityRaw = (Wire.read() << 8) | Wire.read();
    Wire.read(); // Skip checksum
    
    // Convert temperature (in 0.01°C)
    temperature = -45.0 + (175.0 * tempRaw / 65535.0);
    // Convert humidity (in 0.1%)
    humidity = 100.0 * humidityRaw / 65535.0;
  }
}

// put function declarations here:





void setup() {
  // put your setup code here, to run once:
  Serial.setTx(PA9); // מגדיר את המשדר של הבקר לפין PA10 (היכן שחיברת את ה-RX של ה-USB)
  Serial.setRx(PA10);  // מגדיר את הקולט של הבקר לפין PA9 (היכן שחיברת את ה-TX של ה-USB)
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== DEVICE STARTING ===");
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Default relay OFF
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    for(;;); // עצירה אם המסך לא נמצא
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("SEND TIME:");
  display.println("HH:MM:SS DD:MM:YYYY");
  display.println("");
  display.println("Waiting 10 seconds...");
  display.display();
  
  Serial.println("\n=== PCF8563 TIME SYNC ===");
  Serial.println("Send time in format:");
  Serial.println("HH:MM:SS DD:MM:YYYY");
  Serial.println("Example: 15:06:00 14:06:2026");
  Serial.println("Waiting 10 seconds...\n");
  
  // Initialize sensors
  initRTC();
  delay(50);
  
  // Wait for time from PC (send format: "HH:MM:SS DD:MM:YYYY")
  unsigned long waitStart = millis();
  String timeStr = "";
  bool timeReceived = false;
  
  while (millis() - waitStart < 10000) { // Wait 10 seconds for time input
    if (Serial.available()) {
      char c = Serial.read();
      Serial.write(c); // Echo the character
      
      if (c == '\n' || c == '\r') {
        Serial.print("Received string length: ");
        Serial.println(timeStr.length());
        Serial.print("Content: [");
        Serial.print(timeStr);
        Serial.println("]");
        
        if (timeStr.length() >= 19) {
          timeReceived = true;
          break;
        }
      } else {
        timeStr += c;
      }
    }
  }
  
  Serial.println("Timeout reached.");
  
  // Parse time string if received
  if (timeReceived && timeStr.length() >= 19) {
    Serial.println("Parsing time...");
    int h = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    int m = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
    int s = (timeStr[6] - '0') * 10 + (timeStr[7] - '0');
    int d = (timeStr[9] - '0') * 10 + (timeStr[10] - '0');
    int mo = (timeStr[12] - '0') * 10 + (timeStr[13] - '0');
    int y = (timeStr[15] - '0') * 1000 + (timeStr[16] - '0') * 100 + 
            (timeStr[17] - '0') * 10 + (timeStr[18] - '0');
    
    Serial.print("Parsed: ");
    Serial.print(h); Serial.print(":");
    Serial.print(m); Serial.print(":");
    Serial.print(s); Serial.print(" ");
    Serial.print(d); Serial.print("/");
    Serial.print(mo); Serial.print("/");
    Serial.println(y);
    
    setTimeRTC(h, m, s, d, mo, y);
    Serial.print("RTC time set to: ");
    Serial.println(timeStr);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Time set!");
    display.print(timeStr.substring(0, 8));
    display.display();
    delay(1000);
  } else {
    Serial.println("No valid time received.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("No time received");
    display.println("Waiting for input...");
    display.display();
  }
  
  delay(50);
  initSHT30();
  delay(100);
}

void loop() {
  // LED blink: 5 seconds ON, 5 seconds OFF
  static unsigned long lastLedToggle = 0;
  static bool ledState = HIGH;
  
  if (millis() - lastLedToggle >= 1000) {
    lastLedToggle = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
  
  // Relay: ON for 20 seconds every round hour (minute=0, seconds=0-19)
  if (minutes == 0 && seconds < 20) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
  
  // Check for serial commands to update time
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd.length() >= 19) {
      int h = (cmd[0] - '0') * 10 + (cmd[1] - '0');
      int m = (cmd[3] - '0') * 10 + (cmd[4] - '0');
      int s = (cmd[6] - '0') * 10 + (cmd[7] - '0');
      int d = (cmd[9] - '0') * 10 + (cmd[10] - '0');
      int mo = (cmd[12] - '0') * 10 + (cmd[13] - '0');
      int y = (cmd[15] - '0') * 1000 + (cmd[16] - '0') * 100 + 
              (cmd[17] - '0') * 10 + (cmd[18] - '0');
      setTimeRTC(h, m, s, d, mo, y);
      Serial.println("Time updated!");
    }
  }
  
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    
    // Read time from RTC
    readTimeFromRTC();
    
    // Read temperature from SHT30 every 5 seconds
    static unsigned long lastTempRead = 0;
    if (millis() - lastTempRead >= 5000) {
      lastTempRead = millis();
      readTemperatureHumidity();
    }

    display.clearDisplay();
    
    // --- Display Time (top) ---
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 0);
    if(hours < 10) display.print("0"); display.print(hours);
    display.print(":");
    if(minutes < 10) display.print("0"); display.print(minutes);
    display.print(":");
    if(seconds < 10) display.print("0"); display.print(seconds);

    // --- Display Date (middle) ---
    display.setTextSize(1);
    display.setCursor(0, 25);
    if(day < 10) display.print("0"); display.print(day);
    display.print("/");
    if(month < 10) display.print("0"); display.print(month);
    display.print("/");
    display.print(year);
    
    // --- Display Temperature and Humidity (bottom) ---
    display.setCursor(0, 40);
    display.print("T:");
    display.print(temperature, 1);
    display.println("C");
    
    display.setCursor(0, 50);
    display.print("H:");
    display.print(humidity, 0);
    display.println("%");

    display.display();
  }
}