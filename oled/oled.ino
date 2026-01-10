#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Szerokość wyświetlacza w pikselach
#define SCREEN_HEIGHT 64 // Wysokość wyświetlacza w pikselach

// Deklaracja wyświetlacza OLED połączonego przez I2C (piny domyślne 21, 22)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  // Inicjalizacja wyświetlacza z adresem 0x3C (najczęstszy dla 0.96")
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Błąd: Nie znaleziono wyświetlacza SSD1306"));
    for(;;); // Zatrzymaj program
  }

  display.clearDisplay();      // Wyczyść bufor
  display.setTextSize(1);      // Rozmiar czcionki
  display.setTextColor(WHITE); // Kolor tekstu
  display.setCursor(0, 10);    // Ustaw kursor (x, y)
  display.println("ESP32 + OLED 0.96");
  display.display();           // Wyślij dane do ekranu
  delay(2000);
}

int licznik = 0;

void loop() {
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Licznik:");
  
  display.setTextSize(3);
  display.setCursor(30, 30);
  display.print(licznik);
  
  display.display();
  
  licznik++;
  delay(1000);
}