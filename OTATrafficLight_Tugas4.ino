#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h> // Library MQTT baru lu

const char* ssid = "PUT312";       
const char* password = "pnjpnjpnj";  

// URL File GitHub lu
const String firmware_url = "https://raw.githubusercontent.com/1amKefas/OTATrafficLight_Tugas4/main/OTATrafficLight_Tugas4.ino.bin";

// Setup MQTT Broker Publik (Gratis)
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "kefas/pnj/tugas4/ota"; // Topik khusus punya lu

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Flag untuk trigger OTA
bool needUpdate = false;

// ==========================================
// DEKLARASI PIN UNTUK 4 JALUR PEREMPATAN
// ==========================================
const int pinMerah1 = 19; const int pinKuning1 = 18; const int pinHijau1 = 5;
const int pinMerah2 = 23; const int pinKuning2 = 22; const int pinHijau2 = 21;
const int pinMerah3 = 13; const int pinKuning3 = 12; const int pinHijau3 = 15;
const int pinMerah4 = 25; const int pinKuning4 = 33; const int pinHijau4 = 26;

TaskHandle_t TaskLampu;

// ==========================================
// CORE 0: KHUSUS LAMPU (Sama Persis Kayak Sebelumnya)
// ==========================================
void codeLampu(void * parameter) {
  for(;;) {
    // FASE 1: UTARA HIJAU
    digitalWrite(pinMerah1, LOW); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, HIGH);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 2: TRANSISI (UTARA KUNING, TIMUR KUNING)
    digitalWrite(pinMerah1, LOW); digitalWrite(pinKuning1, HIGH); digitalWrite(pinHijau1, LOW); 
    digitalWrite(pinMerah2, LOW); digitalWrite(pinKuning2, HIGH); digitalWrite(pinHijau2, LOW); 
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 3: TIMUR HIJAU
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW); 
    digitalWrite(pinMerah2, LOW); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, HIGH); 
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 4: TRANSISI (TIMUR KUNING, SELATAN KUNING)
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, LOW); digitalWrite(pinKuning2, HIGH); digitalWrite(pinHijau2, LOW); 
    digitalWrite(pinMerah3, LOW); digitalWrite(pinKuning3, HIGH); digitalWrite(pinHijau3, LOW); 
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 5: SELATAN HIJAU
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW); 
    digitalWrite(pinMerah3, LOW); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, HIGH); 
    digitalWrite(pinMerah4, HIGH); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 6: TRANSISI (SELATAN KUNING, BARAT KUNING)
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, LOW); digitalWrite(pinKuning3, HIGH); digitalWrite(pinHijau3, LOW); 
    digitalWrite(pinMerah4, LOW); digitalWrite(pinKuning4, HIGH); digitalWrite(pinHijau4, LOW); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 7: BARAT HIJAU
    digitalWrite(pinMerah1, HIGH); digitalWrite(pinKuning1, LOW); digitalWrite(pinHijau1, LOW);
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW); 
    digitalWrite(pinMerah4, LOW); digitalWrite(pinKuning4, LOW); digitalWrite(pinHijau4, HIGH); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    // FASE 8: TRANSISI (BARAT KUNING, UTARA KUNING)
    digitalWrite(pinMerah1, LOW); digitalWrite(pinKuning1, HIGH); digitalWrite(pinHijau1, LOW); 
    digitalWrite(pinMerah2, HIGH); digitalWrite(pinKuning2, LOW); digitalWrite(pinHijau2, LOW);
    digitalWrite(pinMerah3, HIGH); digitalWrite(pinKuning3, LOW); digitalWrite(pinHijau3, LOW);
    digitalWrite(pinMerah4, LOW); digitalWrite(pinKuning4, HIGH); digitalWrite(pinHijau4, LOW); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

// ==========================================
// FUNGSI CALLBACK MQTT (JIKA ADA PESAN MASUK)
// ==========================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Pesan MQTT masuk: ");
  Serial.println(message);

  // Kalau di aplikasi lu ngetik/ngirim pesan "GAS", alat lu bakal update!
  if (message == "GAS") {
    needUpdate = true; 
  }
}

// ==========================================
// FUNGSI RECONNECT MQTT
// ==========================================
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "ESP32_Kefas_" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Berhasil Terhubung!");
      // Subscribe ke topik lu biar bisa dengerin perintah
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.print("Gagal, error: ");
      Serial.print(mqttClient.state());
      Serial.println(" Coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void updateFirmware() {
  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL
  
  Serial.println("\nMulai Download Firmware Baru dari GitHub...");
  t_httpUpdate_return ret = httpUpdate.update(client, firmware_url);
  
  if (ret == HTTP_UPDATE_OK) {
    Serial.println("Update Berhasil! Restarting...");
  } else {
    Serial.printf("Update Gagal Error (%d)\n", httpUpdate.getLastError());
  }
}

// ==========================================
// CORE 1: KHUSUS WIFI & MQTT LISTENER
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(pinMerah1, OUTPUT); pinMode(pinKuning1, OUTPUT); pinMode(pinHijau1, OUTPUT);
  pinMode(pinMerah2, OUTPUT); pinMode(pinKuning2, OUTPUT); pinMode(pinHijau2, OUTPUT);
  pinMode(pinMerah3, OUTPUT); pinMode(pinKuning3, OUTPUT); pinMode(pinHijau3, OUTPUT);
  pinMode(pinMerah4, OUTPUT); pinMode(pinKuning4, OUTPUT); pinMode(pinHijau4, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected to WiFi!");

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  xTaskCreatePinnedToCore(codeLampu, "TaskLampu", 10000, NULL, 1, &TaskLampu, 0);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop(); // Biar ESP32 terus "mendengarkan"

  // Kalau terima pesan "GAS" dari HP, flag needUpdate jadi true, kita eksekusi OTA!
  if (needUpdate) {
    needUpdate = false; // Reset flag
    updateFirmware();
  }
}