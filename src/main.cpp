/*
 * Monitor de ar condicionado (ESP-01S + DHT22 -> MQTT)
 *
 * Le temperatura e umidade de um sensor DHT22 conectado ao GPIO2 e
 * publica os valores via MQTT a cada 30 segundos. Inclui watchdog de
 * publicacao e timeouts de conexao (WiFi/MQTT) que reiniciam o
 * dispositivo caso ele fique indisponivel.
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 2      // Pino (GPIO2) onde o DHT22 esta conectado
#define DHTTYPE DHT22 // Modelo do sensor

// ---- Configuracao de rede e MQTT ----
const char* ssid = "SEU_WIFI_SSID";
const char* password = "SUA_SENHA_WIFI";
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic_temp = "CPD/BR/arcondicionado/temperatura";
const char* mqtt_topic_hum = "CPD/BR/arcondicionado/umidade";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// ---- Timeouts (ms) ----
const unsigned long WIFI_TIMEOUT = 30000;     // Maximo para conectar ao WiFi
const unsigned long MQTT_RETRY_TIME = 60000;  // Maximo tentando conectar ao MQTT
const unsigned long PUBLISH_TIMEOUT = 120000; // Maximo sem publicar (watchdog)

// Reinicia o ESP de proposito, registrando o motivo no serial
void restart(const char* reason) {
  Serial.print("Reiniciando: ");
  Serial.println(reason);
  delay(100);
  ESP.restart();
}

// Conecta ao WiFi com timeout; se estourar o tempo, reinicia
void setup_wifi() {
  delay(10);
  Serial.print("Conectando ao WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > WIFI_TIMEOUT) {
      restart("timeout WiFi");
    }
  }
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());
}

// Reconecta ao broker MQTT; se nao conectar dentro do tempo, reinicia
void reconnect() {
  unsigned long start = millis();
  while (!client.connected()) {
    if (millis() - start > MQTT_RETRY_TIME) {
      restart("timeout MQTT");
    }
    String clientId = "ESP01S_DHT22_";
    clientId += String(ESP.getChipId()); // ID unico baseado no chip
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT conectado!");
      return;
    }
    Serial.print("Falha MQTT, rc=");
    Serial.print(client.state());
    Serial.println(" tentando de novo em 5s...");
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  static unsigned long lastPublish = millis();

  // Garante conexao MQTT ativa
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Le temperatura e umidade do sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    // Converte float para string com 2 casas decimais e publica
    char buf[8];
    dtostrf(t, 1, 2, buf);
    client.publish(mqtt_topic_temp, buf);
    dtostrf(h, 1, 2, buf);
    client.publish(mqtt_topic_hum, buf);
    lastPublish = millis();
    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" C | Umid: ");
    Serial.print(h);
    Serial.println(" %");
  } else {
    Serial.println("Erro lendo DHT22");
  }

  // Watchdog: se nada foi publicado nos ultimos 2 min, reinicia
  if (millis() - lastPublish > PUBLISH_TIMEOUT) {
    restart("sem publicacao por 2min");
  }

  delay(30000);
}
