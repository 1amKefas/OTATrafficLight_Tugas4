const int pinMerah = 21;
const int pinKuning = 19;
const int pinHijau = 18;

unsigned long prevMillis = 0;
int state = 0;

void setup() {
  pinMode(pinMerah, OUTPUT);
  pinMode(pinKuning, OUTPUT);
  pinMode(pinHijau, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  // DURASI TUGAS 4 (1-1-1) - Tanpa WiFi!
  if (state == 0) {
    digitalWrite(pinMerah, HIGH); digitalWrite(pinKuning, LOW); digitalWrite(pinHijau, LOW);
    if (currentMillis - prevMillis >= 1000) { prevMillis = currentMillis; state = 1; }
  } 
  else if (state == 1) {
    digitalWrite(pinMerah, LOW); digitalWrite(pinKuning, HIGH); digitalWrite(pinHijau, LOW);
    if (currentMillis - prevMillis >= 1000) { prevMillis = currentMillis; state = 2; }
  } 
  else if (state == 2) {
    digitalWrite(pinMerah, LOW); digitalWrite(pinKuning, LOW); digitalWrite(pinHijau, HIGH);
    if (currentMillis - prevMillis >= 1000) { prevMillis = currentMillis; state = 0; }
  }
}