#include <Wire.h>

// --- KONFIGURACJA ---
#define SLAVE_ADDR 9

// Definicja pinów dla 16 diod LED
// D2-D13 (12 sztuk) + A0-A3 (4 sztuki) = 16 LED
const int ledPins[16] = {
  2, 3, 4, 5, 6, 7, 8, 9,   // Pierwsza ósemka (LED 1-8)
  10, 11, 12, 13, A0, A1, A2, A3 // Druga ósemka (LED 9-16)
};

// Mapa kolorów (zgodna z Twoim opisem)
// 0 = Czerwony (Red)
// 1 = Biały (White)
// 2 = Zielony (Green)
// LED 1 i 9 są Zielone, reszta naprzemiennie
const byte ledColors[16] = {
  2, // LED 1: Zielony (Index 0)
  0, // LED 2: Czerwony
  1, // LED 3: Biały
  0, // LED 4: Czerwony
  1, // LED 5: Biały
  0, // LED 6: Czerwony
  1, // LED 7: Biały
  0, // LED 8: Czerwony
  2, // LED 9: Zielony (Index 8)
  1, // LED 10: Biały (Naprzemiennie po zielonym)
  0, // LED 11: Czerwony
  1, // LED 12: Biały
  0, // LED 13: Czerwony
  1, // LED 14: Biały
  0, // LED 15: Czerwony
  1  // LED 16: Biały
};

// Statusy dla Mastera
#define STATUS_READY    250
#define STATUS_SPINNING 255

// Zmienne stanu
volatile byte currentStatus = STATUS_READY;
bool isSpinning = false;

// Zmienne animacji
int currentLedIndex = 0;
long spinDelay = 0;      // Aktualne opóźnienie (prędkość)
long spinTimeTotal = 0;  // Ile czasu już się kręci

void setup() {
  // Konfiguracja pinów
  for (int i = 0; i < 16; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Test startowy - mrugnięcie wszystkimi
  allLedsOn(); delay(200); allLedsOff();

  // Inicjalizacja losowości (szum z niepodłączonego pinu A6 lub A7 - Nano je ma)
  // Jeśli używasz Uno, użyj np. A4/A5 przed startem I2C lub zostaw A0 jeśli wolne
  randomSeed(analogRead(A6)); 

  // Inicjalizacja I2C
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() {
  if (isSpinning) {
    runAnimationStep();
  }
}

// --- LOGIKA ANIMACJI ---
void runAnimationStep() {
  // 1. Zgaś poprzednią diodę
  digitalWrite(ledPins[currentLedIndex], LOW);

  // 2. Przejdź do następnej
  currentLedIndex++;
  if (currentLedIndex >= 16) currentLedIndex = 0;

  // 3. Zapal nową diodę
  digitalWrite(ledPins[currentLedIndex], HIGH);

  // 4. Dźwięk cykania (Opcjonalnie, jeśli masz buzzer na Nano, ale tu go nie ma)
  
  // 5. Czekaj (prędkość obrotu)
  delay(spinDelay);

  // 6. Logika zwalniania (fizyka ruletki)
  // Im dłużej się kręci, tym wolniej (większy delay)
  spinDelay += (spinDelay / 20) + 2; // Wykładnicze zwalnianie

  // 7. Warunek stopu
  // Jeśli opóźnienie jest bardzo duże (np. > 400ms), zatrzymujemy się
  if (spinDelay > 400) {
    isSpinning = false;
    
    // Ustal wynik dla Mastera
    // Odczytujemy kolor z tablicy na podstawie indeksu, na którym stanęliśmy
    byte resultColor = ledColors[currentLedIndex];
    currentStatus = resultColor;
    
    // Efekt "mrugania" wygraną diodą
    blinkWinner(currentLedIndex);
  }
}

// --- KOMUNIKACJA I2C ---

// Odbiór komendy od ESP32 (Master wysyła 1, żeby zacząć)
void receiveEvent(int howMany) {
  if (Wire.available()) {
    int cmd = Wire.read();
    
    if (cmd == 1 && !isSpinning) {
      // START RULETKI
      isSpinning = true;
      currentStatus = STATUS_SPINNING;
      
      // Reset parametrów animacji
      spinDelay = 15; // Startujemy bardzo szybko (15ms)
      
      // Losujemy początkowy indeks, żeby wynik był bardziej losowy
      currentLedIndex = random(0, 16);
      
      allLedsOff();
    }
  }
}

// Odpowiedź dla ESP32 (Master pyta o status)
void requestEvent() {
  Wire.write(currentStatus);
}

// --- POMOCNICZE ---

void allLedsOff() {
  for (int i = 0; i < 16; i++) digitalWrite(ledPins[i], LOW);
}

void allLedsOn() {
  for (int i = 0; i < 16; i++) digitalWrite(ledPins[i], HIGH);
}

void blinkWinner(int index) {
  for(int i=0; i<3; i++) {
    digitalWrite(ledPins[index], LOW);
    delay(150);
    digitalWrite(ledPins[index], HIGH);
    delay(150);
  }
}