
# 🔌 ESP8266 WiFi Smart Switch con SinricPro y Control Físico

Este proyecto permite controlar dos dispositivos eléctricos mediante un módulo **ESP8266**. Integra conectividad con **SinricPro** para control remoto por voz (Alexa, Google Assistant) o desde la app, junto con control físico mediante botones y switches. El sistema incluye gestión automática de WiFi usando **WiFiManager**, soporte para modo AP, y un LED indicador de estado de red.

---

## 🚀 Características principales

- ✅ Control de **2 relés** vía SinricPro (Alexa/Google Assistant)
- ✅ Botones físicos y switches para control manual
- ✅ Pulsación larga para entrar en **modo configuración AP**
- ✅ Estado de WiFi reflejado por LED (rápido, lento, fijo)
- ✅ Restauración automática de estados de los relés al reinicio
- ✅ Código modular y escalable usando `std::map`
- ✅ Compatibilidad con múltiples dispositivos SinricPro

---

## 📦 Hardware necesario

- ESP8266 (NodeMCU o Wemos D1 mini recomendado)
- 2x Relés de 5V
- 2x Botones físicos
- 2x Switches
- 1x LED (indicador WiFi)
- Resistencias (para pull-up si no usas INPUT_PULLUP)
- Fuente de alimentación adecuada (5V/3.3V)

---

## 🧠 Estructura del sistema

| Componente      | Función                                 |
|------------------|------------------------------------------|
| `SinricProSwitch` | Interfaz con SinricPro API               |
| `WiFiManager`     | Gestión de conexión y modo configuración |
| `std::map`        | Asociación de pines y dispositivos        |
| `Relé`            | Salida controlada (ON/OFF)              |
| `Botón físico`    | Control directo de relé + modo AP        |
| `Switch`          | Control directo alternativo             |
| `LED`             | Estado de red WiFi                      |

---

## 🔧 Pines configurados

```cpp
RelayPin1     = GPIO13
RelayPin2     = GPIO14
BUTTON_PIN1   = GPIO0   // también puede actuar como botón de modo AP
BUTTON_PIN2   = GPIO8
SWITCH_PIN1   = GPIO6
SWITCH_PIN2   = GPIO7
wifiLed       = GPIO2   // Estado de WiFi
```

> ⚠️ Algunos pines pueden variar según el módulo ESP8266 usado. Asegúrate de verificar compatibilidad con tu hardware.

---

## ☁️ Integración con SinricPro

Debes crear un proyecto en [SinricPro](https://portal.sinric.pro/) y obtener:

- APP KEY
- APP SECRET
- Device IDs

Ejemplo:

```cpp
#define APP_KEY       "TU_APP_KEY"
#define APP_SECRET    "TU_APP_SECRET"
#define device_ID_1   "ID_DISPOSITIVO_1"
#define device_ID_2   "ID_DISPOSITIVO_2"
```

---

## 🧠 Lógica de botones

- **Pulsación corta**: cambia el estado del relé (ON/OFF)
- **Pulsación larga (> 3s)**: entra al modo AP con `WiFiManager` para configurar una nueva red

---

## 💡 LED de estado WiFi

| Estado           | Comportamiento del LED |
|------------------|------------------------|
| Sin conexión     | Parpadeo rápido        |
| Conectando       | Parpadeo lento         |
| Conectado        | Encendido fijo         |

---

## 🛠️ Instalación

1. Clona este repositorio en tu entorno de desarrollo (Arduino IDE recomendado)
2. Instala las siguientes bibliotecas desde el **Gestor de Bibliotecas**:
   - `ESP8266WiFi`
   - `WiFiManager`
   - `SinricPro`
3. Configura tu `APP_KEY`, `APP_SECRET` y `device_IDs` en el código
4. Carga el programa en tu ESP8266
5. Mantén presionado el botón físico por 3 segundos si deseas ingresar al modo AP

---

## 🔄 Flujo de ejecución

1. Inicialización de pines y estructuras
2. Intento de conexión WiFi usando credenciales previas
3. Si falla, el botón puede activar el modo AP para nueva configuración
4. Una vez conectado, se inicializa SinricPro
5. En el bucle principal:
   - Se actualiza el estado del LED WiFi
   - Se monitorean botones y switches
   - Se mantiene activa la comunicación con SinricPro

---

## 🛡️ Seguridad

Este código expone credenciales como `APP_SECRET`. **Reemplázalas por valores reales y evita compartirlas públicamente.**

---

## 📄 Licencia

Este proyecto está bajo la licencia MIT. Consulta el archivo [LICENSE](LICENSE) para más información.

---

## 🤝 Contribuciones

¿Tienes ideas para mejorar el proyecto? ¡Tus pull requests y sugerencias son bienvenidas!

---

## 📞 Contacto

Autor: **Jorge Gaspar Beltre Rivera**  
Proyecto: Automatización de interruptores con ESP8266 + SinricPro  
GitHub: [github.com/JorgeBeltre](https://github.com/JorgeBeltre)
