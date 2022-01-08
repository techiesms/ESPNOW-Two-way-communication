

#include <ESP8266WiFi.h>
#include <espnow.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xA0, 0xC0, 0x8C};


#define Relay1            D6
#define Relay2            D2
#define Relay3            D1
#define Relay4            D5

// Digital pin connected to the DHT sensor
#define DHTPIN D7    

// Sensor Type
#define DHTTYPE    DHT11     // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Define variables to store DHT readings to be sent
float temperature;
float humidity;
bool sw;

// Define variables to store incoming readings
float incomingTemp;
float incomingHum;
bool incomingSW1;
bool incomingSW2;
bool incomingSW3;
bool incomingSW4;

// Updates DHT readings every 10 seconds
const long interval = 10000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float temp;
    float hum;
    bool sw1;
    bool sw2;
    bool sw3;
    bool sw4;
} struct_message;

// Create a struct_message called DHTReadings to hold sensor readings
struct_message DHTReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Switch 1: ");Serial.println(incomingReadings.sw1);
  Serial.print("Switch 2: ");Serial.println(incomingReadings.sw2);
  Serial.print("Switch 3: ");Serial.println(incomingReadings.sw3);
  Serial.print("Switch 4: ");Serial.println(incomingReadings.sw4);
  digitalWrite(Relay1,incomingSW1);
  digitalWrite(Relay2,incomingSW2);
  digitalWrite(Relay3,incomingSW3);
  digitalWrite(Relay4,incomingSW4);
}

void getReadings(){
  // Read Temperature
  temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  if (isnan(temperature)){
    Serial.println("Failed to read from DHT");
    temperature = 0.0;
  }
  humidity = dht.readHumidity();
  if (isnan(humidity)){
    Serial.println("Failed to read from DHT");
    humidity = 0.0;
  }

  Serial.print("Temperature -");Serial.println(temperature);
  Serial.print("Humidity -");Serial.println(humidity);
}

 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Init DHT sensor
  dht.begin();
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    //Get DHT readings
    getReadings();

    //Set values to send
    DHTReadings.temp = temperature;
    DHTReadings.hum = humidity;

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &DHTReadings, sizeof(DHTReadings));
  }
}
