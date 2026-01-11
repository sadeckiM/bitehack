#define trigPin 18
#define echoPin 5

int interact = 5;
void setup() {
  Serial.begin (115200);
  pinMode(trigPin, OUTPUT); //Pin, do którego podłączymy trig jako wyjście
  pinMode(echoPin, INPUT); //a echo, jako wejście
}
void loop() {  
  long distance = getDistance();
  Serial.print("Odleglosc: ");
  Serial.println(distance);

  if (distance < interact){
    Serial.println("Wykryto monete");
  }
  delay(500);
} 
long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // Wynik w cm
}