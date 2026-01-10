#include <ESP32Servo.h>

// --- KONFIGURACJA UŻYTKOWNIKA ---
// Tutaj zmień wartość, aby ograniczyć ruch serw
// Np. wpisz 90, jeśli chcesz ruch tylko w zakresie 0-90 stopni
const int MAX_ANGLE = 120; 

// --- KONFIGURACJA PINÓW ---
const int servoPinX = 13;
const int servoPinY = 14;
const int joyPinX = 34;
const int joyPinY = 35;

// --- OBIEKTY I ZMIENNE ---
Servo servoX;
Servo servoY;

int joyValX = 0;
int joyValY = 0;
int servoAngleX = 0;
int servoAngleY = 0;

void setup() {
  Serial.begin(115200);

  // Przypisanie serw (z kalibracją 500-2400 dla SG90)
  servoX.attach(servoPinX, 500, 2400);
  servoY.attach(servoPinY, 500, 2400);
  
  Serial.println("Start systemu sterowania");
  Serial.print("Maksymalny kat ustawiony na: ");
  Serial.println(MAX_ANGLE);
}

void loop() {
  // 1. ODCZYT (0-4095)
  joyValX = analogRead(joyPinX);
  joyValY = analogRead(joyPinY);

  // 2. PRZELICZANIE (MAPPING)
  // Zamiast wpisywać "180", używamy teraz zmiennej "MAX_ANGLE"
  // Dzięki temu pełne wychylenie joysticka da nam tylko tyle stopni, ile ustawiliśmy
  servoAngleX = map(joyValX, 0, 4095, 0, MAX_ANGLE);
  servoAngleY = map(joyValY, 0, 4095, 0, MAX_ANGLE);

  // 3. STEROWANIE
  servoX.write(servoAngleX);
  servoY.write(servoAngleY);

  // 4. DIAGNOSTYKA (opcjonalnie, żeby widzieć co się dzieje)
  // Wyświetlamy tylko raz na jakiś czas, żeby nie zaśmiecać monitora
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
    Serial.print("Joy: "); Serial.print(joyValX);
    Serial.print(" -> Kat: "); Serial.println(servoAngleX);
    lastPrint = millis();
  }

  delay(20);
}