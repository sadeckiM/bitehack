


const int PWMA = 6;  // Pin PWM
const int AIN1 = 7;  // Kierunek
const int AIN2 = 8;  // Kierunek

const int PWMB = 10; // Pin PWM
const int BIN1 = 11; // Kierunek 
const int BIN2 = 12; // Kierunek

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  
  Serial.begin(115200); // Standard dla Nano
}

void loop() {
  // --- RUCH DO PRZODU ---
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 200); // Prędkość 0-255
  delay(2000);

  // --- STOP (Luz) ---
  analogWrite(PWMA, 0);
  delay(1000);

  // --- RUCH DO TYŁU ---
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, 150);
  delay(2000);

  // --- HAMOWANIE (Short Brake) ---
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  delay(1000);
}