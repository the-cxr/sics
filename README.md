#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// --- Configurações de Conexão ---
const char* ssid = "3DIND Visitantes";
const char* password = "bemvindo";

// 1. Adicione "/rest/v1/" + o nome da sua tabela no final da URL
String serverName = "https://fsqpnjqbbqgvcyuypgpk.supabase.co/rest/v1/historico_leituras";

// 2. Use APENAS a chave "anon" "public" que começa com "eyJhbGci..."
String apiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZzcXBuanFiYnFndmN5dXlwZ3BrIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzI0NDY2NDEsImV4cCI6MjA4ODAyMjY0MX0.wvWOvOry1MMU888dOb4hyPro751-0YhG7VYgYReUbmE";

// --- Configurações do Dispositivo ---
String DEVICE_ID = "LIXEIRA_SUB_01";
unsigned long lastTime = 0;
unsigned long timerDelay = 3600000; 

#define PIN_TAMPA_O D1
#define PIN_TAMPA_R D2
#define PIN_NIVEL_O1 D7
#define PIN_NIVEL_O2 D8
#define PIN_NIVEL_R1 D5
#define PIN_NIVEL_R2 D6

int last_nivelR, last_nivelO;

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
    
    // --- Cabeçalhos de Segurança (O "Crachá" para o Supabase) ---
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", apiKey);
    http.addHeader("Authorization", "Bearer " + apiKey);

    // Ajustamos para enviar os números como String entre aspas, se o banco pedir texto
    String jsonPayload = "{";
    jsonPayload += "\"estacao_id\": 1,"; 
    jsonPayload += "\"nivel_organico\": \"" + String(last_nivelO) + "\",";
    jsonPayload += "\"nivel_reciclavel\": \"" + String(last_nivelR) + "\"";
    jsonPayload += "}";

    Serial.println("Enviando Dados: " + jsonPayload);
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      Serial.println("Resposta do Servidor: " + String(httpResponseCode));
    } else {
      Serial.println("Erro no envio: " + http.errorToString(httpResponseCode));
    }
    
    http.end();
    lastTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TAMPA_O, INPUT);
  pinMode(PIN_TAMPA_R, INPUT);
  pinMode(PIN_NIVEL_O1, INPUT);
  pinMode(PIN_NIVEL_O2, INPUT);
  pinMode(PIN_NIVEL_R1, INPUT);
  pinMode(PIN_NIVEL_R2, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado com sucesso!");
}

void loop() {
  int atual_nivelO = calcularNivel(PIN_NIVEL_O1, PIN_NIVEL_O2);
  int atual_nivelR = calcularNivel(PIN_NIVEL_R1, PIN_NIVEL_R2);

  if (atual_nivelO != last_nivelO || atual_nivelR != last_nivelR) {
    last_nivelO = atual_nivelO;
    last_nivelR = atual_nivelR;
    enviarDados();
  }

  if ((millis() - lastTime) > timerDelay) {
    enviarDados();
  }
}
