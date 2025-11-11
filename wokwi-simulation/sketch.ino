//================================================================
//             PROJETO DE DETECÇÃO DE QUEDAS COM ESP32
//================================================================
//
// Funcionalidades:
// 1. Lê dados do acelerômetro MPU6050.
// 2. Detecta uma queda baseada em um algoritmo de duplo limiar.
// 3. Ativa um buzzer localmente ao detectar a queda.
// 4. Conecta-se a uma rede Wi-Fi.
// 5. Publica uma mensagem de alerta em um servidor MQTT.
//================================================================

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include "PubSubClient.h"

// --- Configurações de Hardware ---
#define BUZZER_PIN 23 // Pino GPIO onde o buzzer está conectado

// --- Configurações de Wi-Fi (para o Wokwi) ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- Configurações do Servidor MQTT ---
// Usaremos um broker MQTT público para o teste.
// Você pode usar qualquer cliente MQTT (ex: MQTT Explorer) para se inscrever no tópico e ver a mensagem.
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "mackenzie/fall_detector/alert"; // Tópico para publicar o alerta

// --- Parâmetros de Detecção de Queda ---
// Estes valores podem precisar de ajuste fino para um dispositivo real.
const float FREEFALL_THRESHOLD = 3.5; // Limiar baixo para detectar queda livre (em m/s^2)
const float IMPACT_THRESHOLD = 18.0;  // Limiar alto para detectar o impacto (em m/s^2)
const int TIME_WINDOW = 500;          // Janela de tempo em milissegundos entre a queda livre e o impacto

// --- Objetos e Variáveis Globais ---
Adafruit_MPU6050 mpu;
WiFiClient espClient;
PubSubClient client(espClient);

bool freefall_detected = false;
unsigned long freefall_timestamp = 0;
unsigned long last_alert_time = 0;
const long alert_cooldown = 10000; // Cooldown de 10 segundos para evitar alertas repetidos

//================================================================
// FUNÇÃO DE CONFIGURAÇÃO (SETUP)
//================================================================
void setup() {
  Serial.begin(115200);

  // Inicializa o sensor MPU6050
  if (!mpu.begin()) {
    Serial.println("Falha ao encontrar o MPU6050. Verifique as conexões.");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 encontrado!");

  // Configura as faixas de medição do sensor
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Configura o pino do buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Conecta ao Wi-Fi e ao servidor MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  Serial.println("Setup concluído. Monitorando quedas...");
}

//================================================================
// FUNÇÃO DE LOOP PRINCIPAL
//================================================================
void loop() {
  // Mantém a conexão MQTT ativa
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  // Função principal de detecção de queda
  detect_fall();
}

//================================================================
// FUNÇÕES AUXILIARES
//================================================================

// --- Conexão Wi-Fi ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status()!= WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// --- Reconexão MQTT ---
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT broker...");
    String clientId = "ESP32FallDetector-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado!");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// --- Lógica de Detecção de Queda ---
void detect_fall() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calcula a magnitude do vetor de aceleração
  float acceleration_magnitude = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

  // Passo 1: Detectar a condição de queda livre
  if (acceleration_magnitude < FREEFALL_THRESHOLD) {
    freefall_detected = true;
    freefall_timestamp = millis();
  }

  // Passo 2: Se a queda livre foi detectada, procurar por um impacto
  if (freefall_detected) {
    // Verificar se o impacto ocorreu dentro da janela de tempo
    if (millis() - freefall_timestamp < TIME_WINDOW) {
      if (acceleration_magnitude > IMPACT_THRESHOLD) {
        // Queda confirmada!
        
        // Verifica o cooldown para não enviar múltiplos alertas
        if (millis() - last_alert_time > alert_cooldown) {
          Serial.println("=========================");
          Serial.println("QUEDA DETECTADA!");
          Serial.println("=========================");
          
          trigger_alert();
          last_alert_time = millis();
        }
        freefall_detected = false; // Reseta o detector
      }
    } else {
      // Se a janela de tempo expirou, reseta o detector
      freefall_detected = false;
    }
  }
}

// --- Acionamento dos Alertas (Buzzer e MQTT) ---
void trigger_alert() {
  // 1. Publica a mensagem no tópico MQTT
  client.publish(mqtt_topic, "ALERTA: Queda detectada!");
  Serial.println("Alerta MQTT enviado!");

  // 2. Ativa o buzzer por 3 segundos
  Serial.println("Ativando o buzzer...");
  // A função tone() gera uma frequência (em Hz) no pino especificado.
  // O ESP32 não tem a função tone() nativa do Arduino, mas o Wokwi a simula.
  // Para hardware real de ESP32, usaríamos a função ledcWriteTone().
  tone(BUZZER_PIN, 1000, 3000); // Toca um som de 1000 Hz por 3 segundos
  digitalWrite(BUZZER_PIN, HIGH);
  delay(3000);
  digitalWrite(BUZZER_PIN, LOW);
}