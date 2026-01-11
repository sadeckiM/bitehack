#include <Wire.h>
#include <APDS9930.h>
#define ARDUINO_SLAVE_ADDR 8 // Adres I2C Arduino

// --- OBIEKTY I ZMIENNE ---
APDS9930 apds = APDS9930();
float ambient_light = 0;
float interact = 5.0;

bool isBallDetected() {
  if (apds.readAmbientLightLux(ambient_light)) {
    if (ambient_light < interact && ambient_light != -1) {
      return true;
    }
  }
  return false;
}

void setup() {
  // Prędkość standardowa dla ESP32
  Serial.begin(115200);
  delay(1000);
  

  // Inicjalizacja I2C dla sensora (SDA=21, SCL=22)
  Wire.begin(21, 22);

  // Inicjalizacja sensora APDS-9930
  if (apds.init()) {
    Serial.println(F("APDS-9930: Zainicjowany"));
  } else {
    Serial.println(F("APDS-9930: BŁĄD!"));
  }

  // Włączamy funkcje sensora
  apds.enableLightSensor(false);

}
void loop() {
  if (isBallDetected()) {
    Serial.println("KULKA WYKRYTA! Wysyłam do Arduino...");
    
    // Wysyłanie informacji do Arduino (Slave)
    Wire.beginTransmission(ARDUINO_SLAVE_ADDR);
    Wire.write(1); // 1 = Kulka wykryta
    Wire.endTransmission();
    
    delay(1000); // Zapora, by nie wysyłać sygnału ciągle
  }

  delay(100);
}