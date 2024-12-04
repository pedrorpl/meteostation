#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 4

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Configuração Wi-Fi
const char* ssid = "";
const char* password = "";

// Configuração ThingSpeak
const char* server = "http://api.thingspeak.com";
String apiKey = ""; // Substitua pela sua Write API Key

// Sensor BMP280
Adafruit_BMP280 bmp; // Criando o objeto BMP280

void setup() {
  Serial.begin(115200);
  
  // Conexão Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Inicializa o sensor BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("Erro ao inicializar o sensor BMP280!");
    while (1);
  }
}

void loop() {
  // Lê os dados do sensor
  float temperatura = bmp.readTemperature();
  float pressao = bmp.readPressure(); // Em hPa
  float umidade = dht.readHumidity();
  float f = dht.readTemperature(true); 
  float hif_F = dht.computeHeatIndex(f, umidade);
  float hif_C = (hif_F - 32) * 5.0 / 9.0;
  float tempRedund = dht.readTemperature();
  float altitude = bmp.readAltitude(1013.25); // 1013.25 hPa = Pressão ao nível do mar

  // Mostra os dados no Serial Monitor
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");
  Serial.print("Pressão: ");
  Serial.print(pressao);
  Serial.println(" Pa");
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");

  // Envia os dados para o ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Monta a URL da requisição
    String url = String(server) + "/update?api_key=" + apiKey +
                 "&field1=" + String(temperatura) +
                 "&field2=" + String(pressao) +
                 "&field3=" + String(hif_C)+
                 "&field4=" + String(umidade)+
                 "&field5=" + String(altitude)+
                 "&field6=" + String(tempRedund);
    
    http.begin(url);
    int httpCode = http.GET(); // Faz a requisição GET

    if (httpCode > 0) {
      Serial.println("Dados enviados com sucesso!");
    } else {
      Serial.println("Falha ao enviar dados: " + http.errorToString(httpCode));
    }

    http.end();
  }

  delay(20000); // ThingSpeak permite uma atualização a cada 15 segundos
}
