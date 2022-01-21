/**********************************************************************************
 * The following software may be included in this software : SimpleMQTTClient
 * from 
 * Author : SCS
 * Date : 2021.12.19 : For 
 * Date : 
 **********************************************************************************/
//=================================================================================
// Firmware Version
//=================================================================================
const char* board_hardware_verion = "ET-Board_V1.1";
const char* board_firmware_verion = "ET_MQTT_SIMPLE V0.5.0";

//=================================================================================
// Include
//=================================================================================
#include "EspMQTTClient.h"
#include <ArduinoJson.h>




EspMQTTClient client(
  "ketri",
  "ketri2484",
  "broker.hivemq.com",  // MQTT Broker server ip
  "",             // Can be omitted if not needed  // Username
  "",             // Can be omitted if not needed  // Password
  "",           // Client name that uniquely identify your device
  1883                // The MQTT port, default to 1883. this line can be omitted
);


//=================================================================================
// for Board Operation LED
//=================================================================================
#define LED_BLINK_INTERVAL 500
#define LED_BUILTIN 5


//=================================================================================
// 전역변수 선언 시작
//=================================================================================
//울트라 소닉 포트
int TRIG = D9;                       
int ECHO = D8;   

// 2018.08.22 : SCS
// ESP32-WROOM-32
#define MAX_ANALOG 8
//int analogs[MAX_ANALOG]={36, 39, 32, 33, 34, 35, 25, 26};

// 2018.09.28 : SCS : K-board HW 1.0
#define MAX_DIGITAL 10           // 0  1   2   3   4   5   6   7   8   9  
int digitals[MAX_DIGITAL]      = {-1, -1, D2, D3, D4, D5, D6, D7, D8, D9};
int digitals_mode[MAX_DIGITAL] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
int digitals_value[MAX_DIGITAL]= {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
char msg[10];

// 2018.10.15 : SCS

double lastTime = 0.0;
double currentTime = 0.0;

// 2018.08.17 : SCS
unsigned lastMillis = 0;

// 2021.12.19 : SCS : dt->data, et->etboard, smpl -> simple
String mqtt_prefix = "dt/et/smpl";
String mac_address = "";

// 전역변수 선언 종료


//=================================================================================
void setup()
//=================================================================================
{
  
  // fast_blink_led
  pinMode(LED_BUILTIN, OUTPUT);  
  fast_blink_led();

  // init Serial
  Serial.begin(115200);

  // 2018.10.16 : SCS
  displayBoardInformation();

  //
  pinMode(TRIG, OUTPUT);                // 핀 모드 설정
  pinMode(ECHO, INPUT);  
  // init Ports
  //initPorts();  

  // 2018.08.17 : SCS
  lastMillis = millis();


  // serial 
  Serial.begin(115200);

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}


//=================================================================================
void loop()
//=================================================================================
{
  //-------------------------------------------------------------------------------
  // MQTT loop
  //-------------------------------------------------------------------------------
  client.loop();
    
  //-------------------------------------------------------------------------------
  //  Send sensor value
  //------------------------------------------------------------------------------- 
  if (millis() - lastMillis > g_send_interval) {  
      send_analog();
      send_digital();
      lastMillis = millis();
   }  
    
   if (is_changed_digital() == true) {
      send_digital();
   }
  
  //-------------------------------------------------------------------------------
  // Blink Operation LED
  //-------------------------------------------------------------------------------
  blinkOperation();
}


//=================================================================================
void displayBoardInformation()
//=================================================================================
{
  Serial.print("\n\n");  
  Serial.print("Welcome to ");
  Serial.print(board_hardware_verion);
  Serial.println(" !!!");
  Serial.print("Firmware version is ");
  Serial.println(board_firmware_verion);  
}


//=================================================================================
String get_topic_prefix()
//=================================================================================
{
  String prefix = "";
  prefix = mqtt_prefix + "/" + mac_address.substring(9); // only last 3 bytes
  return prefix;
}


//=================================================================================
// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
//=================================================================================
{
  // Execute delayed instructions
  client.executeDelayed(5 * 1000, []() {
    client.publish("sclee" 
        + "/info/version",  String(board_hardware_verion) + "/" + String(board_firmware_verion));
  });

  clinet.excuteDelayed(12 * 100, []() {
    clinet.publish("sclee" + "info/version", string(board_hardware_verion)
  }

  mac_address = WiFi.macAddress();

  recv_digital();
}

//=================================================================================
boolean is_changed_digital()
//=================================================================================
{
  // check if different current and previous value
  boolean bFound = false;
  for (int pinNumber = 6; pinNumber < MAX_DIGITAL; pinNumber++) {  
      int pin = digitals[pinNumber];  
      if(pin >= 0) {        
        pinMode(pin,INPUT);  
        int val = digitalRead(pin); 

        // 2018.10.15 : SCS : To reverse 4 buttons's value on board is pull-up circuit
        if( pinNumber >=6 && pinNumber <= 9) 
        {
          val = !val;
        }

        if (digitals_value[pinNumber] != val) {  
          digitals_value[pinNumber] = val;     
          bFound = true;
        }      
     }
  }      
  return bFound;
  
}

//=================================================================================
void send_digital()
//=================================================================================
{
  //if (bFound == false) return;
  
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
 
  JSONencoder["device"] = board_hardware_verion;  // Todo : remove
  JsonArray& values = JSONencoder.createNestedArray("values");

  value.add(measure_ultrasonic());
  
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
 
  if (client.publish("sclee" + "/digital", JSONmessageBuffer) == true) {
    //Serial.println("Success sending message");
  } else {
    //Serial.println("Error sending message");
  }
}


//=================================================================================
void recv_digital()
//=================================================================================
{
  client.subscribe("sclee" + "/boton12", [](const String & payload) {
  pinMode(D2, OUTPUT);
  if (payload == "0") digitalWrite(D2, LOW);
    else digitalWrite(D2, HIGH);
  });

  client.subscribe("sclee" + "/boton14", [](const String & payload) {
  pinMode(D3, OUTPUT);
  if (payload == "0") digitalWrite(D3, LOW);
    else digitalWrite(D3, HIGH);
  });
  
}

int measure_ultrasonic(){
  long duration, distance;
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
  
    duration = pulseIn (ECHO, HIGH);      // 물체에 반사되어돌아온 초음파의 시간을 저장
  
    distance = duration * 17 / 1000;
    return distance;
}

//=================================================================================
void fast_blink_led() 
//=================================================================================
{
  for(int i=0; i<10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(50);                         // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(50);                         // wait for a second
  }
}

//==============================================================================
// blinkOperation
//==============================================================================
void blinkOperation() {
  static uint8_t ledState = LOW;             // ledState used to set the LED
  static unsigned long previousMillis = 0;   // will store last time LED was updated
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= LED_BLINK_INTERVAL) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(LED_BUILTIN, ledState);  
  }    
}

//==============================================================================
// getTime
//==============================================================================
unsigned long getTime() {
  static unsigned long startTime = millis();
  unsigned long currentTime;
  
  currentTime = millis();
  unsigned long t = (currentTime - startTime) /1000;
  return (t);
}

//==============================================================================
// getTimeMS
//==============================================================================
unsigned long getTimeMS() {
  static unsigned long startTime = millis();
  unsigned long currentTime;
  
  currentTime = millis();
  unsigned long t = (currentTime - startTime);
  return (t);
}

//=================================================================================
// End of File
//=================================================================================
