#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <Preferences.h> // Library bawaan untuk nyimpen data permanen

const char* ssid = "PUT312";       
const char* password = "pnjpnjpnj";  

// URL File GitHub
const String firmware_url = "https://raw.githubusercontent.com/1amKefas/OTATrafficLight_Tugas4/main/OTATrafficLight_Tugas4.ino.bin";
const String version_url = "https://raw.githubusercontent.com/1amKefas/OTATrafficLight_Tugas4/main/version.txt";

const int pinMerah = 21;
const int pinKuning = 19;
const int pinHijau = 18;

Preferences preferences;
String local_version = "";

// Deklarasi Task untuk Multitasking
TaskHandle_t TaskLampu;

// ==========================================
// CORE 0: KHUSUS LAMPU (NGGAK BAKAL FREEZE)
// ==========================================
void codeLampu(void * parameter) {
  for(;;) {
    // --- GANTI DURASI LAMPU DI SINI SAJA NANTI ---
    
    // Merah Nyala (Contoh 1000 ms)
    digitalWrite(pinMerah, HIGH); digitalWrite(pinKuning, LOW); digitalWrite(pinHijau, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    
    // Kuning Nyala (Contoh 1000 ms)
    digitalWrite(pinMerah, LOW); digitalWrite(pinKuning, HIGH); digitalWrite(pinHijau, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    
    // Hijau Nyala (Contoh 1000 ms)
    digitalWrite(pinMerah, LOW); digitalWrite(pinKuning, LOW); digitalWrite(pinHijau, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

// ==========================================
// CORE 1: KHUSUS WIFI & OTA
// ==========================================
void setup() {
  Serial.begin(115200);
  pinMode(pinMerah, OUTPUT);
  pinMode(pinKuning, OUTPUT);
  pinMode(pinHijau, OUTPUT);

  // Ambil versi terakhir yang tersimpan di memori ESP32
  preferences.begin("ota", false);
  local_version = preferences.getString("version", "");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected to WiFi!");

  // Nyalain otak kedua untuk jalankan lampu di background
  xTaskCreatePinnedToCore(codeLampu, "TaskLampu", 10000, NULL, 1, &TaskLampu, 0);
}

void checkUpdate() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  // Cek file version.txt dulu ke GitHub
  http.begin(client, version_url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String remote_version = http.getString();
    remote_version.trim(); // Bersihin spasi
    
    // Kalau versi di GitHub beda sama yang di memori ESP32, baru sikat update!
    if (remote_version != "" && remote_version != local_version) {
      Serial.println("\nWah ada update baru nih! Memulai proses download...");
      
      t_httpUpdate_return ret = httpUpdate.update(client, firmware_url);
      
      if (ret == HTTP_UPDATE_OK) {
        // Simpan versi baru ke memori, lalu restart
        preferences.putString("version", remote_version);
        Serial.println("Update Berhasil! Restarting...");
        ESP.restart();
      } else {
        Serial.printf("Update Gagal Error (%d)\n", httpUpdate.getLastError());
      }
    }
  }
  http.end();
}

void loop() {
  // ESP32 bakal ngecek diam-diam setiap 10 detik.
  // Karena lampu jalan di Core 0, pengecekan ini NGGAK AKAN mengganggu lampu sama sekali!
  checkUpdate();
  delay(10000); 
}