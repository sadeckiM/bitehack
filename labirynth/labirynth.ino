#include <Servo.h> // Standardowa biblioteka dla Arduino (nie ESP32Servo)
#include <Wire.h>

#define SLAVE_ADDR 8

// --- KONFIGURACJA UŻYTKOWNIKA ---
// Tutaj zmień wartość, aby ograniczyć ruch serw
// Np. wpisz 90, jeśli chcesz ruch tylko w zakresie 0-90 stopni
const int MAX_ANGLE = 120; 

// --- KONFIGURACJA PINÓW ---
const int servoPinX = 13;
const int servoPinY = 14; // Upewnij się, że Arduino ma takie piny PWM
const int joyPinX = A0;   // W Arduino piny analogowe to np. A0, A1
const int joyPinY = A1;
const int buttonSW = 2;

// --- OBIEKTY I ZMIENNE ---
Servo servoX;
Servo servoY;

bool gameStarted = false; // Zmienna stanu - czy gra trwa?
volatile bool ballInHole = false;
int lastButtonState = HIGH; // Do wykrywania kliknięcia

int joyValX = 0;
int joyValY = 0;
int swVal = 0;
int servoAngleX = 0;
int servoAngleY = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  // Przypisanie serw (z kalibracją 500-2400 dla SG90)
  servoX.attach(servoPinX, 500, 2400);
  servoY.attach(servoPinY, 500, 2400);
  
  Serial.println("Start systemu sterowania");
  Serial.print("Maksymalny kat ustawiony na: ");
  Serial.println(MAX_ANGLE);
  pinMode(buttonSW, INPUT_PULLUP);
}

// Funkcja wywoływana, gdy Master (ESP) coś wyśle
void receiveEvent(int howMany) {
  while (Wire.available()) {
    byte c = Wire.read();
    if (c == 1) {
      ballInHole = true; // Flaga wykrycia kulki
    }
  }
}

void loop() {

  int currentButtonState = digitalRead(buttonSW);

  if (currentButtonState == LOW && lastButtonState == HIGH) {
    gameStarted = !gameStarted; // Przełącz stan (ON/OFF)
    ballInHole = false; // Resetuj stan przy nowym starcie
    Serial.println(gameStarted ? "GRA START!" : "GRA STOP");
    delay(200);
  }
  lastButtonState = currentButtonState;

  //Warunek zakończenia gry (od ESP32)
  if (gameStarted && ballInHole) {
    gameStarted = false;
    Serial.println("KONIEC GRY - KULKA WYKRYTA PRZEZ MASTERA!");
    // Tutaj serwa mogą wrócić do poziomu 0
  }

  // ODCZYT (0-4095)
  if (gameStarted){
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