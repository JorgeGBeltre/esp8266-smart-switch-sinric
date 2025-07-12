# 🔌 ESP8266 WiFi Smart Switch with SinricPro and Physical Control

This project allows you to control two electrical devices using an **ESP8266** module. It integrates **SinricPro** connectivity for remote control via voice (Alexa, Google Assistant) or the app, along with physical control using buttons and switches. The system includes automatic WiFi management using **WiFiManager**, AP mode support, and an LED indicator for network status.

---

## 🚀 Main Features

- ✅ Control **2 relays** via SinricPro (Alexa/Google Assistant)
- ✅ Physical buttons and switches for manual control
- ✅ Long press activates **WiFi configuration (AP) mode**
- ✅ WiFi status is indicated by LED (fast, slow, solid)
- ✅ Automatic restoration of relay states on restart
- ✅ Modular and scalable code using `std::map`
- ✅ Compatibility with multiple SinricPro devices

---

## 📦 Required Hardware

- ESP8266 (NodeMCU or Wemos D1 mini recommended)
- 2x 5V Relays
- 2x Physical buttons
- 2x Switches
- 1x LED (WiFi indicator)
- Pull-up resistors (if not using `INPUT_PULLUP`)
- Suitable power supply (5V/3.3V)

---

## 🧠 System Architecture

| Component         | Function                                     |
|------------------|----------------------------------------------|
| `SinricProSwitch` | Interface with SinricPro API                 |
| `WiFiManager`     | WiFi connection and configuration management |
| `std::map`        | Pin and device association                   |
| `Relay`           | Controlled output (ON/OFF)                   |
| `Physical button` | Direct relay control + AP mode activation    |
| `Switch`          | Alternative manual control                   |
| `LED`             | WiFi network status                          |

---

## 🔧 Configured Pins

```cpp
RelayPin1     = GPIO13
RelayPin2     = GPIO14
BUTTON_PIN1   = GPIO0   // can also act as AP mode button
BUTTON_PIN2   = GPIO8
SWITCH_PIN1   = GPIO6
SWITCH_PIN2   = GPIO7
wifiLed       = GPIO2   // WiFi status LED
```

> ⚠️ Some pins may vary depending on your ESP8266 board. Make sure to verify compatibility with your hardware.

---

## ☁️ SinricPro Integration

Create a project in [SinricPro](https://portal.sinric.pro/) and obtain:

- APP KEY  
- APP SECRET  
- Device IDs  

Example:

```cpp
#define APP_KEY       "YOUR_APP_KEY"
#define APP_SECRET    "YOUR_APP_SECRET"
#define device_ID_1   "DEVICE_ID_1"
#define device_ID_2   "DEVICE_ID_2"
```

---

## 🧠 Button Logic

- **Short press**: toggles the relay state (ON/OFF)
- **Long press (> 3s)**: enters AP mode using `WiFiManager` for network setup

---

## 💡 WiFi Status LED

| Status         | LED Behavior         |
|----------------|----------------------|
| Disconnected   | Fast blinking        |
| Connecting     | Slow blinking        |
| Connected      | Solid ON             |

---

## 🛠️ Installation

1. Clone this repository into your development environment (Arduino IDE recommended)
2. Install the following libraries from the **Library Manager**:
   - `ESP8266WiFi`
   - `WiFiManager`
   - `SinricPro`
3. Set your `APP_KEY`, `APP_SECRET`, and `device_IDs` in the code
4. Upload the sketch to your ESP8266
5. Hold the physical button for 3 seconds to enter AP mode if needed

---

## 🔄 Execution Flow

1. Initialize pins and structures
2. Attempt to reconnect to WiFi using stored credentials
3. If it fails, the button can activate AP mode for new configuration
4. Once connected, SinricPro is initialized
5. Inside the main loop:
   - WiFi LED status is updated
   - Buttons and switches are monitored
   - SinricPro communication remains active

---

## 🛡️ Security

This code includes credentials like `APP_SECRET`. **Be sure to replace them with your real values and never share them publicly.**

---

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## 🤝 Contributions

Got ideas to improve the project? Pull requests and suggestions are welcome!

---

## 📞 Contact

Author: **Jorge Gaspar Beltre Rivera**  
Project: Smart Switch Automation with ESP8266 + SinricPro  
GitHub: [github.com/JorgeBeltre](https://github.com/JorgeBeltre)
