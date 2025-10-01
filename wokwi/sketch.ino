#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String id_jogadora = "jogadora_001";
String partida = "partida_2024_001";
unsigned long tempo_inicio;
float distancia_acumulada = 0.0;
float velocidade = 0.0;
unsigned long ultimo_tempo = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando à ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      Serial.print("Tópico: ");
      Serial.println("partida/" + partida + "/jogadora/" + id_jogadora + "/estatisticas");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  tempo_inicio = millis();
  ultimo_tempo = millis();
  randomSeed(analogRead(0));
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long tempo_atual = millis();
  unsigned long delta_tempo = tempo_atual - ultimo_tempo;
  
  if (delta_tempo >= 2000) { // Envia dados a cada 2 segundos
    float distancia_incremento = random(5, 20) / 10.0; // 0.5 a 2.0 metros
    distancia_acumulada += distancia_incremento;
    
    velocidade = (distancia_incremento / (delta_tempo / 1000.0));
    
    StaticJsonDocument<200> doc;
    doc["id_jogadora"] = id_jogadora;
    doc["partida"] = partida;
    doc["tempo"] = (tempo_atual - tempo_inicio) / 1000; // segundos
    doc["distancia_acumulada"] = round(distancia_acumulada * 100) / 100.0;
    doc["velocidade"] = round(velocidade * 100) / 100.0;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    String topico = "partida/" + partida + "/jogadora/" + id_jogadora + "/estatisticas";
    client.publish(topico.c_str(), jsonString.c_str());
    
    Serial.print("Publicado: ");
    Serial.println(jsonString);
    
    ultimo_tempo = tempo_atual;
  }
  
  delay(100);
}