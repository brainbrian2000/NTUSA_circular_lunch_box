#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include "OLED096_function.cpp"
#include <Wire.h>
#include "Wifi_connect.cpp"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#define I2C_SDA 15
#define I2C_SCL 13
#define BOTTOM_1 12
#define BOTTOM_2 14
#define BOTTOM_3 2
#define using_led 0
#define state_1_time 10
#if using_led
  #define LED 4
#endif
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
int state;
WiFi_connection wifi;
String Printing;
String BOXID;
String USERID;
String Prev_ID;
struct QRCodeData qrCodeData;
OLED096 oled;
int state_1_time_count = 0;
void onQrCodeTask(void *pvParameters)
{

  while (true)
  {
    if(state==1){
      #if using_led
        digitalWrite(LED, HIGH);
      #endif
      if (reader.receiveQrCode(&qrCodeData, 100))
      {

        if(String((const char *)qrCodeData.payload) == Prev_ID &&Prev_ID!=""){
          Serial.println("Same ID");
          Prev_ID = "";
          vTaskDelay(100 / portTICK_PERIOD_MS);
          continue;
        }


        Serial.println("Found QRCode");
        if (qrCodeData.valid)
        {
          // state = 1;
          Serial.print("Payload: ");
          Serial.println((const char *)qrCodeData.payload);
          BOXID = String((const char *)qrCodeData.payload);
          // state = 2;
          Printing = BOXID;
          #if using_led
            digitalWrite(LED, LOW);
          #endif
          vTaskDelay(10 / portTICK_PERIOD_MS);
          state_1_time_count=0;
          state = 4;
          /*
          if (reader.receiveQrCode(&qrCodeData, 100))
          {
            //For cleaning camera buffer
            if (qrCodeData.valid)
            {
              // state = 1;
              Serial.print("Payload: ");
              Serial.println((const char *)qrCodeData.payload);
              vTaskDelay(10 / portTICK_PERIOD_MS);
              state_1_time_count=0;
            }
            else
            {
              Serial.print("Invalid: ");
              Serial.println((const char *)qrCodeData.payload);
            }
          }
          */
          for (int i =0;i<1024;i++){
            qrCodeData.payload[i] = 0;
          }
          qrCodeData.valid = false;
          qrCodeData.payloadLen = 0;  
        }
        else
        {
          Serial.print("Invalid: ");
          Serial.println((const char *)qrCodeData.payload);
          continue;
        }
      }
      state_1_time_count++;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void display_oled(void *pvParameters){
  while(true){
    if(state == 1){
      oled.print_oled("Scanning");
    }else if (state == 4){
      // oled.print_oled("Check");
      oled.print_oled(String("Check\n")+BOXID+String("\n")+USERID);
    }else if (state == 0){
      oled.print_oled("Waiting Check");
    }else if (state == 2){
      oled.print_oled("Server Checking");
    }else if (state == 3){
      oled.print_oled("Server Done");
    }else if (state == 5){
      oled.print_oled("Turn Right\nThank You");
    }else if (state == 9){
      oled.print_oled("Turn Left\nThank You");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void state_change(void *pvParameters){
  
  while(true){
    // Serial.println(state); 
    if(state ==9){
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      state=0;
    }
    else if(state == 0 && digitalRead(BOTTOM_1)== HIGH){
      Printing = "";
      BOXID = "";
      USERID = "";
      state = 1;
    }else if(state!=1&&digitalRead(BOTTOM_3)==HIGH){
      state = 0;
      // reader.receiveQrCode(&qrCodeData, 100);
      // for (int i =0;i<10;i++){
      //   reader.receiveQrCode(&qrCodeData, 100);
      //   vTaskDelay(10 / portTICK_PERIOD_MS);
      // }
      for (int i =0;i<1024;i++){
        qrCodeData.payload[i] = 0;
      }
      qrCodeData.valid = false;
      qrCodeData.payloadLen = 0;  
      Printing = "";
      BOXID = "";
      USERID = "";
      Serial.println("TurnLeft");
      vTaskDelay(100/ portTICK_PERIOD_MS  );
    }else if (state == 1&&digitalRead(BOTTOM_3)==HIGH){
      // for (int i =0;i<10;i++){
      //   reader.receiveQrCode(&qrCodeData, 100);
      //   vTaskDelay(10 / portTICK_PERIOD_MS);
      // }
      Prev_ID = "";
      Prev_ID = String((const char *)qrCodeData.payload);
      state = 9;
        for (int i =0;i<1024;i++){
          qrCodeData.payload[i] = 0;
        }
      // vTaskDelay(1000/ portTICK_PERIOD_MS);
      // for (int i =0;i<10;i++){
      //   reader.receiveQrCode(&qrCodeData, 100);
      //   vTaskDelay(10 / portTICK_PERIOD_MS);
      // }
      qrCodeData.valid = false;
      qrCodeData.payloadLen = 0;  
      Printing = "";
      BOXID = "";
      USERID = "";
    }else if (state == 4 && digitalRead(BOTTOM_2)== HIGH){
      state = 5;
      for (int i =0;i<1024;i++){
        qrCodeData.payload[i] = 0;
      }
      qrCodeData.valid = false;
      qrCodeData.payloadLen = 0;  
      Serial.println("TurnRight");
    }
    else if (state_1_time_count>state_1_time*10){
      state = 0;
      state_1_time_count = 0;
      #if using_led
        digitalWrite(LED, LOW);
      #endif
      Serial.println("TimeOut");
    }else if (state==5){
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      Serial.println("TurnLeft");
      for (int i =0;i<1024;i++){
        qrCodeData.payload[i] = 0;
      }
      qrCodeData.valid = false;
      qrCodeData.payloadLen = 0;  
      state = 0;
      // reader.receiveQrCode(&qrCodeData, 100);
    }else if (state !=1){
      #if using_led
        digitalWrite(LED, LOW);
      #endif
      for (int i =0;i<1024;i++){
        qrCodeData.payload[i] = 0;
      }
      qrCodeData.valid = false;
      qrCodeData.payloadLen = 0;  
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void wifi_check(void *pvParameters){
  while(true){
    wifi.connect_check();
    // wifi.printWiFiConfig();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.println("Start setup");
  wifi.turn_on_WiFi(0,1);
  wifi.turn_on_WiFi(1,1);
  Wire.begin(I2C_SDA, I2C_SCL);
  oled.init();
  reader.setup();
  
  pinMode(BOTTOM_1, INPUT_PULLDOWN);
  pinMode(BOTTOM_2, INPUT_PULLDOWN);
  pinMode(BOTTOM_3, INPUT_PULLDOWN);
  
  
  Serial.println("Setup QRCode Reader");
  reader.beginOnCore(1);
  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
  xTaskCreate(display_oled, "display_oled", 3 * 1024, NULL, 5, NULL);
  xTaskCreate(state_change, "state_change", 3 * 1024, NULL, 10, NULL);
  xTaskCreate(wifi_check, "wifi_check", 50 * 1024, NULL, 6, NULL);

  #if using_led
    pinMode(LED,OUTPUT);
  #endif
}

void loop()
{
  delay(100);
}