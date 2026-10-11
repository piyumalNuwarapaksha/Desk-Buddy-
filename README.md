# 🤖 Desk Buddy Bot

An embedded desktop companion built around the **ESP32-C3 Super Mini**, combining an OLED display, capacitive touch interaction, sound feedback, and addressable RGB lighting in a compact desktop device.

## ✨ Features

- **OLED Display:** SSD1306 OLED driven over I²C for visual output and status information.
- **Capacitive Touch Controls:** Two TTP223 touch sensors for interactive input.
- **Audio Feedback:** Passive buzzer controlled using PWM to generate tones and sound effects.
- **RGB Lighting:** WS2812B individually addressable LEDs for customizable lighting effects.
- **Movement Detection:** Optional digital vibration sensor for detecting physical movement.
- **Operating Modes:** Physical switch connected to GPIO0 for switching between software-controlled modes.
- **USB-C Power:** Powered through a 5 V USB-C input, with 3.3 V GPIO logic.

## 🛠️ Hardware Components

| Component | Purpose |
|---|---|
| ESP32-C3 Super Mini | Main microcontroller |
| SSD1306 OLED | Visual output |
| 2 × TTP223 | Capacitive touch input |
| Passive buzzer | Audio feedback |
| WS2812B LEDs | RGB lighting effects |
| Digital vibration sensor (optional) | Movement detection |
| USB-C 5 V input | Power supply |

## ⚙️ System Architecture

The ESP32-C3 coordinates the connected peripherals through their respective interfaces:

- **I²C:** OLED display communication.
- **Digital GPIO:** Touch sensors, vibration sensor, and mode switch.
- **PWM:** Buzzer tone generation.
- **Single-wire serial interface:** WS2812B LED control.

The firmware initializes the peripherals, reads user inputs, updates the display, generates audio feedback, and controls RGB lighting according to the active operating mode.

## 🔌 Power and Logic Considerations

- The system receives 5 V through USB-C.
- ESP32-C3 GPIO signals operate at 3.3 V logic and must not be exposed directly to 5 V signals.
- The power supply must accommodate the combined current requirements of the ESP32-C3, OLED, buzzer, and RGB LEDs.
- WS2812B power and data connections must be designed according to the LED specifications.
- Ensure that all connected modules share a common ground.

## 🎯 Project Objective

The objective of Desk Buddy Bot is to develop a compact embedded desktop companion that demonstrates microcontroller-based peripheral integration, human–machine interaction, display interfacing, audio generation, and programmable RGB lighting.

## 💻 Development Platform

- **Microcontroller:** ESP32-C3 Super Mini
- **IDE:** Arduino IDE
- **Programming Language:** C/C++ (Arduino framework)
- **Communication:** I²C and digital GPIO
- **Power Input:** 5 V USB-C

## 🚀 Future Improvements

- Customizable OLED animations and expressions.
- Multiple touch-triggered interactions.
- Additional buzzer sound patterns.
- Configurable RGB lighting effects.
- Temperature and humidity monitoring.
- Improved power management and operating modes.

---

**Desk Buddy Bot** is an embedded systems project exploring the integration of hardware peripherals, real-time input handling, and interactive output in a compact desktop companion.
