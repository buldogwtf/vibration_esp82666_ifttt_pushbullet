/*
Sources :
 - File > Examples > ES8266WiFi > WiFiClient
 - File > Examples > ES8266HTTPClient > HTTPClient
 
 Schematic:
 
 - PIR leg 1 - VCC
 - PIR leg 2 - D0 (or nr.16)
 - PIR leg 3 - GND

Versions:

Nikant - V1.0 https://gist.github.com/nikant
buldogwtf -v1.1 added static ip

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define WIFI_SSID "SSID"
#define WIFI_KEY "Password"

#define NOTIFY_URL "http://maker.ifttt.com/trigger/wasmachine/with/key/yourownkey"

#define SECOND 1000
#define QUARTER_SECOND 250

#define SENSOR_PIN 16 

bool machineRunning = false;

bool lastState = false;
int lastTripped = 0;

int tripBucket = 0;
int tripBucketLastDripped = 0;


void setup() {
  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);
}


void loop() {
  int now = millis();
  int sinceLastTripped = now - lastTripped;
  int sinceLastDrip = now - tripBucketLastDripped;

  if (tripBucket > 0 && sinceLastDrip > SECOND) {
    tripBucket--;
    tripBucketLastDripped = now;
    Serial.print("Drip! ");
    Serial.println(tripBucket);
  }

  // Read the state and see if the sensor was tripped
  bool state = digitalRead(SENSOR_PIN) == 0 ? false : true;
  if (lastState != state) {
    lastState = state;

    // Can be tripped a maximum of once per second
    if (sinceLastTripped > QUARTER_SECOND) {
      lastTripped = now;

      if (tripBucket < 40) {
        tripBucket++;
      }
    }
  }


  if (machineRunning && tripBucket == 0) {
    machineRunning = false;
    Serial.println("Machine stopped");
    sendDoneNotification();
  }

  if (!machineRunning && tripBucket > 20) {  //change number for number of fibration to trigger event
    machineRunning = true;
    Serial.println("Machine started");
  }

  delay(5);
}


void sendDoneNotification() {
 // WiFi.begin(WIFI_SSID, WIFI_KEY);

 //next 4 lines for static ip, remove for dhcp

WiFi.begin(WIFI_SSID, WIFI_KEY);
IPAddress ip(192,168,2,203);   
IPAddress gateway(192,168,2,254);   
IPAddress subnet(255,255,255,0);   
WiFi.config(ip, gateway, subnet);

//end lines for static ip

  
  while((WiFi.status() != WL_CONNECTED)) {
    delay(40);
  }

  HTTPClient http;
  http.begin(NOTIFY_URL);
  int httpCode = http.GET();
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

}
