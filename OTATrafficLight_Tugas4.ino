#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

const char* ssid = "PUT312";       
const char* password = "pnjpnjpnj";  

const String firmware_url = "https://raw.githubusercontent.com/1amKefas/OTATrafficLight_Tugas4/main/OTATrafficLight_Tugas4.ino.bin";

const int pinMerah = 21;
const int pinKuning = 19;
const int pinHijau = 18;

unsigned long prevMillis = 0;
unsigned long prevUpdateMillis = 0; // Timer khusus buat ngecek update
int state = 0;

void setup() {
  Serial.begin(115200);
  pinMode(pinMerah, OUTPUT);
  pinMode(pinKuning, OUTPUT);
  pinMode(pinHijau, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); Serial.print("."); 
  }
  Serial.println("\nConnected to WiFi!");
}

void updateFirmware() {
  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL
  
  Serial.println("Mengecek update di GitHub...");
  t_httpUpdate_return ret = httpUpdate.update(client, firmware_url);
  
  switch (ret) {
    case HTTP_UPDATE_FAILED: 
      Serial.printf("Update Gagal Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); 
      break;
    case HTTP_UPDATE_NO_UPDATES: 
      Serial.println("Versi masih sama. Tidak ada update baru."); 
      break;
    case HTTP_UPDATE_OK: 
      Serial.println("Update Berhasil! Restarting..."); 
      break;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // FITUR PENGECEKAN RUTIN: Cek GitHub setiap 20 detik (20000 ms)
  if (currentMillis - prevUpdateMillis >= 20000) {
    prevUpdateMillis = currentMillis; 
    updateFirmware();
  }

  // --- AREA DURASI LAMPU (GANTI ANGKA DI SINI AJA NANTI) ---
  if (state == 0) {
    digitalWrite(pinMerah, HIGH); digitalWrite(pinKuning, LOW); digitalWrite(pinHijau, LOW);
    if (currentMillis - prevMillis >= 10000) { prevMillis = currentMillis; state = 1; }
  } 
  else if (state == 1) {
    digitalWrite(pinMerah, LOW); digitalWrite(pinKuning, HIGH); digitalWrite(pinHijau, LOW);
    if (currentMillis - prevMillis >= 5000) { prevMillis = currentMillis; state = 2; }
  } 
  else if (state == 2) {
    digitalWrite(pinMerah, LOW); digitalWrite(pinKuning, LOW); digitalWrite(pinHijau, HIGH);
    if (currentMillis - prevMillis >= 10000) { prevMillis = currentMillis; state = 0; }
  }
}