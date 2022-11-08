#include <Arduino.h>
#include <SPI.h>
#include <DW1000.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>

// dwm macros
#define DES_ADD_IDX 5
#define SRC_ADD_IDX 7
#define POLL_TS_IDX 10
#define RESP_TS_IDX 14
#define LAST_TS_IDX 18
#define DIST_RE_IDX 10
#define ANC_LOC_IDX 14
#define HEADING_IDX 10
#define MSG_COM_LEN 10
#define FCN_COD_IDX 9

// filter frequency
#define Freq 1.0f // Hz
#define HTs 0.01f // heading sampling time
#define DTs 0.03f // distance sampling time

// dwm connection pins
const uint8_t PIN_RST = D1;
const uint8_t PIN_IRQ = D2;
const uint8_t PIN_SS = SS;

// navigation status frame
typedef struct
{
  float D1, D2, D3;
  float Heading;
} NavState;

// system handles
WiFiClient net;
MQTTClient client;
volatile NavState target;

// dwm received handler
void handleReceived()
{
  byte buffer[4];
  float variable;
  String message;
  DW1000.getData(message);

  if (message.charAt(FCN_COD_IDX) == 0xE0)
  {
    message.getBytes(buffer, 5, HEADING_IDX);
    ets_memcpy(&variable, buffer, 4);
    target.Heading += (variable - target.Heading) * TWO_PI * Freq * HTs;
  }

  if (message.charAt(FCN_COD_IDX) == 0xE1)
  {
    message.getBytes(buffer, 5, DIST_RE_IDX);
    ets_memcpy(&variable, buffer, 4);
    switch (message[SRC_ADD_IDX + 1])
    {
    case '1':
      target.D1 += (variable - target.D1) * TWO_PI * Freq * DTs;
      break;
    case '2':
      target.D2 += (variable - target.D2) * TWO_PI * Freq * DTs;
      break;
    case '3':
      target.D3 += (variable - target.D3) * TWO_PI * Freq * DTs;
      break;
    }
  }
}

// initialize dwm
void dwm_init()
{
  // initialize the driver
  DW1000.begin(PIN_IRQ, PIN_RST);
  DW1000.select(PIN_SS);
  Serial.println(F("DW1000 initialized ..."));
  // general configuration
  DW1000.newConfiguration();
  DW1000.setChannel(DW1000.CHANNEL_2);
  DW1000.setPulseFrequency(DW1000.TX_PULSE_FREQ_64MHZ);
  DW1000.setPreambleLength(DW1000.TX_PREAMBLE_LEN_128);
  DW1000.setPreambleCode(DW1000.PREAMBLE_CODE_64MHZ_9);
  DW1000.setDataRate(DW1000.TRX_RATE_6800KBPS);
  DW1000.interruptOnSent(true);
  DW1000.interruptOnReceived(true);
  DW1000.commitConfiguration();
  Serial.println(F("Committed configuration ..."));
  // wait a bit
  delay(1000);
  // start reception
  DW1000.attachReceivedHandler(handleReceived);
  DW1000.newReceive();
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}

void connect()
{
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);
  while (!client.connect(String(ESP.getChipId()).c_str()))
    delay(1000);
}

void setup()
{
  // debug monitoring
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // get online
  WiFi.begin("mkia", "kamran123");
  client.begin("broker.mqttdashboard.com", net);

  // wait a bit
  delay(3000);

  // init dwm
  dwm_init();
}

String serialize(volatile NavState *s)
{
  StaticJsonDocument<JSON_OBJECT_SIZE(4)> doc;
  String json;
  doc["D1"] = s->D1;
  doc["D2"] = s->D2;
  doc["D3"] = s->D3;
  doc["Heading"] = s->Heading;
  serializeJson(doc, json);
  return json;
}

void loop()
{
  // update mqtt connection
  client.loop();

  // delay for wi-fi and other connections stabilization
  delay(50);

  // connect in case of connection drop
  if (!client.connected())
    connect();

  // report to server
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 1000)
  {
    lastMillis = millis();
    String json = serialize(&target);
    bool res = client.publish("KNTUMasters/Mechat/Navigation", json);
    if (res == true)
      Serial.println(json);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
