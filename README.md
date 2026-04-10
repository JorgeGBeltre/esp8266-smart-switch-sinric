# ESP8266 WiFi Smart Switch with SinricPro and Physical Control

This project allows you to control two electrical devices using an **ESP8266** module. It integrates **SinricPro** connectivity for remote control via voice (Alexa, Google Assistant) or the app, along with physical control using buttons and switches. The system includes automatic WiFi management using **WiFiManager**, AP mode support, and an LED indicator for network status.

---

## Table of Contents

1. [Main Features](#main-features)
2. [Required Hardware](#required-hardware)
3. [System Architecture](#system-architecture)
4. [Configured Pins](#configured-pins)
5. [SinricPro Integration](#sinricpro-integration)
6. [Button Logic](#button-logic)
7. [AP Mode](#ap-mode)
8. [WiFi Status LED](#wifi-status-led)
9. [Reconnection Behavior](#reconnection-behavior)
10. [Getting Started](#getting-started)
11. [Installation](#installation)
11. [Installation](#installation)
12. [Execution Flow](#execution-flow)
13. [Debug Mode](#debug-mode)
14. [Security](#security)
15. [License](#license)
16. [Contributions](#contributions)
17. [Contact](#contact)

---

## Main Features

- Control **2 relays** via SinricPro (Alexa/Google Assistant)
- Physical buttons and switches for manual control
- Long press activates **WiFi configuration (AP) mode** with a unique SSID based on chip ID (`VESIS-<ChipID>`)
- WiFi status is indicated by LED (fast, slow, solid)
- Automatic relay state restoration on restart via SinricPro
- Automatic WiFi reconnection every 10 seconds when connection is lost
- Automatic SinricPro reconnection every 30 seconds when disconnected
- Modular and scalable code using `std::map`
- Debug mode via `ENABLE_DEBUG` flag with Serial output at 115200 baud

---

## Required Hardware

- ESP8266 (NodeMCU or Wemos D1 mini recommended)
- 2x 5V Relays
- 2x Physical buttons
- 2x Switches
- 1x LED (WiFi indicator)
- Pull-up resistors (if not using `INPUT_PULLUP`)
- Suitable power supply (5V/3.3V)

---

## System Architecture

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

## Configured Pins

```cpp
RelayPin1     = GPIO5    // D1
RelayPin2     = GPIO4    // D2
BUTTON_PIN1   = GPIO0    // D3 - also acts as AP mode button
BUTTON_PIN2   = GPIO14   // D5
SWITCH_PIN1   = GPIO12   // D6
SWITCH_PIN2   = GPIO13   // D7
wifiLed       = GPIO2    // D4 - WiFi status LED
```

> Some pins may vary depending on your ESP8266 board. Make sure to verify compatibility with your hardware.

---

## SinricPro Integration

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

## Button Logic

- **Short press** (> 50 ms debounce): toggles the relay state (ON/OFF) and reports the new state to SinricPro if connected
- **Long press (> 3s)**: enters AP mode using `WiFiManager` for network setup

---

## AP Mode

When AP mode is triggered (either on first boot with no saved credentials or via long press), the device creates a hotspot with:

- **SSID**: `VESIS-<ChipID>` (unique per device)
- **Password**: `12345678`
- **Portal timeout**: 180 seconds

After configuration is saved, the ESP8266 restarts automatically.

---

## WiFi Status LED

| Status         | LED Behavior               |
|----------------|----------------------------|
| Disconnected   | Fast blinking (100 ms)     |
| Connecting     | Slow blinking (400 ms)     |
| Connected      | Solid ON                   |

---

## Reconnection Behavior

| Service     | Retry Interval |
|-------------|----------------|
| WiFi        | Every 10 seconds |
| SinricPro   | Every 30 seconds |

---

## Getting Started

### Prerequisites

Before you begin, make sure you have the following installed:

- [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x)
- ESP8266 board support package — add this URL in Arduino IDE under **File > Preferences > Additional Boards Manager URLs**:
  ```
  http://arduino.esp8266.com/stable/package_esp8266com_index.json
  ```
  Then go to **Tools > Board > Boards Manager**, search for `esp8266`, and install it.

### Clone the Repository

```bash
git clone https://github.com/JorgeGBeltre/Relay_Wifi_ESP8266_Switch.git
cd Relay_Wifi_ESP8266_Switch
```

### Install Required Libraries

Open Arduino IDE and go to **Sketch > Include Library > Manage Libraries**, then search and install:

| Library       | Author              |
|---------------|---------------------|
| `WiFiManager` | tzapu / tablatronics |
| `SinricPro`   | Sinric Pro          |

`ESP8266WiFi` is included automatically with the ESP8266 board package.

### Configure Credentials

Open `Relay_Wifi_ESP8266_Switch.ino` and replace the placeholder values:

```cpp
#define APP_KEY       "YOUR_APP_KEY"
#define APP_SECRET    "YOUR_APP_SECRET"
#define device_ID_1   "DEVICE_ID_1"
#define device_ID_2   "DEVICE_ID_2"
```

Obtain these values from your project at [portal.sinric.pro](https://portal.sinric.pro/).

---

## Installation

1. Clone this repository into your development environment (Arduino IDE recommended)
2. Install the following libraries from the **Library Manager**:
   - `ESP8266WiFi`
   - `WiFiManager`
   - `SinricPro`
3. Set your `APP_KEY`, `APP_SECRET`, and `device_IDs` in the code
4. Upload the sketch to your ESP8266
5. On first boot with no saved credentials, the device will automatically enter AP mode
6. Hold any physical button for 3 seconds to re-enter AP mode if needed

---

## Execution Flow

1. Initialize pins, relays, and buttons
2. Check for saved WiFi credentials
   - If found: attempt connection with a 10-second timeout
   - If not found: automatically activate AP mode for initial configuration
3. Once connected, SinricPro is initialized and device states are restored
4. Inside the main loop:
   - WiFi LED status is updated
   - WiFi connection is monitored and reconnected if lost
   - SinricPro reconnection is handled automatically
   - Buttons and switches are monitored
   - SinricPro communication remains active

---

## Debug Mode

Debug output is enabled by default via the `ENABLE_DEBUG` flag at the top of the file. Serial output runs at **115200 baud** and logs relay changes, button presses, WiFi status, and SinricPro events. To disable debug output, comment out or remove the `#define ENABLE_DEBUG` line.

---

## Security

This code includes credentials like `APP_SECRET` and a hardcoded AP password (`12345678`). **Be sure to replace them with your real values and never share them publicly.**

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## Contributions

Got ideas to improve the project? Pull requests and suggestions are welcome!

---

## Contact

Author: **Jorge Gaspar Beltre Rivera**  
Project: Smart Switch Automation with ESP8266 + SinricPro  
GitHub: [github.com/JorgeGBeltre](https://github.com/JorgeGBeltre)