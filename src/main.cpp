#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R2, 14, 13, 15, U8X8_PIN_NONE);

const char* ssid = "AMGARDEN";
const char* password = "AleMar7981";
const char* mqtt_server = "192.168.1.161";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

String zgiorno = "";
String znotte = "";
String zingresso = "";
String mansarda = "";

String hma = "";
String hzi = "";
String hzn = "";
String hzg = "";



void showData() {

  u8g2.clearBuffer();
  u8g2.drawLine(0, 2, 128, 2);
  u8g2.setFont(u8g2_font_DigitalDisco_tu);
  u8g2.drawStr(25,14,"AM DOMUS");
  u8g2.drawLine(0, 15, 128, 15);

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(6,24,"Rilevamento Temperature");


  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(6,35,"Mans.");

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(35,35,(char*)mansarda.c_str());

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(66,35,"ZGio.");

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(95,35,zgiorno.c_str());

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(6,45,"ZNot.");

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(35,45,znotte.c_str());

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(66,45,"Corr.");

  u8g2.setFont(u8g2_font_profont10_mf);
  u8g2.drawStr(95,45, zingresso.c_str());

  u8g2.drawLine(0, 47, 128, 47);

  u8g2.setFont(u8g2_font_iconquadpix_m_all);
  String statuses = hma + " " + hzg + " " + hzn + " " + hzi;
  u8g2.drawStr(10,61,statuses.c_str());


  u8g2.sendBuffer();

}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  DynamicJsonDocument doc(1024);

  String input = (char*)payload;

  deserializeJson(doc, input);
  JsonObject obj = doc.as<JsonObject>();

  String temp = obj["temperature"];
  String status = obj["status"];

  String topicString = topic;

  if(topicString == "amdomus/clima/status/zona-giorno") {

    hzg = (status == "keep")?"E":"T";
    zgiorno = temp;

  }else if(topicString == "amdomus/clima/status/zona-notte") {

    hzn = (status == "keep")?"E":"T";
    znotte = temp;

  }else if(topicString == "amdomus/clima/status/zona-ingresso") {

    hzi = (status == "keep")?"E":"T";
    zingresso = temp;

  }else if(topicString == "amdomus/clima/status/mansarda") {
  
    hma = (status == "keep")?"E":"T";
    mansarda = temp;

  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(temp);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");      
      client.publish("tele/amdomus/hmi/LWT", "Online");
      
      client.subscribe("amdomus/clima/status/zona-giorno");
      client.subscribe("amdomus/clima/status/zona-notte");
      client.subscribe("amdomus/clima/status/zona-ingresso");
      client.subscribe("amdomus/clima/status/mansarda");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void) {

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  u8g2.begin();
}

void loop(void) {
  

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Online", value);
    client.publish("tele/amdomus/hmi/LWT", msg);
  }


  showData();
  delay(1000);

}