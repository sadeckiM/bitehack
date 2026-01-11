#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int licznik = 0;
const int CEL = 10; // Licznik, przy którym odpali się animacja

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); 
  }
  display.clearDisplay();
}

// Funkcja animacji wygranej
void animacjaWin() {
  for(int i = 0; i < 6; i++) {
    display.clearDisplay();
    
    // Miganie napisem i inwersja ekranu
    if(i % 2 == 0) {
      display.fillScreen(SSD1306_WHITE); // Białe tło
      display.setTextColor(SSD1306_BLACK); // Czarny tekst
    } else {
      display.fillScreen(SSD1306_BLACK); // Czarne tło
      display.setTextColor(SSD1306_WHITE); // Biały tekst
    }

    display.setTextSize(4);
    display.setCursor(20, 15);
    display.print("WIN!");
    display.display();
    delay(300);
  }
  delay(1000);
  
  licznik = 0; // Reset licznika po wygranej
}

void animacjaLose() {
  for(int i = 0; i < 6; i++) {
    display.clearDisplay();
    
    // Miganie napisem i inwersja ekranu
    if(i % 2 == 0) {
      display.fillScreen(SSD1306_WHITE); // Białe tło
      display.setTextColor(SSD1306_BLACK); // Czarny tekst
    } else {
      display.fillScreen(SSD1306_BLACK); // Czarne tło
      display.setTextColor(SSD1306_WHITE); // Biały tekst
    }

    display.setTextSize(3);
    display.setCursor(20, 20);
    display.print("LOSE!");
    display.display();
    delay(300);
  }
  delay(1000);
  
  licznik = 0; // Reset licznika po wygranej
}

void animacjaGAMEOVER() {
  for(int i = 0; i < 6; i++) {
    display.clearDisplay();
    
    // Miganie napisem i inwersja ekranu
    if(i % 2 == 0) {
      display.fillScreen(SSD1306_WHITE); // Białe tło
      display.setTextColor(SSD1306_BLACK); // Czarny tekst
    } else {
      display.fillScreen(SSD1306_BLACK); // Czarne tło
      display.setTextColor(SSD1306_WHITE); // Biały tekst
    }

    display.setTextSize(2);
    display.setCursor(15, 20);
    display.print("GAMEOVER!");
    display.display();
    delay(300);
  }
  delay(1000);
  
  licznik = 0; // Reset licznika po wygranej
}

void animacjaTryAgain() {
  for(int i = 0; i < 6; i++) {
    display.clearDisplay();
    
    // Miganie napisem i inwersja ekranu
    if(i % 2 == 0) {
      display.fillScreen(SSD1306_WHITE); // Białe tło
      display.setTextColor(SSD1306_BLACK); // Czarny tekst
    } else {
      display.fillScreen(SSD1306_BLACK); // Czarne tło
      display.setTextColor(SSD1306_WHITE); // Biały tekst
    }

    display.setTextSize(2);
    display.setCursor(8, 20);
    display.print("Try Again!");
    display.display();
    delay(300);
  }
  delay(1000);
  
  licznik = 0; // Reset licznika po wygranej
}

void loop() {
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Licznik:");
  
  display.setTextSize(3);
  display.setCursor(45, 30);
  display.print(licznik);
  
  display.display();

  // Sprawdzenie warunku wygranej
  if (licznik >= CEL) {
    delay(500);
    animacjaTryAgain();
  }

  licznik++;
  delay(500); // Przyspieszyłem trochę dla testu
}