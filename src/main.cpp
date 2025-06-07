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
