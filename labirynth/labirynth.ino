#include <Servo.h>
#include <Wire.h>

#define SLAVE_ADDR 8

// --- KONFIGURACJA ---
const int MAX_ANGLE = 120; // Zakres ruchu serw
const int CENTER_ANGLE = MAX_ANGLE / 2; // Pozycja spoczynkowa

// --- PINY (UNO R4) ---
const int servoPinX = 10;
const int servoPinY = 11; 
const int joyPinX = A0;   
const int joyPinY = A1;

Servo servoX;
Servo servoY;

// Zmienna decydująca o tym, czy można grać
// volatile oznacza, że zmienna może być zmieniona w przerwaniu (receiveEvent)
volatile bool gameEnabled = false; 

void setup() {
  Serial.begin(115200);

  // Inicjalizacja I2C jako SLAVE
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  servoX.attach(servoPinX);
  servoY.attach(servoPinY);
  
  // Ustawienie na środek przy starcie
  centerServos();
  
  Serial.println("Slave gotowy. Czekam na Mastera...");
  analogReadResolution(10); // Dla R4 (0-1023)
}

void loop() {
  if (gameEnabled) {
    // --- TRYB GRY ---
    int valX = analogRead(joyPinX);
    int valY = analogRead(joyPinY);

    // Mapowanie joysticka na kąt serw
    int angleX = map(valX, 0, 1023, 0, MAX_ANGLE);
    int angleY = map(valY, 0, 1023, 0, MAX_ANGLE);

    servoX.write(angleX);
    servoY.write(angleY);
    
    delay(15); // Małe opóźnienie dla płynności
  } 
  else {
    // --- TRYB BLOKADY (Koniec czasu lub wygrana) ---
    // W pętli ciągle wymuszamy środek, żeby gracz nie mógł ruszać
    // Można to robić rzadziej, żeby nie "buczały" serwa
    static unsigned long lastLockTime = 0;
    if (millis() - lastLockTime > 500) {
       centerServos();
       lastLockTime = millis();
    }
  }
}

// Funkcja ustawiająca labirynt w poziomie
void centerServos() {
  servoX.write(CENTER_ANGLE);
  servoY.write(CENTER_ANGLE);
}

// --- ODBIÓR DANYCH Z ESP32 ---
// Master wyśle: 1 = START, 0 = STOP
void receiveEvent(int howMany) {
  if (Wire.available()) {
    int command = Wire.read();
    
    if (command == 1) {
      gameEnabled = true;
      Serial.println("CMD: START");
    } 
    else if (command == 0) {
      gameEnabled = false;
      centerServos(); // Natychmiastowy reset po otrzymaniu STOP
      Serial.println("CMD: STOP");
    }
  }
}