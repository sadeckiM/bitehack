// Definicja pinów dla Arduino Nano
const int PWMA = 3;  // Pin PWM
const int AIN1 = 4;  // Kierunek
const int AIN2 = 5;  // Kierunek

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  
  Serial.begin(9600); // Standard dla Nano
  Serial.println("Test silnika TB6612 + Arduino Nano");
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