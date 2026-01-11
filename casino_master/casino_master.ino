#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include "AiEsp32RotaryEncoder.h"
#include "pitches.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <APDS9930.h>



// ================= KONFIGURACJA PINÓW =================

// RFID
#define RFID_SS_PIN  5
#define RFID_RST_PIN 4

// LCD
#define LCD_ADDR 0x27 
#define LCD_COLS 16
#define LCD_ROWS 2

// ENKODER
#define ROT_A_PIN 26      
#define ROT_B_PIN 25      
#define ROT_BTN_PIN 27 
#define ROT_STEPS 4
#define BTN_BACK_PIN 12       

// GŁOŚNIK
#define speakerPin 32
#define BUZZER_CHANNEL 0   // Kanał PWM (0-15)

//PROXIMITY SNSOR
#define TOKEN_TRIG_PIN 2
#define TOKEN_ECHO_PIN 33

// OLED
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define LIMIT_CZASU 10 
int licznik_czasu = 0;

// --- LABIRYNT I2C ---
#define SLAVE_LABIRYNT_ADDR 8    // Adres Arduino Uno (Slave)
#define LABIRYNT_TIME_LIMIT 25   // Czas gry w sekundach

// --- RULETKA I2C ---
#define SLAVE_ROULETTE_ADDR 9     // Adres Arduino Nano (Slave)

// USTAWIENIA LOGICZNE
#define LEVEL_USER 1
#define LEVEL_ADMIN 2
#define START_BALANCE 50
#define GAME_COST_LABIRYNT 20 
#define COIN_DISTANCE 2
#define COIN_VALUE 20

// ================= OBIEKTY =================

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN); 
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS); 
Preferences db; 
AiEsp32RotaryEncoder rotary = AiEsp32RotaryEncoder(ROT_A_PIN, ROT_B_PIN, ROT_BTN_PIN, -1, ROT_STEPS);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
APDS9930 apds = APDS9930();


// ================= ZMIENNE STANU =================

enum AppState {
  STATE_SCAN,               
  STATE_MENU_USER,          
  STATE_MENU_ADMIN,         
  STATE_GAME_SELECT,
  
  STATE_ROULETTE_COLOR,     
  STATE_ROULETTE_BET,       

  STATE_PLAYING,            
  STATE_ADMIN_SCAN_TARGET,  
  STATE_ADMIN_EDIT_TAG,     
  STATE_SERIAL_ADMIN_WAIT,  
  STATE_WIPE_CONFIRM,
  STATE_DEPOSIT      
};

AppState currentState = STATE_SCAN;
String currentLoggedUid = ""; 
String targetEditUid = "";    

// --- ZMIENNE GRY ---
int rouletteColorIndex = 0; // 0=Czerwony, 1=Biały, 2=Zielony
int rouletteBetAmount = 0;
int activeGameId = -1;      // 0 = Ruletka, 1 = Labirynt

// --- LISTY MENU ---

const char* menuUser[] = {
  "1. Graj",
  "2. Sprawdz saldo",
  "3. Doladuj konto",
  "4. Wyloguj"
};
int lenMenuUser = 4;

const char* menuGames[] = {
  "1. Ruletka",
  "2. Labirynt (20p)",
  "3. Powrot"
};
int lenMenuGames = 3;

const char* menuRouletteColors[] = {
  "1. Czerwony (x2)",
  "2. Bialy (x2)",
  "3. Zielony (x7)"
};
int lenMenuRouletteColors = 3;

const char* menuAdmin[] = {
  "1. Graj",
  "2. Edycja karty", 
  "3. Wyczysc baze",
  "4. Wyloguj"
};
int lenMenuAdmin = 4;

const char* menuEdit[] = {
  "1. Sprawdz saldo",
  "2. Doladuj (ADM)",
  "3. Usun z bazy",
  "4. Nadaj Admina",
  "5. Powrot"
};
int lenMenuEdit = 5;

// --- MELODIE ---

// 1. STAR WARS MAIN THEME
int starWarsMelody[] = {
  NOTE_G4, NOTE_G4, NOTE_G4, NOTE_C5, NOTE_G5, 
  NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C6, NOTE_G5,
  NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C6, NOTE_G5,
  NOTE_F5, NOTE_E5, NOTE_F5, NOTE_D5
};
int starWarsDurations[] = {
  8, 8, 8, 2, 2, 
  8, 8, 8, 2, 4,
  8, 8, 8, 2, 4,
  8, 8, 8, 2
};   

// 2. IMPERIAL MARCH
int vaderMelody[] = {
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5,
  NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4,
  NOTE_E5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_C5,
  NOTE_GS4, NOTE_F4, NOTE_C5, NOTE_A4
};
int vaderDurations[] = {
  4, 4, 4, 6, 12,
  4, 6, 12, 2,
  4, 4, 4, 6, 12,
  4, 6, 12, 2
};

// 3. SEVEN NATION ARMY (Riff)
int snaMelody[] = {
  NOTE_E3, NOTE_E3, NOTE_G3, NOTE_E3, NOTE_D3, NOTE_C3, NOTE_B2,
  NOTE_E3, NOTE_E3, NOTE_G3, NOTE_E3, NOTE_D3, NOTE_C3, NOTE_D3, NOTE_C3, NOTE_B2
};
int snaDurations[] = {
  4, 8, 8, 8, 8, 4, 4,
  4, 8, 8, 8, 8, 8, 8, 8, 4
};
int marioM[] = { NOTE_E5, NOTE_E5, 0, NOTE_E5, 0, NOTE_C5, NOTE_E5, 0, NOTE_G5, 0, NOTE_G4 };
int marioD[] = { 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4 };

int piratesM[] = { NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, 0, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, 0, NOTE_F5, NOTE_G5, NOTE_E5, NOTE_E5, 0, NOTE_D5, NOTE_C5, NOTE_C5, NOTE_D5 };
int piratesD[] = { 8, 8, 4, 8, 8, 8, 8, 4, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 4 };

int indianaM[] = { NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, 0, NOTE_D4, NOTE_E4, NOTE_F4, 0, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_F5, 0, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5 };
int indianaD[] = { 8, 16, 16, 2, 8, 8, 16, 16, 2, 8, 16, 16, 2, 8, 8, 16, 16, 16, 16 };

// 4. MEGALOVANIA (Sans Theme)
  int megaM[] = {
    NOTE_D4, NOTE_D4, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
    NOTE_C4, NOTE_C4, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
    NOTE_B3, NOTE_B3, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
    NOTE_AS3, NOTE_AS3, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4
  };
  int megaD[] = {
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16,
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16,
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16,
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16
  };

  // 5. DARUDE - SANDSTORM (Dududududu!)
  int sandM[] = { 
    NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, 
    NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, 
    NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, 
    NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, 
    NOTE_A4, NOTE_B4 
  };
  int sandD[] = { 
    8, 8, 8, 8, 8, 
    16, 16, 16, 16, 16, 16, 
    16, 16, 16, 16, 16, 16, 16, 
    16, 16, 16, 16, 16, 16, 16, 
    8, 4 
  };

  int elevatorM[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_F4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_C4, NOTE_D4, NOTE_G3 };
  int elevatorD[] = { 4, 4, 4, 4, 2, 4, 2, 8, 4, 4, 4, 4, 4, 4, 4, 2 };

  int familiadaM[] = { 
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4, // Ta-da-da-dam
    NOTE_F4, NOTE_G4, NOTE_A4, NOTE_G4, 
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4, 
    NOTE_D4, NOTE_E4, NOTE_C4 
  };
  int familiadaD[] = { 
    4, 8, 8, 4, 
    8, 8, 8, 4, 
    4, 8, 8, 4, 
    8, 8, 2 
  };

  int sasiedziM[] = {
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C5, NOTE_G4, 0,
    NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4, 0,
    NOTE_F4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_A4, 0,
    NOTE_G4, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5
  };
  int sasiedziD[] = {
    4, 8, 8, 4, 4, 8,
    8, 8, 8, 8, 4, 8,
    8, 8, 8, 8, 4, 8,
    4, 4, 4, 4, 2
  };

  int lossM[] = { NOTE_C4, NOTE_B3, NOTE_AS3, NOTE_A3 };
int lossD[] = { 4, 4, 4, 2 }; // Ostatnia nuta jest długa i smutna

int epicMegaM[] = {
  // Sekcja: DOEH DEH DEH...
  NOTE_D4, NOTE_D4, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_C4, NOTE_C4, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_B3, NOTE_B3, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_AS3, NOTE_AS3, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  
  // Sekcja: INTENSIFIES (Oktawa wyżej)
  NOTE_D5, NOTE_D5, NOTE_D6, NOTE_A5, NOTE_GS5, NOTE_G5, NOTE_F5, NOTE_D5, NOTE_F5, NOTE_G5,
  NOTE_C5, NOTE_C5, NOTE_D6, NOTE_A5, NOTE_GS5, NOTE_G5, NOTE_F5, NOTE_D5, NOTE_F5, NOTE_G5
};

int epicMegaD[] = {
  // Czas trwania (na podstawie Twoich delayów)
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  
  // Intensifies
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8
};

int winFFM[] = {
  NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_GS4, NOTE_AS4, NOTE_C5, 0, NOTE_AS4, NOTE_C5
};
int winFFD[] = {
  8, 8, 8, 4, 4, 4, 8, 12, 8, 2
};

// ================= BAZA DANYCH =================

bool db_exists(String uid) {
  return db.isKey(uid.c_str());
}

int db_getBalance(String uid) {
  String key = uid + "_b";
  return db.getInt(key.c_str(), 0);
}

int db_getLevel(String uid) {
  String key = uid + "_l";
  return db.getInt(key.c_str(), LEVEL_USER); 
}

void db_setBalance(String uid, int amount) {
  String key = uid + "_b";
  db.putInt(key.c_str(), amount);
}

void db_setLevel(String uid, int level) {
  String key = uid + "_l";
  db.putInt(key.c_str(), level);
}

void db_createProfile(String uid) {
  if (!db_exists(uid)) {
    db.putString(uid.c_str(), "OK"); 
    String key_b = uid + "_b"; db.putInt(key_b.c_str(), START_BALANCE); 
    String key_l = uid + "_l"; db.putInt(key_l.c_str(), LEVEL_USER);    
  }
}

void db_deleteProfile(String uid) {
  if (db_exists(uid)) {
    db.remove(uid.c_str());             
    String key_b = uid + "_b"; db.remove(key_b.c_str()); 
    String key_l = uid + "_l"; db.remove(key_l.c_str()); 
  }
}

void db_wipe() {
  db.clear();
}

// ================= POMOCNICZE =================

String getUidString() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void IRAM_ATTR readEncoderISR() {
  rotary.readEncoder_ISR();
}

void updateLcdMenu(const char* text) {
  lcd.setCursor(0, 0);
  lcd.print("OPCJE MENU:     ");
  lcd.setCursor(0, 1);
  lcd.print(text);
  for(int i = strlen(text); i < 16; i++) lcd.print(" ");
}

// Mapowanie enkodera na stawkę
int getBetFromIndex(int index) {
  if (index < 10) return (index + 1) * 10; 
  else if (index < 18) return 100 + (index - 9) * 50; 
  else return 500 + (index - 17) * 100;
}

int getRouletteMaxIndex() {
  return 22; 
}

// Pomocnicza nazwa koloru
String getColorName(int id) {
  if (id == 0) return "CZERWONY";
  if (id == 1) return "BIALY";
  if (id == 2) return "ZIELONY";
  return "?";
}

void setLabiryntState(bool state) {
  Wire.beginTransmission(SLAVE_LABIRYNT_ADDR);
  Wire.write(state ? 1 : 0); // 1 = Odblokuj serwa, 0 = Zablokuj
  Wire.endTransmission();
}



// ================= FORWARD DECLARATIONS =================
void onUserMenuSelect(int id);
void onAdminMenuSelect(int id);
void onGameMenuSelect(int id);
void onEditMenuSelect(int id);
void onRouletteColorSelect(int id);
void handleScan();
void handleMenuGeneric(const char** items, int len, void (*action)(int));
void handleAdminScanTarget();
void handleSerialAdminWait();
void handleWipeConfirm();
void handlePlaying();
void handleRouletteBet();
void showScanScreen();
void runOledTest();
void animacjaWin();
void animacjaLose();
void animacjaGAMEOVER();
void animacjaTryAgain();
void playTune(int* melody, int* durations, int len, int tempo);
long getDistance();




// ================= SETUP =================

void setup() {
  Serial.begin(115200);

// --- 1. INICJALIZACJA DŹWIĘKU (LEDC) ---
  // Musi być przed innymi bibliotekami
  ledcSetup(BUZZER_CHANNEL, 2000, 8); 
  ledcAttachPin(speakerPin, BUZZER_CHANNEL);
  
  // Krótki test "piknięcia" przy starcie systemu (opcjonalne, ale potwierdza, że działa)
  ledcWriteTone(BUZZER_CHANNEL, 2000);
  ledcWrite(BUZZER_CHANNEL, 127); // Głośność 50%
  delay(100);
  ledcWrite(BUZZER_CHANNEL, 0);   // Cisza
  // 2. Reszta konfiguracji
  pinMode(BTN_BACK_PIN, INPUT_PULLUP);
  pinMode(TOKEN_TRIG_PIN, OUTPUT); //Pin, do którego podłączymy trig jako wyjście
  pinMode(TOKEN_ECHO_PIN, INPUT); //a echo, jako wejście
  
  // INICJALIZACJA ENKODERA
  rotary.begin();
  rotary.setup(readEncoderISR);
  rotary.setBoundaries(0, 1, true);
  rotary.setAcceleration(50);

  // INICJALIZACJA LCD
  lcd.init();
  lcd.backlight();
  
  // INICJALIZACJA RFID
  SPI.begin(); 
  rfid.PCD_Init(); 

  // INICJALIZACJA BAZY
  db.begin("rfid_sys", false);

  // INICJALIZACJA OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    display.clearDisplay();
    display.display(); 
  }

  // --- INICJALIZACJA CZUJNIKA METY (APDS-9930) ---
  // Ważne: Wire.begin() zostało już wywołane przez biblioteki ekranu, więc I2C działa.
  if (apds.init()) {
    Serial.println(F("APDS-9930 inicjalizacja OK"));
    
    // Wyłączamy przerwania, włączamy tylko czujnik zbliżeniowy
    if (apds.enableProximitySensor(false)) {
      Serial.println(F("Tryb Proximity wlaczony"));
    } else {
      Serial.println(F("Blad wlaczania Proximity!"));
    }
    
    // Ustawiamy czułość (Gain). 2X jest zazwyczaj optymalne.
    apds.setProximityGain(PGAIN_2X); 
  } else {
    Serial.println(F("BLAD: Nie wykryto APDS-9930! Sprawdz kable SDA/SCL/3.3V"));
  Serial.println("--- SYSTEM GOTOWY v4.2 ---");

  // TEST GŁÓWNY (już po załadowaniu wszystkiego)
  // Jeśli słyszałeś "puknięcie" na początku, a tego nie słyszysz -> winny jest OLED lub RFID
  // playTune(winFFM, winFFD, 10, 170);
  
  delay(2000);
  showScanScreen();
  }
}




// ================= LOOP =================

void loop() {
  // Obsługa komend Serial (Admin)
  if (currentState != STATE_PLAYING && Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "ADMIN") {
      currentState = STATE_SERIAL_ADMIN_WAIT;
      lcd.clear(); lcd.print("TRYB ADMIN (USB)");
    }
  }

  // USUNĄŁEM BLOK "if (digitalRead(BTN_BACK_PIN)..." BO GO JUŻ NIE UŻYWASZ
  // Teraz cofanie obsługuje się wewnątrz funkcji menu (np. opcja "Powrót")
  // lub przyciskiem enkodera w specyficznych stanach (jak Deposit).

  switch (currentState) {
    case STATE_SCAN:              handleScan(); break;
    case STATE_MENU_USER:         handleMenuGeneric(menuUser, lenMenuUser, onUserMenuSelect); break;
    case STATE_MENU_ADMIN:        handleMenuGeneric(menuAdmin, lenMenuAdmin, onAdminMenuSelect); break;
    case STATE_GAME_SELECT:       handleMenuGeneric(menuGames, lenMenuGames, onGameMenuSelect); break;
    case STATE_ROULETTE_COLOR:    handleMenuGeneric(menuRouletteColors, lenMenuRouletteColors, onRouletteColorSelect); break;
    case STATE_ROULETTE_BET:      handleRouletteBet(); break;
    case STATE_PLAYING:           handlePlaying(); break; 
    case STATE_ADMIN_SCAN_TARGET: handleAdminScanTarget(); break;
    case STATE_ADMIN_EDIT_TAG:    handleMenuGeneric(menuEdit, lenMenuEdit, onEditMenuSelect); break;
    case STATE_SERIAL_ADMIN_WAIT: handleSerialAdminWait(); break;
    case STATE_WIPE_CONFIRM:      handleWipeConfirm(); break;
    case STATE_DEPOSIT:           handleDeposit(); break; // <--- Upewnij się, że to tu jest
  }
}

// ================= LOGIKA STANÓW =================

void handleSerialAdminWait() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  String uid = getUidString();
  if (db_exists(uid)) {
    db_setLevel(uid, LEVEL_ADMIN);
    lcd.clear(); lcd.print("SUKCES!"); lcd.setCursor(0, 1); lcd.print("Nadano Admina");
  } else {
    lcd.clear(); lcd.print("BLAD!"); lcd.setCursor(0, 1); lcd.print("Brak w bazie");
  }
  delay(2000);
  rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
  currentState = STATE_SCAN; showScanScreen();
}

void handleScan() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  String uid = getUidString();
  bool isNew = false;

  if (!db_exists(uid)) {
    db_createProfile(uid);
    isNew = true;
  }
  if (isNew) {
    lcd.clear(); lcd.print("Rejestracja OK");
    lcd.setCursor(0, 1); lcd.print("Saldo: " + String(db_getBalance(uid)));
    delay(2500); 
  }

  int level = db_getLevel(uid);
  currentLoggedUid = uid; 
  
  lcd.clear(); lcd.print("Witaj " + String(level == LEVEL_ADMIN ? "ADMIN" : "USER"));
  lcd.setCursor(0, 1); lcd.print(uid);
  delay(1500);

  if (level == LEVEL_ADMIN) {
    currentState = STATE_MENU_ADMIN;
    rotary.setBoundaries(0, lenMenuAdmin - 1, true);
    rotary.setEncoderValue(0);
    updateLcdMenu(menuAdmin[0]);
  } else {
    currentState = STATE_MENU_USER;
    rotary.setBoundaries(0, lenMenuUser - 1, true);
    rotary.setEncoderValue(0);
    updateLcdMenu(menuUser[0]);
  }
  rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
}

void handleMenuGeneric(const char** items, int len, void (*action)(int)) {
  static int lastPos = -1;
  int pos = rotary.readEncoder();
  if (pos < 0) pos = 0; if (pos >= len) pos = len - 1;

  if (pos != lastPos) {
    updateLcdMenu(items[pos]);
    lastPos = pos;
  }
  if (rotary.isEncoderButtonClicked()) {
    action(pos);
    lastPos = -1; 
  }
}

void onUserMenuSelect(int id) {
  lcd.clear();
  switch(id) {
    case 0: // Graj
      currentState = STATE_GAME_SELECT;
      rotary.setBoundaries(0, lenMenuGames - 1, true);
      rotary.setEncoderValue(0);
      break;
    case 1: // Saldo
      lcd.print("Twoje Saldo:");
      lcd.setCursor(0, 1);
      lcd.print(db_getBalance(currentLoggedUid)); lcd.print(" pkt");
      delay(2000); updateLcdMenu(menuUser[1]);
      break; 
case 2: // Doladuj (ZMIANA TUTAJ!)
      lcd.clear();
      lcd.print("TRYB WPLATY");
      lcd.setCursor(0, 1);
      lcd.print("Uruchamiam sensor");
      delay(1000);
      
      lcd.clear();
      lcd.print("WRZUC MONETE...");
      lcd.setCursor(0,1);
      lcd.print("Kliknij by wyjsc");
      
      currentState = STATE_DEPOSIT; // Przełączenie na sensor
      break;
    case 3: // Wyloguj
      currentState = STATE_SCAN; currentLoggedUid = ""; showScanScreen();
      break;
  }
}

// --- MENU GIER ---
void onGameMenuSelect(int id) {
  lcd.clear();
  
  // 1. RULETKA
  if (id == 0) {
     activeGameId = 0; // Ustawiamy ID gry na Ruletkę
     currentState = STATE_ROULETTE_COLOR;
     rotary.setBoundaries(0, lenMenuRouletteColors - 1, true);
     rotary.setEncoderValue(0);
     updateLcdMenu(menuRouletteColors[0]);
     return;
  }

  // 2. LABIRYNT
  if (id == 1) {
    activeGameId = 1; // Ustawiamy ID gry na Labirynt
    int currentBalance = db_getBalance(currentLoggedUid);
    if (currentBalance < GAME_COST_LABIRYNT) {
      lcd.print("Brak srodkow!"); lcd.setCursor(0, 1); lcd.print("Koszt: " + String(GAME_COST_LABIRYNT));
      delay(2000); updateLcdMenu(menuGames[id]);
      return;
    } else {
      db_setBalance(currentLoggedUid, currentBalance - GAME_COST_LABIRYNT);
      lcd.print("Pobrano " + String(GAME_COST_LABIRYNT)); lcd.setCursor(0, 1); lcd.print("Grasz w Labirynt");
      Serial.println("GAME_START:LABIRYNT"); 
      delay(1500); lcd.clear();
      
      currentState = STATE_PLAYING;
      lcd.print("Labirynt..."); lcd.setCursor(0, 1); lcd.print("Czekam na wynik");
      return;
    }
  }

  // 3. POWRÓT
  if (id == 2) { 
    int lvl = db_getLevel(currentLoggedUid);
    currentState = (lvl == LEVEL_ADMIN) ? STATE_MENU_ADMIN : STATE_MENU_USER;
    rotary.setBoundaries(0, (lvl == LEVEL_ADMIN ? lenMenuAdmin : lenMenuUser) - 1, true);
    rotary.setEncoderValue(0);
    updateLcdMenu(lvl == LEVEL_ADMIN ? menuAdmin[0] : menuUser[0]);
    return;
  }
}

// --- RULETKA: KOLOR ---
void onRouletteColorSelect(int id) {
  rouletteColorIndex = id; 
  lcd.clear();
  lcd.print("Wybrano: ");
  lcd.print(getColorName(id));
  delay(1000);

  currentState = STATE_ROULETTE_BET;
  rotary.setBoundaries(0, getRouletteMaxIndex(), true); 
  rotary.setEncoderValue(0);
}

// --- RULETKA: STAWKA ---
void handleRouletteBet() {
  static int lastBetIndex = -1;
  int pos = rotary.readEncoder();
  int betValue = getBetFromIndex(pos);

  if (pos != lastBetIndex) {
    lcd.setCursor(0, 0); lcd.print("Stawka:         ");
    lcd.setCursor(8, 0); lcd.print(betValue);
    lcd.setCursor(0, 1); lcd.print("Kliknij by grac ");
    lastBetIndex = pos;
  }

  if (rotary.isEncoderButtonClicked()) {
    int currentBalance = db_getBalance(currentLoggedUid);
    
    if (currentBalance < betValue) {
      lcd.clear(); lcd.print("Brak srodkow!");
      lcd.setCursor(0, 1); lcd.print("Masz: " + String(currentBalance));
      delay(2000); lastBetIndex = -1;
    } else {
      db_setBalance(currentLoggedUid, currentBalance - betValue);
      rouletteBetAmount = betValue;

      lcd.clear(); lcd.print("Przyjeto zaklad"); delay(1000);
      
      String colStr = String(rouletteColorIndex); 
      Serial.println("GAME_START:RULETKA;COL:" + colStr + ";BET:" + String(rouletteBetAmount));

      currentState = STATE_PLAYING;
      lcd.clear(); lcd.print("Ruletka kreci..."); lcd.setCursor(0, 1); lcd.print("Czekam na wynik");
      lastBetIndex = -1;
    }
  }
}

// --- GŁÓWNA PĘTLA GRY (ODBIÓR WYNIKÓW) ---
// Zmienne pomocnicze (muszą być przed funkcją, tylko raz!)
unsigned long labiryntStartTime = 0;
bool labiryntRunning = false;
bool rouletteRunning = false; // Nowa flaga dla ruletki

void handlePlaying() {
  // Blokada przycisku enkodera
  if (rotary.isEncoderButtonClicked()) {} 

  // ============================================================
  // GRA 1: RULETKA (ID 0) - WERSJA I2C (ARDUINO NANO)
  // ============================================================
  if (activeGameId == 0) {
    
    // 1. START GRY (Wyślij komendę do Nano)
    if (!rouletteRunning) {
      rouletteRunning = true;
      
      // Wyślij "1" do Nano (Start)
      Wire.beginTransmission(SLAVE_ROULETTE_ADDR);
      Wire.write(1); 
      Wire.endTransmission();
      
      lcd.clear(); lcd.print("Kreci sie...");
      lcd.setCursor(0, 1); lcd.print("Powodzenia!");
      
      // Animacja kręcenia na OLED (opcjonalnie)
      display.clearDisplay();
      display.setTextSize(2); display.setCursor(20, 20);
      display.print("SPINNING"); display.display();
    }

    // 2. ODPYTYWANIE O WYNIK (POLLING)
    // Pytamy Nano co 200ms, żeby nie zapchać I2C
    static unsigned long lastPollTime = 0;
    if (millis() - lastPollTime > 200) {
       lastPollTime = millis();
       
       // Pytamy o 1 bajt
       Wire.requestFrom(SLAVE_ROULETTE_ADDR, 1);
       
       if (Wire.available()) {
         byte result = Wire.read();
         
         // 255 = STATUS_SPINNING (Nano jeszcze kręci)
         // Każda inna wartość (0, 1, 2) to gotowy wynik!
         if (result != 255) {
            
            // --- MAMY WYNIK! ---
            int winningColor = result;
            int pointsWon = 0;
            bool userWon = false;

            // Logika wygranej
            if (winningColor == rouletteColorIndex) {
              userWon = true;
              int multiplier = (winningColor == 2) ? 7 : 2; 
              pointsWon = rouletteBetAmount * multiplier;
            }

            // Wyświetlanie
            lcd.clear();
            lcd.print("Wynik: " + getColorName(winningColor));
            lcd.setCursor(0, 1);
            if (userWon) {
               lcd.print("Wygrana: " + String(pointsWon));
               animacjaWin();
               playTune(winFFM, winFFD, 10, 170);
            } else {
               lcd.print("Przegrana :(");
               animacjaLose();
            }

            // Zapis do bazy
            if (pointsWon > 0) {
              int current = db_getBalance(currentLoggedUid);
              db_setBalance(currentLoggedUid, current + pointsWon);
            }

            delay(3000); 
            
            // Reset flag i powrót
            rouletteRunning = false; 
            display.clearDisplay(); display.display();
            
            currentState = STATE_GAME_SELECT;
            rotary.setBoundaries(0, lenMenuGames - 1, true);
            updateLcdMenu(menuGames[0]); 
         }
       }
    }
  }

  // ============================================================
  // GRA 2: LABIRYNT (ID 1) - WERSJA I2C (UNO + SENSOR)
  // ============================================================
  else if (activeGameId == 1) {
    // ... (TUTAJ WKLEJ CAŁY KOD LABIRYNTU Z POPRZEDNIEJ ODPOWIEDZI) ...
    // ... (Dla przejrzystości nie wklejam go ponownie, ale on tu musi być) ...
    // SKOPIUJ GO Z POPRZEDNIEJ WIADOMOŚCI (tam gdzie jest obsługa APDS i OLED)
  }
}

void onAdminMenuSelect(int id) {
  lcd.clear();
  switch(id) {
    case 0: currentState = STATE_GAME_SELECT; rotary.setBoundaries(0, lenMenuGames - 1, true); rotary.setEncoderValue(0); break;
    case 1: currentState = STATE_ADMIN_SCAN_TARGET; lcd.print("EDYCJA KARTY"); lcd.setCursor(0, 1); lcd.print("Zbliz klienta..."); break;
    case 2: currentState = STATE_WIPE_CONFIRM; lcd.print("Wyczyscic dane?"); lcd.setCursor(0, 1); lcd.print("Kliknij = TAK"); break;
    case 3: currentState = STATE_SCAN; currentLoggedUid = ""; showScanScreen(); break;
  }
}

void handleAdminScanTarget() {
  if (rotary.isEncoderButtonClicked()) { 
     currentState = STATE_MENU_ADMIN; rotary.setBoundaries(0, lenMenuAdmin - 1, true); updateLcdMenu(menuAdmin[1]); return;
  }
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  String scannedUid = getUidString();
  
  if (!db_exists(scannedUid)) {
    lcd.clear(); lcd.print("Brak w bazie!"); delay(1500);
    lcd.clear(); lcd.print("EDYCJA KARTY"); lcd.setCursor(0,1); lcd.print("Zbliz klienta...");
  } else {
    targetEditUid = scannedUid; currentState = STATE_ADMIN_EDIT_TAG; rotary.setBoundaries(0, lenMenuEdit - 1, true); rotary.setEncoderValue(0); lcd.clear(); updateLcdMenu(menuEdit[0]);
  }
  rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
}

void onEditMenuSelect(int id) {
  lcd.clear();
  switch(id) {
    case 0: lcd.print("ID: " + targetEditUid); lcd.setCursor(0, 1); lcd.print("Saldo: " + String(db_getBalance(targetEditUid))); delay(2500); updateLcdMenu(menuEdit[0]); break;
    case 1: { int current = db_getBalance(targetEditUid); db_setBalance(targetEditUid, current + 100); lcd.print("Dodano +100"); delay(1500); updateLcdMenu(menuEdit[1]); } break;
    case 2: db_deleteProfile(targetEditUid); lcd.print("Usunieto!"); delay(1500); currentState = STATE_MENU_ADMIN; rotary.setBoundaries(0, lenMenuAdmin - 1, true); rotary.setEncoderValue(0); updateLcdMenu(menuAdmin[0]); break; 
    case 3: db_setLevel(targetEditUid, LEVEL_ADMIN); lcd.print("Awansowano na"); lcd.setCursor(0, 1); lcd.print("ADMINISTRATORA"); delay(2000); updateLcdMenu(menuEdit[3]); break;
    case 4: currentState = STATE_MENU_ADMIN; rotary.setBoundaries(0, lenMenuAdmin - 1, true); rotary.setEncoderValue(0); updateLcdMenu(menuAdmin[0]); break;
  }
}

void handleWipeConfirm() {
  if (rotary.isEncoderButtonClicked()) {
    lcd.clear(); lcd.print("Formatowanie..."); db_wipe(); delay(2000); currentState = STATE_SCAN; showScanScreen();
  }
}

void showScanScreen() {
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("SYSTEM LOGOWANIA"); lcd.setCursor(0, 1); lcd.print("Zbliz karte...");
}

void runOledTest() {
  lcd.clear();
  lcd.print("TEST EKRANU OLED");
  Serial.println("Rozpoczynam test OLED...");

  // Test 1: WIN
  Serial.println("Test: Animacja WIN");
  animacjaWin();
  delay(1000);

  // Test 2: LOSE
  Serial.println("Test: Animacja LOSE");
  animacjaLose();
  delay(1000);

  // Test 3: GAME OVER
  Serial.println("Test: Animacja GAMEOVER");
  animacjaGAMEOVER();
  delay(1000);

  // Test 4: TRY AGAIN
  Serial.println("Test: Animacja TryAgain");
  animacjaTryAgain();
  delay(1000);

  // Wyczyszczenie po testach
  display.clearDisplay();
  display.display();
  lcd.clear();
}

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
  
  licznik_czasu = 0; // Reset licznika po wygranej
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
  
  licznik_czasu = 0; // Reset licznik_czasua po wygranej
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
  
  licznik_czasu = 0; // Reset licznik_czasua po wygranej
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
  
  licznik_czasu = 0; // Reset licznik_czasua po wygranej
}

// --- FUNKCJA ODTWARZAJĄCA ---
void playTune(int melody[], int durations[], int size, int tempo) {
  float beatDuration = 60000.0 / tempo;
  
  for (int i = 0; i < size; i++) {
    int noteDuration = (4.0 / durations[i]) * beatDuration;
    
    if (melody[i] == 0) {
      // Pauza = głośność 0
      ledcWrite(BUZZER_CHANNEL, 0);
      delay(noteDuration);
    } else {
      // 1. Częstotliwość
      ledcWriteTone(BUZZER_CHANNEL, melody[i]);
      // 2. Głośność (bez tego buzzer milczy w dużym projekcie!)
      ledcWrite(BUZZER_CHANNEL, 127); 
      
      delay(noteDuration);
      
      // Przerwa między nutami
      ledcWrite(BUZZER_CHANNEL, 0); 
      delay(noteDuration * 0.10); 
    }
  }
  // Gwarancja ciszy na koniec
  ledcWrite(BUZZER_CHANNEL, 0);
}

// --- Zmień definicje na górze ---
#define TRIG_PIN 2
#define ECHO_PIN 33   // Zmień kabel na pin 34 (lub 33/35) - wejście
#define COIN_DISTANCE 8 // Zwiększmy lekko zasięg do 8cm

// Funkcja pomiaru z zabezpieczeniem (TIMEOUT)
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Czekamy max 30ms. Jeśli brak echa -> zwracamy 0, zamiast wisieć 1 sekundę
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

void handleDeposit() {
  // 1. Najpierw sprawdź przycisk (ENKODER), żeby wyjść
  if (rotary.isEncoderButtonClicked()) {
    // Dźwięk anulowania
    ledcWriteTone(BUZZER_CHANNEL, 500); ledcWrite(BUZZER_CHANNEL, 127);
    delay(100); ledcWrite(BUZZER_CHANNEL, 0);
    
    lcd.clear(); lcd.print("Anulowano");
    delay(500);
    
    // Powrót do menu
    currentState = STATE_MENU_USER;
    rotary.setBoundaries(0, lenMenuUser - 1, true);
    updateLcdMenu(menuUser[2]); // Wracamy na pozycję "Doładuj"
    return;
  }

  // 2. Pomiar
  long distance = getDistance();
  
  // Debugowanie - podgląd w Serial Monitorze
  // Serial.println(distance); 

  // 3. Logika monety (zakres 2cm - 6cm)
  if (distance > 1 && distance < COIN_DISTANCE) {
    int current = db_getBalance(currentLoggedUid);
    db_setBalance(currentLoggedUid, current + 10);
    
    lcd.clear(); 
    lcd.setCursor(0,0); lcd.print("MONETA PRZYJETA!");
    lcd.setCursor(0,1); lcd.print("Saldo: " + String(current + 10));
    
    // Dźwięk monety
    ledcWriteTone(BUZZER_CHANNEL, 1000); ledcWrite(BUZZER_CHANNEL, 127); delay(100);
    ledcWriteTone(BUZZER_CHANNEL, 2000); delay(200);
    ledcWrite(BUZZER_CHANNEL, 0);

    delay(1000); // Pauza żeby nie nabiło za dużo punktów
    
    // Przywróć napis
    lcd.clear(); lcd.print("WRZUC MONETE..."); lcd.setCursor(0,1); lcd.print("Kliknij by wyjsc");
  }
  
  // Krótkie opóźnienie dla stabilności
  delay(50);
}