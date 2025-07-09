#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include <WiFiClient.h> 
const char* ssid = "Логин точки доступа"; // сюда логин      
const char* password = "Пароль"; // сюда название

const int SYSTEM_LED_PIN = LED_BUILTIN; 
const int BUTTON_PIN = A0; 
const int BUTTON_THRESHOLD = 100; 

const char* serverAddress = "http://айпишник:порт/button_pressed"; 

bool previousButtonState = false; 

WiFiClient client; 

void setup() {
  pinMode(SYSTEM_LED_PIN, OUTPUT);
  digitalWrite(SYSTEM_LED_PIN, HIGH); 

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(SYSTEM_LED_PIN, LOW);
    delay(100);
    digitalWrite(SYSTEM_LED_PIN, HIGH);
    delay(100);
  }
  digitalWrite(SYSTEM_LED_PIN, LOW); 
  delay(1000); 
  digitalWrite(SYSTEM_LED_PIN, HIGH); 
}

void loop() {
  int analogValue = analogRead(BUTTON_PIN);
  bool currentButtonState = (analogValue > BUTTON_THRESHOLD);

  if (currentButtonState == true && previousButtonState == false) {
    digitalWrite(SYSTEM_LED_PIN, LOW); 

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      http.begin(client, serverAddress); 
      int httpCode = http.GET(); 

      http.end(); 
    }
  } 
  else if (currentButtonState == false && previousButtonState == true) {
    digitalWrite(SYSTEM_LED_PIN, HIGH); 
  }

  previousButtonState = currentButtonState;
  delay(20); 
}