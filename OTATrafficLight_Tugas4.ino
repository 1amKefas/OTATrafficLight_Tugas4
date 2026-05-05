#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <Preferences.h> 

const char* ssid = "PUT312";       
const char* password = "pnjpnjpnj";  

// URL File GitHub
const String firmware_url = "https://raw.githubusercontent.com/1amKefas/OTATrafficLight_Tugas4/main/OTATrafficLight_Tugas4.ino.bin";
const String version_url = "https://raw.githubusercontent.com/1amKefas/OTATrafficLight_Tugas4/main/version.txt";

// ==========================================
// DEKLARASI PIN UNTUK 4 JALUR PEREMPATAN
// ==========================================
// Jalur 1 (Utara)
const int pinMerah1 = 19; const int pinKuning1 = 18; const int pinHijau1 = 5;
// Jalur 2 (Timur)
const int pinMerah2 = 23; const int pinKuning2 = 22; const int pinHijau2 = 21;
// Jalur 3 (Selatan)
const int pinMerah3 = 13; const int pinKuning3 = 12; const int pinHijau3 = 15;
// Jalur 4 (Barat)
const int pinMerah4 = 25; const int pinKuning4 = 33; const int pinHijau4 = 26;

Preferences preferences;
String local_version = "";

TaskHandle_t TaskLampu;

// ==========================================
// CORE 0: KHUSUS LAMPU (NGGAK BAKAL FREEZE)
// ==========================================
void codeLampu(void * parameter) {
  for(;;) {
    // ==========================================
    // FASE 1: UTARA HIJAU JALAN
    // ==========================================
    digitalWrite(pinMerah1, LOW); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, HIGH);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 2: TRANSISI (UTARA SIAP STOP, TIMUR ANCANG-ANCANG)
    // ==========================================
    digitalWrite(pinMerah1, LOW); digitalWrite(pinKuning1, HIGH); digitalWrite(pinHijau1, LOW); 
    digitalWrite(pinMerah2, LOW); digitalWrite(pinKuning2, HIGH); digitalWrite(pinHijau2, LOW); 
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 3: TIMUR HIJAU JALAN
    // ==========================================
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW); 
    digitalWrite(pinMerah2, LOW); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, HIGH); 
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 4: TRANSISI (TIMUR SIAP STOP, SELATAN ANCANG-ANCANG)
    // ==========================================
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, LOW); digitalWrite(pinKuning2, HIGH); digitalWrite(pinHijau2, LOW); 
    digitalWrite(pinMerah3, LOW); digitalWrite(pinKuning3, HIGH); digitalWrite(pinHijau3, LOW); 
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 5: SELATAN HIJAU JALAN
    // ==========================================
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW); 
    digitalWrite(pinMerah3, LOW); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, HIGH); 
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 6: TRANSISI (SELATAN SIAP STOP, BARAT ANCANG-ANCANG)
    // ==========================================
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, LOW); digitalWrite(pinKuning3, HIGH); digitalWrite(pinHijau3, LOW); 
    digitalWrite(pinMerah4, LOW); digitalWrite(pinKuning4, HIGH); digitalWrite(pinHijau4, LOW); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 7: BARAT HIJAU JALAN
    // ==========================================
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW); 
    digitalWrite(pinMerah4, LOW); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, HIGH); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // ==========================================
    // FASE 8: TRANSISI (BARAT SIAP STOP, UTARA ANCANG-ANCANG)
    // ==========================================
    digitalWrite(pinMerah1, LOW); digitalWrite(pinKuning1, HIGH); digitalWrite(pinHijau1, LOW); 
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, LOW); digitalWrite(pinKuning4, HIGH); digitalWrite(pinHijau4, LOW); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

// ==========================================
// CORE 1: KHUSUS WIFI & OTA
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(pinMerah1, OUTPUT); pinMode(pinKuning1, OUTPUT); pinMode(pinHijau1, OUTPUT);
  pinMode(pinMerah2, OUTPUT); pinMode(pinKuning2, OUTPUT); pinMode(pinHijau2, OUTPUT);
  pinMode(pinMerah3, OUTPUT); pinMode(pinKuning3, OUTPUT); pinMode(pinHijau3, OUTPUT);
  pinMode(pinMerah4, OUTPUT); pinMode(pinKuning4, OUTPUT); pinMode(pinHijau4, OUTPUT);

  preferences.begin("ota", false);
  local_version = preferences.getString("version", "");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected to WiFi!");

  // Nyalain otak kedua (Core 0) untuk jalankan lampu di background
  xTaskCreatePinnedToCore(codeLampu, "TaskLampu", 10000, NULL, 1, &TaskLampu, 0);
}

void checkUpdate() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, version_url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String remote_version = http.getString();
    remote_version.trim(); 
    
    if (remote_version != "" && remote_version != local_version) {
      Serial.println("\nWah ada update baru nih! Memulai proses download...");
      t_httpUpdate_return ret = httpUpdate.update(client, firmware_url);
      
      if (ret == HTTP_UPDATE_OK) {
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
  // Cek update ke GitHub setiap 15 detik. Bebas freeze!
  checkUpdate();
  delay(15000); 
}