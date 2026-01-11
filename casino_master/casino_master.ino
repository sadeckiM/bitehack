#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include "AiEsp32RotaryEncoder.h"

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

// BUTTON BACK
#define BTN_BACK_PIN 14

// USTAWIENIA LOGICZNE
#define LEVEL_USER 1
#define LEVEL_ADMIN 2
#define START_BALANCE 50
#define GAME_COST_LABIRYNT 20 

// ================= OBIEKTY =================

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN); 
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS); 
Preferences db; 
AiEsp32RotaryEncoder rotary = AiEsp32RotaryEncoder(ROT_A_PIN, ROT_B_PIN, ROT_BTN_PIN, -1, ROT_STEPS);

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
  STATE_WIPE_CONFIRM        
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

// ================= SETUP =================

void setup() {
  Serial.begin(115200);
  
  pinMode(BTN_BACK_PIN, INPUT_PULLUP);
  
  rotary.begin();
  rotary.setup(readEncoderISR);
  rotary.setBoundaries(0, 1, true);
  rotary.setAcceleration(50);

  lcd.init();
  lcd.backlight();
  
  SPI.begin(); 
  rfid.PCD_Init(); 

  db.begin("rfid_sys", false);

  showScanScreen();
  Serial.println("--- SYSTEM GOTOWY v4.1 (Logic on ESP) ---");
}

// ================= LOOP =================

void loop() {
  if (currentState != STATE_PLAYING && Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "ADMIN") {
      currentState = STATE_SERIAL_ADMIN_WAIT;
      lcd.clear(); lcd.print("TRYB ADMIN (USB)"); lcd.setCursor(0, 1); lcd.print("Zbliz karte...");
    }
  }

  if (currentState != STATE_PLAYING && digitalRead(BTN_BACK_PIN) == LOW) {
    delay(200);
    if (currentState != STATE_SCAN) {
      currentState = STATE_SCAN;
      currentLoggedUid = ""; 
      showScanScreen();
    }
  }

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
    case 2: // Doładuj
      {
        int bal = db_getBalance(currentLoggedUid);
        db_setBalance(currentLoggedUid, bal + 10);
        lcd.print("Doladowano +10");
        delay(1500); updateLcdMenu(menuUser[2]);
      }
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
void handlePlaying() {
  if (rotary.isEncoderButtonClicked()) {} // Blokada

  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("SCORE:")) {
      int scoreValue = cmd.substring(6).toInt(); 
      int pointsWon = 0;
      bool userWon = false;

      // === LOGIKA DLA RULETKI ===
      if (activeGameId == 0) {
        int winningColor = scoreValue; // W ruletce SCORE to ID koloru
        
        // Sprawdź czy kolor pasuje
        if (winningColor == rouletteColorIndex) {
          userWon = true;
          int multiplier = 2; // Domyślny dla Czerwony/Biały
          if (winningColor == 2) multiplier = 7; // Zielony
          
          pointsWon = rouletteBetAmount * multiplier;
        } else {
          userWon = false;
          pointsWon = 0;
        }

        // Wyświetlanie dla Ruletki
        lcd.clear();
        lcd.print("Wynik: " + getColorName(winningColor));
        lcd.setCursor(0, 1);
        if (userWon) {
           lcd.print("Wygrana: " + String(pointsWon));
        } else {
           lcd.print("Przegrana :(");
        }

      // === LOGIKA DLA LABIRYNTU (i innych gier) ===
      } else {
        // W labiryncie SCORE to po prostu punkty
        pointsWon = scoreValue;
        userWon = (pointsWon > 0);
        
        lcd.clear();
        if (userWon) {
          lcd.print("WYGRANA!");
          lcd.setCursor(0, 1);
          lcd.print("+" + String(pointsWon) + " pkt");
        } else {
          lcd.print("KONIEC GRY");
          lcd.setCursor(0, 1);
          lcd.print("Powodzenia next!");
        }
      }

      // --- WSPÓLNA LOGIKA ZAPISU DO BAZY ---
      if (pointsWon > 0) {
        int current = db_getBalance(currentLoggedUid);
        db_setBalance(currentLoggedUid, current + pointsWon);
      }

      delay(3000); 

      // Reset i powrót
      currentState = STATE_GAME_SELECT;
      rotary.setBoundaries(0, lenMenuGames - 1, true);
      updateLcdMenu(menuGames[0]); 
    }
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