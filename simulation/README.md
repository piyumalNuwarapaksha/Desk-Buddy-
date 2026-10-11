# DeskBot firmware

ESP32-S3 / C3 + 1.3" SH1106 OLED, controlled from a phone over WiFi (HTTP).

| File | What it is |
|---|---|
| `sketch.ino` | Main firmware: WiFi, web server, Serial, command parser |
| `face.h` | Face engine: eyes, 8 moods, blinking, gaze, speech text |
| `web_ui.h` | The phone control page the ESP serves at `/` |
| `diagram.json`, `libraries.txt` | Wokwi simulation circuit + library list |
| `wokwi.toml` | Only for the Wokwi VS Code extension |

## 1. Simulate on wokwi.com (quickest)

1. New project → **ESP32-S3** → replace `sketch.ino` and `diagram.json` with these files.
2. Add files `face.h`, `web_ui.h`, `libraries.txt` (the ▾ next to the file tabs → New file), paste contents.
3. Keep `#define SIMULATION 1`. Press ▶.
4. The OLED shows *Connecting → Ready! + IP*, then the face.
5. **Be the phone:** type commands into the Serial Monitor, e.g.

```
happy
look left
say Hello Dulen!
mood love
auto off
sleep
wake
```

The browser can't reach the simulated web server on wokwi.com without a paid private gateway, so Serial is the stand-in for the app; it runs the exact same `runCommand()` the HTTP endpoint uses.

## 2. Simulate the phone page too (Wokwi for VS Code)

Put the files in a folder named `sketch`, build for ESP32S3 Dev Module, start the simulator with `wokwi.toml` and open **http://localhost:8180** on your PC — that's the real phone UI talking to the simulated ESP.

## 3. Real hardware

1. Arduino IDE → ESP32 board package (Espressif) → board *ESP32S3 Dev Module* (or *ESP32C3 Dev Module*). Library Manager → **U8g2**.
2. In `sketch.ino` set `#define SIMULATION 0` and your WiFi name/password.
3. Wire the OLED: **VCC→3V3, GND→GND, SDA→GPIO8, SCL→GPIO9**.
4. Flash. The screen shows the IP — open it on your phone (same WiFi), or `http://deskbot.local`.
5. No router? After 15 s it falls back to its own network: join **DeskBot** / `deskbot123`, open **192.168.4.1**.

## HTTP API (for your own app later)

| Request | Reply |
|---|---|
| `GET /cmd?c=mood%20happy` | `{"ok":true,"msg":"mood happy","state":{...}}` |
| `GET /state` | `{"mood":"happy","auto":true,"ip":"...","ap":false}` |

CORS is open, so a Flutter/React/Android app can call it directly.
