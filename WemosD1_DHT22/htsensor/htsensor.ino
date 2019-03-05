/*
온습도 정보를 ThingSpeak 로 전송
온습도 센서 DHT22

*/
#include <ESP8266WiFi.h>
#include "DHT.h"

// 1. Wifi & ThingSpeak 정보
#ifndef STASSID
#define STASSID "scolas"
#define STAPSK  "01048568946"
#define THINGSPEAK_APIKEY "KRUZUWIWD6HZB4VW"
#define THINGSPEAK_HOST "api.thingspeak.com"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
String apiKey = THINGSPEAK_APIKEY;
const char* host = THINGSPEAK_HOST;
const uint16_t port = 80;

// 2. DHT 정보
#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);

  // WIFI Initial
  Serial.print("Connect WIFI ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print(" Connected ");
  Serial.println(WiFi.localIP());

  // DHT Initial
  dht.begin();
}

void loop() {
  // 온습도 측정
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Heat Index를 섭씨로 계산 (3번째 인수는 화씨 여부)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C  Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));

  // ThingSpeak 연결
  Serial.print("Connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.print(port);

  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println(" Failed");
    delay(5000);
    return;
  }

  // 데이터 전송
  if (client.connected()) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(h);
    postStr += "&field2=";
    postStr += String(t);
    postStr += "&field3=";
    postStr += String(hic);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print(" Success");
  }

  Serial.println("");
  client.stop();
  delay(60000);
}
