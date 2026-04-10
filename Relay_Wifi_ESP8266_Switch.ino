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

#define APP_KEY       "YOUR_APP_KEY"
#define APP_SECRET    "YOUR_APP_SECRET"
#define device_ID_1   "DEVICE_ID_1"
#define device_ID_2   "DEVICE_ID_2"


#define RelayPin1     5   
#define RelayPin2     4   
#define BUTTON_PIN1   0   
#define BUTTON_PIN2   14  
#define SWITCH_PIN1   12 
#define SWITCH_PIN2   13  
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


unsigned long lastSinricProReconnect = 0;
const unsigned long SINRICPRO_RECONNECT_INTERVAL = 30000; 


unsigned long lastWiFiRetry = 0;
const unsigned long WIFI_RETRY_INTERVAL = 10000; 

void setupRelays() {
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, LOW);
    Serial.printf("Relé en pin %d configurado\n", relayPIN);
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
    Serial.printf("Botón en pin %d configurado para dispositivo %s\n", buttonPIN, device.first.c_str());
  }

  pinMode(SWITCH_PIN1, INPUT_PULLUP);
  pinMode(SWITCH_PIN2, INPUT_PULLUP);
  Serial.printf("Switches configurados en pines %d y %d\n", SWITCH_PIN1, SWITCH_PIN2);
}

bool onPowerState(String deviceId, bool &state) {
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "ON" : "OFF");
  
  if (devices.find(deviceId) != devices.end()) {
    int relayPIN = devices[deviceId].relayPIN;
    digitalWrite(relayPIN, state ? HIGH : LOW);
    Serial.printf("Relé en pin %d cambiado a %s\n", relayPIN, state ? "HIGH" : "LOW");
    return true;
  }
  
  Serial.printf("ERROR: Device ID %s no encontrado\n", deviceId.c_str());
  return false;
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
        Serial.printf("Botón en pin %d presionado\n", buttonPIN);
      }
    } 
    else if (buttonState == HIGH && lastButtonState == LOW) {
      unsigned long pressDuration = actualMillis - button.second.buttonPressStartTime;
      button.second.isButtonPressed = false;
      Serial.printf("Botón en pin %d liberado - Duración: %lu ms\n", buttonPIN, pressDuration);


      if (pressDuration > BUTTON_LONG_PRESS_TIME) {
        Serial.println(" BOTÓN PRESIONADO POR LARGO TIEMPO - ACTIVANDO MODO AP ");
        digitalWrite(wifiLed, LOW);
        WiFiManager wifiManager;
        
        uint64_t chipID = ESP.getChipId(); 
        String apSSID = "VESIS-" + String(chipID);
        const char* apPassword = "12345678";
        
        Serial.printf("AP SSID: %s\n", apSSID.c_str());
        wifiManager.setConfigPortalTimeout(180);
        wifiManager.startConfigPortal(apSSID.c_str(), apPassword);
        
        Serial.println("Modo AP finalizado. Reiniciando ESP...");
        delay(1000);
        ESP.restart();
      } 
      
      else if (pressDuration > DEBOUNCE_TIME) {
        String deviceId = button.second.deviceId;
        
        if (devices.find(deviceId) != devices.end()) {
          int relayPIN = devices[deviceId].relayPIN;
          bool newRelayState = !digitalRead(relayPIN);  
          digitalWrite(relayPIN, newRelayState);  
          
          Serial.printf(" Botón %s - Cambiando relé a: %s \n", 
                       deviceId.c_str(), newRelayState ? "ON" : "OFF");

          if (WiFi.status() == WL_CONNECTED && SinricPro.isConnected()) {
            SinricProSwitch &mySwitch = SinricPro[deviceId];
            mySwitch.sendPowerStateEvent(newRelayState);
          } else {
            Serial.println("SinricPro no conectado - evento no enviado");
          }
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
  if (currentSwitchState1 != lastSwitchState1 && (currentMillis - lastSwitchChangeTime1 > DEBOUNCE_TIME)) {
    lastSwitchChangeTime1 = currentMillis;
    if (currentSwitchState1 == LOW) {
      bool newRelayState = !digitalRead(RelayPin1);
      digitalWrite(RelayPin1, newRelayState);
      Serial.printf("*** Switch 1 - Cambiando relé 1 a: %s ***\n", newRelayState ? "ON" : "OFF");
      if (WiFi.status() == WL_CONNECTED && SinricPro.isConnected()) {
        SinricProSwitch &mySwitch = SinricPro[device_ID_1];
        mySwitch.sendPowerStateEvent(newRelayState);
      }
    }
    lastSwitchState1 = currentSwitchState1;
  }

  bool currentSwitchState2 = digitalRead(SWITCH_PIN2);
  if (currentSwitchState2 != lastSwitchState2 && (currentMillis - lastSwitchChangeTime2 > DEBOUNCE_TIME)) {
    lastSwitchChangeTime2 = currentMillis;
    if (currentSwitchState2 == LOW) {
      bool newRelayState = !digitalRead(RelayPin2);
      digitalWrite(RelayPin2, newRelayState);
      Serial.printf("*** Switch 2 - Cambiando relé 2 a: %s ***\n", newRelayState ? "ON" : "OFF");
      if (WiFi.status() == WL_CONNECTED && SinricPro.isConnected()) {
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

void attemptWiFiConnection() {
  Serial.println("Intentando conectar a WiFi con credenciales guardadas...");
  WiFi.begin();
  currentWiFiStatus = WIFI_CONNECTING;
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(100);
    updateWiFiLED(); 
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado exitosamente");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    currentWiFiStatus = WIFI_CONNECTED;
  } else {
    Serial.println("Fallo conexión WiFi");
    currentWiFiStatus = WIFI_DISCONNECTED;
    WiFi.disconnect();
  }
}

void setupWiFi() {
 
  if (WiFi.SSID() != "") {
    Serial.printf("Red guardada encontrada: %s\n", WiFi.SSID().c_str());
    attemptWiFiConnection();
  } else {
   
    Serial.println("No hay redes WiFi guardadas. Activando modo AP para configuración inicial...");
    digitalWrite(wifiLed, LOW);
    WiFiManager wifiManager;
    uint64_t chipID = ESP.getChipId(); 
    String apSSID = "VESIS-" + String(chipID);
    const char* apPassword = "12345678";
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.startConfigPortal(apSSID.c_str(), apPassword);
    Serial.println("Configuración completada. Reiniciando...");
    delay(1000);
    ESP.restart();
  }
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED && currentWiFiStatus != WIFI_CONNECTING) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastWiFiRetry >= WIFI_RETRY_INTERVAL) {
      lastWiFiRetry = currentMillis;
      Serial.println("Reintentando conexión WiFi...");
      attemptWiFiConnection();
    }
  }
}

void updateWiFiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    if (currentWiFiStatus != WIFI_CONNECTED) {
      currentWiFiStatus = WIFI_CONNECTED;
      Serial.println("WiFi conectado");
    }
  } else {
    if (currentWiFiStatus == WIFI_CONNECTED) {
      Serial.println("Conexión WiFi perdida");
      currentWiFiStatus = WIFI_DISCONNECTED;
    }
    reconnectWiFi();
  }
}

void handleSinricProReconnection() {
  unsigned long currentMillis = millis();
  if (WiFi.status() == WL_CONNECTED && !SinricPro.isConnected() && 
      (currentMillis - lastSinricProReconnect >= SINRICPRO_RECONNECT_INTERVAL)) {
    lastSinricProReconnect = currentMillis;
    Serial.println("Reconectando SinricPro...");
    SinricPro.begin(APP_KEY, APP_SECRET);
  }
}

void restoreRelayStates() {
  delay(1000);
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    bool relayState = digitalRead(relayPIN); 
    Serial.printf("Restaurando estado del relé para %s (pin %d): %s\n", 
                 device.first.c_str(), relayPIN, relayState ? "ON" : "OFF");
    if (SinricPro.isConnected()) {
      SinricProSwitch &mySwitch = SinricPro[device.first.c_str()];
      mySwitch.sendPowerStateEvent(relayState);
      delay(100);
    }
  }
}

void setupSinricPro() {
  Serial.println("Configurando SinricPro...");
  for (auto &device : devices) {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
    Serial.printf("Dispositivo %s configurado\n", deviceId);
  }
  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
  Serial.println("SinricPro inicializado");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n INICIANDO ESP8266 \n");
  
  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  setupRelays();
  setupButtons();
  setupWiFi();
  setupSinricPro();
  
  Serial.println(" SETUP COMPLETADO \n");
}

void loop() {
  updateWiFiLED();
  updateWiFiStatus();
  handleSinricProReconnection();
  SinricPro.handle();
  handleButtons();
  handleSwitches();
}
