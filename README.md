
>👩🏻‍💻Feby 🗓 7 June 2025

# MONITORING CURRENT WEATHER FROM OPENWEATHERMAP API VIA ESP 32-BASED WIFI CONNECTION

---
## **A. Introduction**

In this practicum, an Internet of Things (IoT)-based weather monitoring system is developed by utilizing the ESP32 microcontroller as the control center. The system is designed to be able to access and display the latest weather data from the OpenWeatherMap service via a Wi-Fi network. The information displayed includes temperature, weather conditions, humidity, and wind speed. 

## **B. Diagram**
This diagram was created using the ESP32 Microcontroller based Wokwi simulation.
<img width="1020" alt="1" src="https://github.com/user-attachments/assets/01b3b342-ff0d-476c-8f4b-2e15da6bd9e0" />







```json
{
  "version": 1,
  "author": "Cagita Dian A'yunin",
  "editor": "wokwi",
  "parts": [
    { "type": "wokwi-esp32-devkit-v1", "id": "esp", "top": -50, "left": -100, "attrs": {} },
    {
      "type": "wokwi-lcd1602",
      "id": "lcd",
      "top": -32,
      "left": 130.4,
      "attrs": { "pins": "i2c" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btnNext",
      "top": 35,
      "left": -201.6,
      "attrs": { "color": "green", "label": "Next" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btnPrev",
      "top": -32.2,
      "left": -201.6,
      "attrs": { "color": "blue", "label": "Prev" }
    }
  ],
  "connections": [
    [ "esp:TX0", "$serialMonitor:RX", "", [] ],
    [ "esp:RX0", "$serialMonitor:TX", "", [] ],
    [ "lcd:SCL", "esp:D22", "black", [ "h-100", "v-40" ] ],
    [ "lcd:SDA", "esp:D21", "black", [ "h-90", "v-20" ] ],
    [ "lcd:VCC", "esp:3V3", "black", [ "h-110", "v60" ] ],
    [ "lcd:GND", "esp:GND.1", "black", [ "h-130", "v40" ] ],
    [ "btnNext:1.l", "esp:D18", "green", [ "h-30", "v-20" ] ],
    [ "btnNext:2.l", "esp:GND.2", "black", [ "h-30", "v20" ] ],
    [ "btnPrev:1.l", "esp:D19", "green", [ "h-30", "v-20" ] ],
    [ "btnPrev:2.l", "esp:GND.2", "black", [ "h-30", "v20" ] ]
  ],
  "dependencies": {}
}


```
## **C. Program Code**
This program code is for setting the temperature and humidity, as well as the connection to Blynk.

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>            
#include <HTTPClient.h>
#include <ArduinoJson.h>     

const char* ssid = "Wokwi-GUEST";  
const char* password = "";         
String apiKey = "aa183d09b9792e4ef1d50d33a2222699";  
String city = "Malang City, East Java";           
String units = "metric";           
String server = "http://api.openweathermap.org/data/2.5/weather?q=Malang&units=metric&appid=aa183d09b9792e4ef1d50d33a2222699";

LiquidCrystal_I2C lcd(0x27, 16, 2);  

String displayText = "";             
int scrollIndex = 0;                 

unsigned long lastUpdateTime = 0;    
unsigned long lastScrollTime = 0;    
const long updateInterval = 60000;   
const long scrollInterval = 300;     

const int buttonNextPin = 18;        
const int buttonPrevPin = 19;        
int currentPage = 0;                 
const int totalPages = 4;            
unsigned long lastDebounceTime = 0;  
const long debounceDelay = 200;      

String temp = "";                    
String desc = "";                    
String humidity = "";                
String wind = "";                    

void updateWeather();
void showPage(int page);
void scrollDisplay();


void setup() {
  Serial.begin(115200);  

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Weather Info:");

  pinMode(buttonNextPin, INPUT_PULLUP);  
  pinMode(buttonPrevPin, INPUT_PULLUP); 

  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  delay(2000);  
  lcd.clear();


  updateWeather();
  showPage(currentPage);  
}

void loop() {
  unsigned long currentMillis = millis();  
  if (currentMillis - lastUpdateTime >= updateInterval) {
    updateWeather();
    lastUpdateTime = currentMillis;
    showPage(currentPage); 
  }
  if (currentPage == 2) {
    if (currentMillis - lastScrollTime >= scrollInterval) {
      scrollDisplay();
      lastScrollTime = currentMillis;
    }
  }
  if (digitalRead(buttonNextPin) == LOW) {
    if (millis() - lastDebounceTime > debounceDelay) {
      currentPage++;
      if (currentPage >= totalPages) currentPage = 0;  
      showPage(currentPage);
      lastDebounceTime = millis();
    }
  }
  if (digitalRead(buttonPrevPin) == LOW) {
    if (millis() - lastDebounceTime > debounceDelay) {
      if (currentPage == 0) currentPage = totalPages - 1;  
      else currentPage--;
      showPage(currentPage);
      lastDebounceTime = millis();
    }
  }

}

void updateWeather() {
  if (WiFi.status() == WL_CONNECTED) {  
    HTTPClient http;
    http.begin(server);    
    int httpCode = http.GET();  
    
    if (httpCode > 0) {  
      String payload = http.getString(); 
      Serial.println(payload);   
      
      
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);
      
      
      temp = String(doc["main"]["temp"].as<float>());
      desc = doc["weather"][0]["description"].as<String>();
      humidity = String(doc["main"]["humidity"].as<int>());
      wind = String(doc["wind"]["speed"].as<float>());
      
      
      displayText = "| Temp: " + temp + " C | " + desc;
      scrollIndex = 0;  
    } else {
      Serial.println("Error on HTTP request");
    }
    
    http.end();  
  }
}



void showPage(int page) {
  lcd.clear();
  switch (page) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Location:");
      lcd.setCursor(0, 1);
      lcd.print(city);
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Temperature:");
      lcd.setCursor(0, 1);
      lcd.print(temp + " C");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Weather:");
      lcd.setCursor(0, 1);
      if (desc.length() > 16) {
        lcd.print(desc.substring(0, 16));  
      } else {
        lcd.print(desc);
      }
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Wind: " + wind + " m/s");
      lcd.setCursor(0, 1);
      lcd.print("Humidity: " + humidity + "%");
      break;
  }
}



void scrollDisplay() {
  if (displayText.length() > 0) {
    lcd.setCursor(0, 1);
    if (scrollIndex + 16 <= displayText.length()) {
      lcd.print(displayText.substring(scrollIndex, scrollIndex + 16)); 
    } else {
      String part1 = displayText.substring(scrollIndex);
      String part2 = displayText.substring(0, 16 - part1.length());
      lcd.print(part1 + part2); 
    }
    
    scrollIndex++;
    if (scrollIndex >= displayText.length()) {
      scrollIndex = 0; 
    }
  }
}

```
## **C. Result**
The information obtained includes air temperature in Celsius, description of weather conditions such as “clear sky” or “overcast clouds”, air humidity in percent, and wind speed in meters per second. The data is automatically updated every 60 seconds to ensure the information displayed remains accurate and up-to-date. All this weather information is displayed alternately on four different pages on the LCD screen. The first page (page 0) displays the name of the city, i.e. “Malang”, the second page displays the current air temperature, the third page presents a description of the weather conditions equipped with a scrolling feature when the text exceeds 16 characters, and the fourth page shows wind speed and air humidity data.
<img width="1020" alt="2" src="https://github.com/user-attachments/assets/3d677a78-b4a1-4a6d-9733-3a37a852510d" />

System navigation is done using two buttons with a debouncing mechanism for 200 milliseconds, so that switching between pages feels more responsive and stable. Button testing shows results that are in accordance with their functions, namely the “Next” button is used to move to the next page, while the “Prev” button is used to return to the previous page. The whole process of experimentation and system testing was carried out using the Wokwi simulation platform and Visual Studio Code, with the configuration of ESP32, I2C LCD display, and two push buttons. The simulation results show that the system functions as designed and is able to optimally execute all predefined features.
<img width="1020" alt="3" src="https://github.com/user-attachments/assets/7ffd8f42-eaed-4d4d-95f8-18edc00b0506" />




For more details, please follow the implementation steps in the Weather API check report (pdf).
