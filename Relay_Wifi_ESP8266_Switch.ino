#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>  
#include <WiFiManager.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include <map>

#define APP_KEY       "9529d69f-654d-4cdf-9e03-2778703acce1"
#define APP_SECRET    "3a530804-fbc1-45b7-ae01-a63e1b9cd234-53b693a0-6a6d-4a31-95ec-2afb10afdef1"
#define device_ID_1   "676b410013f98f1416156643"
#define device_ID_2   "66b67fbe54041e4ff61a52a9"

#define RelayPin1     13  
#define RelayPin2     14 
#define BUTTON_PIN1   0  
#define BUTTON_PIN2   8 
#define SWITCH_PIN1   6  
#define SWITCH_PIN2   7  
#define wifiLed       2  
#define DEBOUNCE_TIME 50
#define BUTTON_LONG_PRESS_TIME 3000  

typedef struct {
  int relayPIN;
  int buttonPIN;
} deviceConfig_t;

std::map<String, deviceConfig_t> devices = {
    {device_ID_1, {RelayPin1, BUTTON_PIN1}},
    {device_ID_2, {RelayPin2, BUTTON_PIN2}}
};

typedef struct {
  String deviceId;
  bool lastButtonState;
  unsigned long lastButtonChange;
  unsigned long buttonPressStartTime;
  bool isButtonPressed;
} buttonConfig_t;

std::map<int, buttonConfig_t> buttons;

enum WiFiStatus {
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED
};

WiFiStatus currentWiFiStatus = WIFI_DISCONNECTED;
unsigned long previousMillis = 0;
const int fastBlinkInterval = 100;  
const int slowBlinkInterval = 400; 

void setupRelays() {
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, LOW);
  }
}

void setupButtons() {
  for (auto &device : devices) {
    buttonConfig_t buttonConfig;
    buttonConfig.deviceId = device.first;
    buttonConfig.lastButtonChange = 0;
    buttonConfig.lastButtonState = HIGH;
    buttonConfig.isButtonPressed = false;

    int buttonPIN = device.second.buttonPIN;
    buttons[buttonPIN] = buttonConfig;
    pinMode(buttonPIN, INPUT_PULLUP);
  }


  pinMode(SWITCH_PIN1, INPUT_PULLUP);
  pinMode(SWITCH_PIN2, INPUT_PULLUP);
}

bool onPowerState(String deviceId, bool &state) {
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN;
  digitalWrite(relayPIN, state ? HIGH : LOW);
  return true;
}

void handleButtons() {
  unsigned long actualMillis = millis();
  for (auto &button : buttons) {
    int buttonPIN = button.first;
    bool lastButtonState = button.second.lastButtonState;
    bool buttonState = digitalRead(buttonPIN);

    if (buttonState == LOW && lastButtonState == HIGH) {
      if (!button.second.isButtonPressed) {
        button.second.buttonPressStartTime = actualMillis;
        button.second.isButtonPressed = true;
      }
    } else if (buttonState == HIGH && lastButtonState == LOW) {
      unsigned long pressDuration = actualMillis - button.second.buttonPressStartTime;
      button.second.isButtonPressed = false;

      if (pressDuration > BUTTON_LONG_PRESS_TIME) {
        Serial.println("Botón presionado por largo tiempo. Activando modo AP...");
        WiFiManager wifiManager;

        uint64_t chipID = ESP.getChipId(); 
        String apSSID = "VESIS-" + String(chipID);
        const char* apPassword = "12345678";

        wifiManager.startConfigPortal(apSSID.c_str(), apPassword);
        Serial.println("Modo AP finalizado. Reiniciando...");
        ESP.restart();
      } else if (pressDuration > DEBOUNCE_TIME) {
        String deviceId = button.second.deviceId;
        int relayPIN = devices[deviceId].relayPIN;
        bool newRelayState = !digitalRead(relayPIN);  
        digitalWrite(relayPIN, newRelayState);  

        Serial.printf("Estado del relé de %s cambiado a: %s\n", deviceId.c_str(), newRelayState ? "ON" : "OFF");

        if (WiFi.status() == WL_CONNECTED) {
          SinricProSwitch &mySwitch = SinricPro[deviceId];
          mySwitch.sendPowerStateEvent(newRelayState);  
        }
      }
    }

    button.second.lastButtonState = buttonState;
  }
}

void handleSwitches() {
  static bool lastSwitchState1 = HIGH, lastSwitchState2 = HIGH;
  static unsigned long lastSwitchChangeTime1 = 0, lastSwitchChangeTime2 = 0;

  unsigned long currentMillis = millis();

 
  bool currentSwitchState1 = digitalRead(SWITCH_PIN1);
  bool currentSwitchState2 = digitalRead(SWITCH_PIN2);

  
  if (currentSwitchState1 != lastSwitchState1 && (currentMillis - lastSwitchChangeTime1 > DEBOUNCE_TIME)) {
    lastSwitchChangeTime1 = currentMillis;
    if (currentSwitchState1 == LOW) {
      int relayPIN = RelayPin1;  
      bool newRelayState = !digitalRead(relayPIN);
      digitalWrite(relayPIN, newRelayState);
      Serial.printf("Estado del relé 1 cambiado a: %s\n", newRelayState ? "ON" : "OFF");

      if (WiFi.status() == WL_CONNECTED) {
        SinricProSwitch &mySwitch = SinricPro[device_ID_1];
        mySwitch.sendPowerStateEvent(newRelayState);  
      }
    }
    lastSwitchState1 = currentSwitchState1;
  }

  
  if (currentSwitchState2 != lastSwitchState2 && (currentMillis - lastSwitchChangeTime2 > DEBOUNCE_TIME)) {
    lastSwitchChangeTime2 = currentMillis;
    if (currentSwitchState2 == LOW) {
      int relayPIN = RelayPin2;  
      bool newRelayState = !digitalRead(relayPIN);
      digitalWrite(relayPIN, newRelayState);
      Serial.printf("Estado del relé 2 cambiado a: %s\n", newRelayState ? "ON" : "OFF");

      if (WiFi.status() == WL_CONNECTED) {
        SinricProSwitch &mySwitch = SinricPro[device_ID_2];
        mySwitch.sendPowerStateEvent(newRelayState);  
      }
    }
    lastSwitchState2 = currentSwitchState2;
  }
}

void updateWiFiLED() {
  static bool ledState = LOW;
  unsigned long currentMillis = millis();

  if (currentWiFiStatus == WIFI_DISCONNECTED) {
    if (currentMillis - previousMillis >= fastBlinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(wifiLed, ledState);
    }
  } else if (currentWiFiStatus == WIFI_CONNECTING) {
    if (currentMillis - previousMillis >= slowBlinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(wifiLed, ledState);
    }
  } else if (currentWiFiStatus == WIFI_CONNECTED) {
    digitalWrite(wifiLed, HIGH);
  }
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    if (currentWiFiStatus != WIFI_CONNECTING) {
      Serial.println("Intentando reconectar al WiFi...");
      currentWiFiStatus = WIFI_CONNECTING;
      WiFi.begin();
    }
  } else {
    if (currentWiFiStatus != WIFI_CONNECTED) {
      Serial.println("Reconexión al WiFi exitosa");
      currentWiFiStatus = WIFI_CONNECTED;  
    }
  }
}

void updateWiFiStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    if (currentWiFiStatus == WIFI_CONNECTED) {
      Serial.println("Conexión al WiFi perdida");
      currentWiFiStatus = WIFI_DISCONNECTED;
    }
    reconnectWiFi();
  }
}

void restoreRelayStates() {
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    bool relayState = digitalRead(relayPIN); 
    Serial.printf("Restaurando estado del relé %d: %s\n", relayPIN, relayState ? "ON" : "OFF");
    SinricProSwitch &mySwitch = SinricPro[device.first.c_str()];
    mySwitch.sendPowerStateEvent(relayState); 
  }
}

void setupWiFi() {
  WiFiManager wifiManager;

  uint64_t chipID = ESP.getChipId(); 
  String apSSID = "VESIS-" + String(chipID);
  const char* apPassword = "12345678";

  currentWiFiStatus = WIFI_CONNECTING;  

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Intentando reconectar con credenciales guardadas...");
    WiFi.begin();
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 3000) {
      delay(100);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado al WiFi");
    currentWiFiStatus = WIFI_CONNECTED;  
  } else {
    Serial.println("No se pudo conectar al WiFi. Modo AP desactivado por defecto.");
    currentWiFiStatus = WIFI_DISCONNECTED;
  }
}

void setupSinricPro() {
  for (auto &device : devices) {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
  restoreRelayStates();
}

void setup() {
  DEBUG_ESP_PORT.begin(115200);
  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  setupRelays();
  setupButtons();
  setupWiFi();
  setupSinricPro();
}

void loop() {
  updateWiFiLED();
  updateWiFiStatus();
  SinricPro.handle();
  handleButtons();
  handleSwitches();  
}
