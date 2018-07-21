#include <FS.h>

#include <EEPROM.h> //Store counter in EEPROM memory to avoid sudden reset
#include <Wire.h>// I2C is used to communicate with sensors
// #include <VL53L0X.h>
#include <Ticker.h>

#include <ESP8266WiFi.h>// WiFi Library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define DAQ_INTERVAL 60 //5 minutes


Ticker DAQTimer;


volatile bool measurementFlag= true;
char sensorID[34] = "";
bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, HIGH);
  Wire.begin();
  wifiInit();
  DAQTimer.attach(DAQ_INTERVAL, setMeasurementFlag);
}

void loop() {
//   runDAQ(); 

  yield();
}


void wifiInit() {
  Serial.println("mounting FS...");
//  SPIFFS.format();
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json"); 
          strcpy(sensorID, json["sensorID"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
  WiFiManagerParameter custom_sensorID("sensorID", "Location", sensorID, 32);
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_sensorID);
  
  wifiManager.setTimeout(120);
  wifiManager.setConnectTimeout(30);


  if (!wifiManager.autoConnect(String(ESP.getChipId()).c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");  

  //save the custom parameters to FS
  if (shouldSaveConfig) {
  strcpy(sensorID, custom_sensorID.getValue());
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["sensorID"] = sensorID;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void postData(){
  digitalWrite(LED_BUILTIN, LOW);
  HTTPClient http;
  http.setReuse(true);
  static char msg[50];
  static int port = 443;
  String fingerprint = "d0ef874071f9f864abf5c1dc6376b591ee989866";
  String host = "konttiserver.ddns.net";
  String urlAPI = "/api/v1/hamk/tung";

//   snprintf(msg, 75, "{\"countIn\": %lu, \"countOut\": %lu,\"id\": \"%s\"}", countIn, countOut,sensorID);
  Serial.print("Sensor location: ");
  Serial.println(sensorID);
//   http.begin(host, port, urlAPI, fingerprint);
//   http.addHeader("Content-Type", "application/json");
//   int postcode = http.POST(msg);

  String payload = http.getString();
//   http.end();
  Serial.println("Sent");
  digitalWrite(LED_BUILTIN, HIGH);
  measurementFlag = false;
}

void setMeasurementFlag() {
  measurementFlag = true;
}


