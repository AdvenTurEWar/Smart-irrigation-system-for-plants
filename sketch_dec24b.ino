#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Update these with your network credentials
const char* ssid = "RAKSACHAT_NB 4860";
const char* password = "123456789";
const char* mqtt_server = "10.1.1.145";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (150)
char msg[MSG_BUFFER_SIZE];
int value = 0;

float temperature;
float humidity;
int soilMoistureValue = 0;
DHT dht14(D4, DHT11);
const int LED_pin_R = D1;
const int LED_pin_G = D6;
const int LED_pin_B = D2;
const int relayPin = D5; // Relay connected to D5
const int soilMoisturePin = D3; // Soil Moisture Sensor connected to D3

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(LED_pin_R, HIGH);
  digitalWrite(LED_pin_G, HIGH);
  digitalWrite(LED_pin_B, HIGH);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Simple switch to control LED based on MQTT message
  if ((char)payload[0] == '1') {
    digitalWrite(LED_pin_R, LOW);
    digitalWrite(LED_pin_B, LOW);
  } else {
    digitalWrite(LED_pin_R, HIGH);
    digitalWrite(LED_pin_B, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
      client.subscribe("ledstatus");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_pin_R, OUTPUT);
  pinMode(LED_pin_G, OUTPUT);
  pinMode(LED_pin_B, OUTPUT);
  pinMode(relayPin, OUTPUT); // Set relay pin as output
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht14.begin();

  digitalWrite(LED_pin_R, HIGH);
  digitalWrite(LED_pin_G, HIGH);
  digitalWrite(LED_pin_B, HIGH);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Read humidity and temperature from DHT11
    humidity = dht14.readHumidity();
    temperature = dht14.readTemperature();

    // Read the soil moisture value from the sensor
    soilMoistureValue = analogRead(soilMoisturePin);

    // Control relay based on soil moisture
    if (soilMoistureValue < 502) { // Turn on water pump if soil is dry
      digitalWrite(relayPin, HIGH);
    } else if (soilMoistureValue > 707) { // Turn off water pump if soil is wet
      digitalWrite(relayPin, LOW);
    }

    // Create the JSON string with temperature, humidity, and soil moisture
    snprintf(msg, MSG_BUFFER_SIZE, "{\"temperature\":%.2f,\"humidity\":%.2f,\"soilMoisture\":%d}", temperature, humidity, soilMoistureValue);

    // Publish the message to MQTT topic "sensors"
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensors", msg);
  }
}
