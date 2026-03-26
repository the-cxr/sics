#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// --- CONFIGURAÇÕES DE CONEXÃO ---
const char* ssid = "3DIND Visitantes";
const char* password = "bemvindo";

String serverName = "https://fsqpnjqbbqgvcyuypgpk.supabase.co/rest/v1/historico_leituras";
String apiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZzcXBuanFiYnFndmN5dXlwZ3BrIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzI0NDY2NDEsImV4cCI6MjA4ODAyMjY0MX0.wvWOvOry1MMU888dOb4hyPro751-0YhG7VYgYReUbmE";

// --- PINOS ESP32 ---
#define PIN_TAMPA_O 33
#define PIN_TAMPA_R 32
#define PIN_NIVEL_O1 21
#define PIN_NIVEL_O2 22
#define PIN_NIVEL_R1 25
#define PIN_NIVEL_R2 26
#define PIN_CALOR 4

// Variáveis para salvar o último estado e comparar
int last_nivelR = 0, last_nivelO = 0;
bool last_tampaO = false; // NOVA
bool last_tampaR = false; // NOVA
unsigned long lastTime = 0;
unsigned long timerDelay = 60000; 

int calcularNivel(int pA, int pB) {
  bool sA = digitalRead(pA);
  bool sB = digitalRead(pB);
  if (sA && sB)   return 100;
  if (sA && !sB)  return 50;
  if (!sA && sB)  return 25;
  return 0;
}

void enviarDados() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); 
    HTTPClient http;
    http.begin(client, serverName);
    
    int nivelO = calcularNivel(PIN_NIVEL_O1, PIN_NIVEL_O2);
    int nivelR = calcularNivel(PIN_NIVEL_R1, PIN_NIVEL_R2);
    bool tO = digitalRead(PIN_TAMPA_O); 
    bool tR = digitalRead(PIN_TAMPA_R);
    bool fogo = (analogRead(PIN_CALOR) * 0.1 > 60.0);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", apiKey);
    http.addHeader("Authorization", "Bearer " + apiKey);

    String json = "{";
    json += "\"estacao_id\": 1,";
    json += "\"nivel_organico\": " + String(nivelO) + ",";
    json += "\"nivel_reciclavel\": " + String(nivelR) + ",";
    json += "\"tampa_organico\": " + String(tO ? "true" : "false") + ",";
    json += "\"tampa_reciclavel\": " + String(tR ? "true" : "false") + ",";
    json += "\"alerta_fogo\": " + String(fogo ? "true" : "false");
    json += "}";

    Serial.println(">>> SICS ENVIANDO: " + json);
    int responseCode = http.POST(json);
    Serial.println("Resposta do Servidor: " + String(responseCode));
    
    http.end();
    lastTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TAMPA_O, INPUT_PULLUP);
  pinMode(PIN_TAMPA_R, INPUT_PULLUP);
  pinMode(PIN_NIVEL_O1, INPUT_PULLUP);
  pinMode(PIN_NIVEL_O2, INPUT_PULLUP);
  pinMode(PIN_NIVEL_R1, INPUT_PULLUP);
  pinMode(PIN_NIVEL_R2, INPUT_PULLUP);
  pinMode(PIN_CALOR, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
  
  // Leitura inicial para não disparar envio falso no boot
  last_tampaO = digitalRead(PIN_TAMPA_O);
  last_tampaR = digitalRead(PIN_TAMPA_R);
}

void loop() {
  int atualO = calcularNivel(PIN_NIVEL_O1, PIN_NIVEL_O2);
  int atualR = calcularNivel(PIN_NIVEL_R1, PIN_NIVEL_R2);
  bool atualTampaO = digitalRead(PIN_TAMPA_O);
  bool atualTampaR = digitalRead(PIN_TAMPA_R);

  // CONDIÇÃO ATUALIZADA: Envia se mudar NÍVEL OU mudar ESTADO DA TAMPA OU tempo esgotar
  if (atualO != last_nivelO || 
      atualR != last_nivelR || 
      atualTampaO != last_tampaO || 
      atualTampaR != last_tampaR || 
      (millis() - lastTime > timerDelay)) {
    
    // Atualiza as referências
    last_nivelO = atualO;
    last_nivelR = atualR;
    last_tampaO = atualTampaO;
    last_tampaR = atualTampaR;
    
    enviarDados();
  }
}
