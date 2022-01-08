#include <esp_now.h>
#include <WiFi.h>
#include "xbm.h"
#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0x80, 0x7D, 0x3A, 0x6E, 0x8D, 0x23};


uint16_t calData[5] = { 520, 3372, 831, 2476, 7 }; // callibration point for touch screen


// Switch position and size
#define FRAME_X 20
#define FRAME_Y 10
#define FRAME_W 120
#define FRAME_H 50

// Red zone size
#define REDBUTTON_X FRAME_X
#define REDBUTTON_Y FRAME_Y
#define REDBUTTON_W (FRAME_W/2)
#define REDBUTTON_H FRAME_H

// Green zone size
#define GREENBUTTON_X (REDBUTTON_X + REDBUTTON_W)
#define GREENBUTTON_Y FRAME_Y
#define GREENBUTTON_W (FRAME_W/2)
#define GREENBUTTON_H FRAME_H

#define X_Button_Margin 150
#define Y_Button_Margin 100


// Define variables to store BME280 readings to be sent
float temperature = 11;
float humidity = 12;
float pressure = 13;

// Define variables to store incoming readings
float incomingTemp;
float incomingHum;
float incomingPres;

String TEMP_DATA;
String HUM_DATA;

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

// Create a struct_message called switch_reading to hold sensor readings
struct_message switch_reading;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0) {
    success = "Delivery Success :)";
  }
  else {
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);


  TEMP_DATA = (String)incomingReadings.temp;
  HUM_DATA = (String)incomingReadings.hum;

  Serial.println(TEMP_DATA);
  Serial.println(HUM_DATA);
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);
  Serial.println("Code start");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);


  tft.init();

  tft.setRotation(3);

  // clear screen
  tft.fillScreen(TFT_BLACK);

  buttons(); // print buttons on screen
}

void loop() {

  uint16_t x, y;

  // See if there's any touch data for us
  if (tft.getTouch(&x, &y))
  {
    int Touch_Y_Cor = map(y, 50, 150, 0, 240);

    Serial.print("x - "); Serial.println(x);
    Serial.println("y - "); Serial.println(y);


    if ((x > REDBUTTON_X + X_Button_Margin) && (x < (REDBUTTON_X + REDBUTTON_W + X_Button_Margin ) )) {
      if ((Touch_Y_Cor > REDBUTTON_Y) && (Touch_Y_Cor <= (REDBUTTON_Y + REDBUTTON_H))) {
        Serial.println("Switch 1 OFF");
        switch_reading.sw1 = 0;
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));
      }
    }

    if ((x > REDBUTTON_X) && (x < (REDBUTTON_X + REDBUTTON_W))) {
      if ((Touch_Y_Cor > REDBUTTON_Y) && (Touch_Y_Cor <= (REDBUTTON_Y + REDBUTTON_H))) {
        Serial.println("Switch 2 OFF");
        switch_reading.sw2 = 0;
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));
      }
    }

    if ((x > REDBUTTON_X + X_Button_Margin) && (x < (REDBUTTON_X + REDBUTTON_W + X_Button_Margin ) )) {
      if ((Touch_Y_Cor > REDBUTTON_Y + Y_Button_Margin ) && (Touch_Y_Cor <= ((REDBUTTON_Y + Y_Button_Margin) + REDBUTTON_H))) {
        Serial.println("Switch 3 OFF");
        switch_reading.sw3 = 0;
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));
      }
    }

    if ((x > REDBUTTON_X) && (x < (REDBUTTON_X + REDBUTTON_W))) {
      if ((Touch_Y_Cor > REDBUTTON_Y + Y_Button_Margin ) && (Touch_Y_Cor <= (REDBUTTON_Y + REDBUTTON_H + Y_Button_Margin))) {
        Serial.println("Switch 4 OFF");
        switch_reading.sw4 = 0;
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));
      }
    }

    {
      if ((x > GREENBUTTON_X + X_Button_Margin ) && (x < (GREENBUTTON_X + GREENBUTTON_W + X_Button_Margin ))) {
        if ((Touch_Y_Cor > GREENBUTTON_Y) && (Touch_Y_Cor <= (REDBUTTON_Y + GREENBUTTON_H))) {
          Serial.println("Switch 1 ON");
          switch_reading.sw1 = 1;
          // Send message via ESP-NOW
          esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));

        }
      }

      if ((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W))) {
        if ((Touch_Y_Cor > GREENBUTTON_Y) && (Touch_Y_Cor <= (REDBUTTON_Y + GREENBUTTON_H))) {
          Serial.println("Switch 2 ON");
          switch_reading.sw2 = 1;
          // Send message via ESP-NOW
          esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));

        }
      }

      if ((x > GREENBUTTON_X + X_Button_Margin ) && (x < (GREENBUTTON_X + GREENBUTTON_W + X_Button_Margin ))) {
        if ((Touch_Y_Cor > GREENBUTTON_Y + Y_Button_Margin) && (Touch_Y_Cor <= (REDBUTTON_Y + GREENBUTTON_H + Y_Button_Margin))) {
          Serial.println("Switch 3 ON");
          switch_reading.sw3 = 1;
          // Send message via ESP-NOW
          esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));

        }
      }
      if ((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W))) {
        if ((Touch_Y_Cor > GREENBUTTON_Y + Y_Button_Margin) && (Touch_Y_Cor <= (REDBUTTON_Y + GREENBUTTON_H + Y_Button_Margin))) {
          Serial.println("Switch 4 ON");
          switch_reading.sw4 = 1;
          // Send message via ESP-NOW
          esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &switch_reading, sizeof(switch_reading));

        }
      }
    }

  }
  else
  {
    print_data_on_screen();
  }
}



void buttons()
{
  tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, TFT_CYAN);
  tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("OFF", GREENBUTTON_X + (GREENBUTTON_W / 2), GREENBUTTON_Y + (GREENBUTTON_H / 2));
  tft.drawString("ON", (REDBUTTON_X) + (REDBUTTON_W / 2) + 1, REDBUTTON_Y + (REDBUTTON_H / 2));

  tft.fillRect((REDBUTTON_X + X_Button_Margin ), REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, TFT_CYAN);
  tft.fillRect((GREENBUTTON_X + X_Button_Margin), GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("OFF", (GREENBUTTON_X + X_Button_Margin) + (GREENBUTTON_W / 2), GREENBUTTON_Y + (GREENBUTTON_H / 2));
  tft.drawString("ON", (REDBUTTON_X + X_Button_Margin) + (REDBUTTON_W / 2) + 1, REDBUTTON_Y + (REDBUTTON_H / 2));

  tft.fillRect(REDBUTTON_X, (REDBUTTON_Y + Y_Button_Margin) , REDBUTTON_W, REDBUTTON_H, TFT_CYAN);
  tft.fillRect(GREENBUTTON_X, (GREENBUTTON_Y + Y_Button_Margin) , GREENBUTTON_W, GREENBUTTON_H, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("OFF", GREENBUTTON_X + (GREENBUTTON_W / 2), ( GREENBUTTON_Y + Y_Button_Margin) + (GREENBUTTON_H / 2));
  tft.drawString("ON", (REDBUTTON_X) + (REDBUTTON_W / 2) + 1, (REDBUTTON_Y + Y_Button_Margin) + (REDBUTTON_H / 2));


  tft.fillRect((REDBUTTON_X + X_Button_Margin ), (REDBUTTON_Y + Y_Button_Margin), REDBUTTON_W, REDBUTTON_H, TFT_CYAN);
  tft.fillRect((GREENBUTTON_X + X_Button_Margin), (GREENBUTTON_Y + Y_Button_Margin), GREENBUTTON_W, GREENBUTTON_H, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("OFF", (GREENBUTTON_X + X_Button_Margin) + (GREENBUTTON_W / 2), (GREENBUTTON_Y + Y_Button_Margin) + (GREENBUTTON_H / 2));
  tft.drawString("ON", (REDBUTTON_X + X_Button_Margin) + (REDBUTTON_W / 2) + 1, (REDBUTTON_Y + Y_Button_Margin) + (REDBUTTON_H / 2));


  tft.fillRect(0, (REDBUTTON_Y + Y_Button_Margin + 70), 160, REDBUTTON_H, TFT_GREEN);
  tft.fillRect(160, (GREENBUTTON_Y + Y_Button_Margin + 70), 160, GREENBUTTON_H, TFT_RED);
  tft.setTextColor(TFT_BLACK);

}

void print_data_on_screen()
{
  tft.fillRect(0, (REDBUTTON_Y + Y_Button_Margin + 70), 160, REDBUTTON_H, TFT_GREEN);
  tft.fillRect(160, (GREENBUTTON_Y + Y_Button_Margin + 70), 160, GREENBUTTON_H, TFT_RED);
  tft.setTextColor(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Temp " + TEMP_DATA, 90, (REDBUTTON_Y + Y_Button_Margin + 70) + (REDBUTTON_H / 2));
  tft.drawString("Hum " +  HUM_DATA, 230, (GREENBUTTON_Y + Y_Button_Margin + 70 ) + (GREENBUTTON_H / 2));
}
