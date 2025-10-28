#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
 
const char* ssid = "iPhone de coelh";
const char* password = "11111111";
 
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
 
WiFiClient espClient;
PubSubClient client(espClient);
 
const int trigPin = 5;
const int echoPin = 18;
 
#define DHTPIN 4
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);
 
String id_jogadora = "jogadora_001";
String partida = "partida_2024_001";
unsigned long tempo_inicio;
unsigned long ultimo_tempo = 0;
float distancia_acumulada = 0.0;
float velocidade = 0.0;
 
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}
 
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    client.connect(clientId.c_str());
  }
}
 
float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracao = pulseIn(echoPin, HIGH, 30000);
  float distancia = duracao * 0.034 / 2.0;
  return distancia;
}
 
void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  tempo_inicio = millis();
  ultimo_tempo = millis();
}
 
void loop() {
  if (!client.connected()) reconnect();
  client.loop();
 
  unsigned long tempo_atual = millis();
  unsigned long delta_tempo = tempo_atual - ultimo_tempo;
 
  if (delta_tempo >= 2000) {
    float distancia_atual = medirDistancia(); 
    distancia_acumulada += distancia_atual / 100.0; 
 
    float temperatura = dht.readTemperature(); 
    if (isnan(temperatura)) temperatura = 0;
 
    velocidade = abs(temperatura) / 10.0; 
 
    StaticJsonDocument<200> doc;
    doc["id_jogadora"] = id_jogadora;
    doc["partida"] = partida;
    doc["tempo"] = (tempo_atual - tempo_inicio) / 1000;
    doc["distancia_acumulada"] = round(distancia_acumulada * 100) / 100.0;
    doc["velocidade"] = round(velocidade * 100) / 100.0;
 
    String jsonString;
    serializeJson(doc, jsonString);
 
    String topico = "partida/" + partida + "/jogadora/" + id_jogadora + "/estatisticas";
    client.publish(topico.c_str(), jsonString.c_str());
 
    Serial.println(jsonString);
 
    ultimo_tempo = tempo_atual;
  }
}