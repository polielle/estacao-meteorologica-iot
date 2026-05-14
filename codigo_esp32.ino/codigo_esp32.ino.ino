#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"

// ======================================================
// WIFI
// ======================================================

const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ======================================================
// INFLUXDB
// ======================================================

const char* influxUrl =
"SEU_URL";

const char* token =
"SEU_TOKEN";

// ======================================================
// SENSORES
// ======================================================

// ----- DHT22 -----
#define DHTPIN 3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ----- LDR -----
#define LDRPIN 1

// ----- BMP280 -----
Adafruit_BMP280 bmp;

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  delay(3000);

  Serial.println("=================================");
  Serial.println("INICIANDO SISTEMA");
  Serial.println("=================================");

  // ==================================================
  // WIFI
  // ==================================================

  WiFi.begin(ssid, password);

  Serial.print("Conectando no WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ==================================================
  // DHT22
  // ==================================================

  dht.begin();

  Serial.println("DHT22 iniciado!");

  // ==================================================
  // BMP280
  // ==================================================

  // SDA = GPIO 4
  // SCL = GPIO 5

  Wire.begin(8, 9);

  if (!bmp.begin(0x76)) {

    if (!bmp.begin(0x77)) {

      Serial.println("BMP280 NAO ENCONTRADO!");

      while (1);
    }
  }

  Serial.println("BMP280 iniciado!");

  // ==================================================
  // LDR
  // ==================================================

  pinMode(LDRPIN, INPUT);

  Serial.println("LDR iniciado!");

  Serial.println("=================================");
  Serial.println("SISTEMA PRONTO");
  Serial.println("=================================");
}

// ======================================================
// LEITURA LDR
// ======================================================

int lerLuminosidade() {

  int soma = 0;

  for (int i = 0; i < 10; i++) {

    soma += analogRead(LDRPIN);

    delay(10);
  }

  return soma / 10;
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  // ================================================
  // VERIFICA WIFI
  // ================================================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi desconectado!");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi reconectado!");
  }

  // ================================================
  // DHT22
  // ================================================

  float umidade = dht.readHumidity();

  float temperaturaDHT = dht.readTemperature();

  // ================================================
  // BMP280
  // ================================================

  float temperaturaBMP = bmp.readTemperature();

  float pressao = bmp.readPressure() / 100.0;

  // ================================================
  // LDR
  // ================================================

  int luminosidade = lerLuminosidade();

  // ================================================
  // SERIAL MONITOR
  // ================================================

  Serial.println();
  Serial.println("============== DADOS ==============");

  // ----- DHT22 -----

  if (isnan(umidade) || isnan(temperaturaDHT)) {

    Serial.println("Erro ao ler DHT22");

  } else {

    Serial.print("Temperatura DHT22: ");
    Serial.print(temperaturaDHT);
    Serial.println(" °C");

    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");
  }

  // ----- BMP280 -----




  Serial.print("Pressao: ");
  Serial.print(pressao);
  Serial.println(" hPa");

  // ----- LDR -----

  Serial.print("Luminosidade: ");
  Serial.println(luminosidade);

  Serial.println("===================================");

  // ================================================
  // ENVIO PARA INFLUXDB
  // ================================================

  if (WiFi.status() == WL_CONNECTED) {

    String data = "clima,local=sala ";

    data += "temperatura_dht=" + String(temperaturaDHT);

    data += ",umidade=" + String(umidade);

    data += ",temperatura_bmp=" + String(temperaturaBMP);

    data += ",pressao=" + String(pressao);

    data += ",luminosidade=" + String(luminosidade);

    HTTPClient http;

    http.begin(influxUrl);

    http.addHeader("Authorization", "Token " + String(token));

    http.addHeader("Content-Type", "text/plain");

    int httpResponseCode = http.POST(data);

    Serial.println();
    Serial.println("=========== INFLUXDB ===========");

    Serial.print("Enviado: ");
    Serial.println(data);

    Serial.print("Resposta HTTP: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 204) {

      Serial.println("Dados enviados com sucesso!");

    } else {

      Serial.println("Erro ao enviar dados!");
    }

    Serial.println("================================");

    http.end();
  }

  // ================================================
  // ESPERA
  // ================================================

  delay(5000);
}