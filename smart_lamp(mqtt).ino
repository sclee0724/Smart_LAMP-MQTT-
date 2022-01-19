#include <WiFi.h> // Para el ESP32
WiFiClient WIFI_CLIENT;
#include <PubSubClient.h>
PubSubClient MQTT_CLIENT;

const char* ssid = "ketri";
const char* password = "ketri2484";

#define LED1 D2 
#define LED2 D3 

int TRIG = D9;                         
int ECHO = D8;
long lastMsg = 0;
float  ultrasonic_sensor;
String packet;

char msg[50];
String message = "";

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting with ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}

Serial.println("");
Serial.print("WiFi conected. IP: ");
Serial.println(WiFi.localIP());

// Setting Callback.
  MQTT_CLIENT.setCallback(callback);
}

// What to do when it receives the data. 
void callback(char* recibido, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.print(recibido);
  Serial.print("   ");
  message = "";
  for (int i=0;i<length;i++) {
    message += (char)payload[i];
  }
   Serial.println(message);
   if (message == "Set LED12 ON") {digitalWrite(LED1, HIGH);}
   if (message == "Set LED12 OFF") {digitalWrite(LED1, LOW);}
   if (message == "Set LED14 ON") {digitalWrite(LED2, HIGH);}
   if (message == "Set LED14 OFF") {digitalWrite(LED2, LOW);}
}
 
void loop() {
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }

  // PUBLISH topic.
  // Does not Publish.
  
  MQTT_CLIENT.loop(); // Check Subscription.

  ultrasonic_sensor = ultrasonic_sensor_value();
  Serial.print(ultrasonic_sensor);
  mqtt_publish(ultrasonic_sensor);
}

float ultrasonic_sensor_value(){
  
  long duration, distance;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  duration = pulseIn (ECHO, HIGH);        
  distance = duration * 17 / 1000;      
  return distance;
}

// Reconecta con MQTT broker
void reconnect() {
MQTT_CLIENT.setServer("broker.hivemq.com", 1883);  
//MQTT_CLIENT.setServer("mqtt.eclipse.org", 1883);
MQTT_CLIENT.setClient(WIFI_CLIENT);

// Trying connect with broker.
while (!MQTT_CLIENT.connected()) {
Serial.println("Trying to connect with Broker MQTT.");
MQTT_CLIENT.connect("JuanAntonio"); // it isn't necessary..
MQTT_CLIENT.subscribe("juan/boton12"); // HERE SUBSCRIBE.
MQTT_CLIENT.subscribe("juan/boton14"); // HERE SUBSCRIBE.

// Wait to try to reconnect again...
delay(3000);
}

Serial.println("Conectado a MQTT.");
}

void mqtt_publish(float ultrasonic_sensor){
  if (!MQTT_CLIENT.connected()) {
    reconnect();
  }
//  MQTT_CLIENT.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    packet = String(ultrasonic_sensor) ; 
    packet.toCharArray(msg, 50); 
    Serial.print("Publish message: ");
    Serial.println(msg);
    MQTT_CLIENT.publish("juan/ul", msg);
  }
  delay(1000); 
}
