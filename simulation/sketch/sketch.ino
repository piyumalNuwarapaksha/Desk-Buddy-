/*
  DeskBot firmware
  ESP32-S3 / ESP32-C3  +  1.3" SH1106 OLED (I2C)  +  WiFi HTTP control from a phone

  Control paths (both run the same runCommand()):
    • Phone:  open http://<ip>/  (or http://deskbot.local/)  → buttons call GET /cmd?c=<command>
              Your own app can call the same endpoint; replies are JSON.
    • Serial: type a command in the Serial Monitor (this is how you "be the phone" in Wokwi)

  Commands:
    mood <neutral|happy|sad|angry|surprised|tired|love|sleep>   (the mood name alone also works)
    look <left|right|up|down|center>
    blink
    say <text>          (scrolls if longer than the screen)
    auto <on|off>       (idle blinking + wandering gaze)
    wake                (same as mood neutral)
    ip | help
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "face.h"
#include "web_ui.h"

// ---------------- Build configuration ----------------
#define SIMULATION 1   // 1 = Wokwi (SSD1306 part + Wokwi-GUEST WiFi), 0 = real hardware (SH1106)

#define PIN_SDA 8      // default I2C pins on both ESP32-S3 DevKitC and ESP32-C3 DevKitM
#define PIN_SCL 9

#if SIMULATION
// Wokwi has no SH1106 part; the SSD1306 is the same 128x64 I2C panel, so drawing code is identical.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_SCL, PIN_SDA);
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASS = "";
#else
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_SCL, PIN_SDA);
const char *WIFI_SSID = "YOUR_WIFI_NAME";
const char *WIFI_PASS = "YOUR_WIFI_PASSWORD";
#endif

// If the router can't be reached, the bot hosts its own network: join "DeskBot", open 192.168.4.1
const char *AP_SSID = "DeskBot";
const char *AP_PASS = "deskbot123";      // min 8 characters

const uint32_t FRAME_MS = 20;            // 50 fps

// ---------------- Globals ----------------
Face face(u8g2);
WebServer server(80);
String ipStr = "-";
bool apMode = false;

// ---------------- Command handling ----------------
String stateJson() {
  return String("{\"mood\":\"") + MOOD_NAMES[face.mood] + "\",\"auto\":" +
         (face.autoMode ? "true" : "false") + ",\"ip\":\"" + ipStr + "\",\"ap\":" +
         (apMode ? "true" : "false") + "}";
}

// Returns a short human-readable result; same function for HTTP and Serial.
// cmdOk is set false when the command was rejected.
bool cmdOk = true;
String fail(const String &msg) { cmdOk = false; return msg; }

String runCommand(String line) {
  cmdOk = true;
  line.trim();
  if (line.length() == 0) return fail("empty command");

  int sp = line.indexOf(' ');
  String cmd = sp < 0 ? line : line.substring(0, sp);
  String arg = sp < 0 ? "" : line.substring(sp + 1);
  arg.trim();
  cmd.toLowerCase();

  if (cmd == "mood") { cmd = arg; cmd.toLowerCase(); }   // "mood happy" → "happy"
  if (cmd == "wake") cmd = "neutral";

  for (int i = 0; i < MOOD_COUNT; i++) {
    if (cmd == MOOD_NAMES[i]) { face.setMood((Mood)i); return "mood " + cmd; }
  }

  if (cmd == "look") {
    arg.toLowerCase();
    if      (arg == "left")   face.look(-16, 0);
    else if (arg == "right")  face.look(16, 0);
    else if (arg == "up")     face.look(0, -8);
    else if (arg == "down")   face.look(0, 8);
    else if (arg == "center" || arg == "centre" || arg == "") face.look(0, 0);
    else return fail("look: use left/right/up/down/center");
    return "look " + arg;
  }
  if (cmd == "blink") { face.blink(); return "blink"; }
  if (cmd == "say") {
    if (arg.length() == 0) return fail("say: nothing to say");
    face.say(arg.c_str());
    return "say " + arg;
  }
  if (cmd == "auto") {
    arg.toLowerCase();
    face.autoMode = (arg != "off" && arg != "0");
    return String("auto ") + (face.autoMode ? "on" : "off");
  }
  if (cmd == "ip")   return ipStr;
  if (cmd == "help") return "mood <name> | look <dir> | blink | say <text> | auto on/off | wake | ip";
  return fail("unknown command: " + cmd);
}

// ---------------- HTTP ----------------
void sendJson(int code, const String &body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");   // lets a custom app/web page call us
  server.send(code, "application/json", body);
}

void setupServer() {
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", INDEX_HTML); });

  server.on("/cmd", HTTP_GET, []() {
    if (!server.hasArg("c")) { sendJson(400, "{\"ok\":false,\"msg\":\"missing ?c=\"}"); return; }
    String res = runCommand(server.arg("c"));
    Serial.println("[http] " + server.arg("c") + " -> " + res);
    bool ok = cmdOk;
    res.replace("\\", "/");   // keep the JSON valid
    res.replace("\"", "'");
    sendJson(ok ? 200 : 400, String("{\"ok\":") + (ok ? "true" : "false") +
                             ",\"msg\":\"" + res + "\",\"state\":" + stateJson() + "}");
  });

  server.on("/state", HTTP_GET, []() { sendJson(200, stateJson()); });
  server.onNotFound([]() { sendJson(404, "{\"ok\":false,\"msg\":\"not found\"}"); });
  server.begin();
}

// ---------------- WiFi ----------------
void setupNetwork() {
  WiFi.mode(WIFI_STA);
#if SIMULATION
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);   // Wokwi-GUEST is on channel 6; skips the scan
#else
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif

  char dots[8] = "";
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    int n = ((millis() - t0) / 400) % 4;
    for (int i = 0; i < 3; i++) dots[i] = i < n ? '.' : ' ';
    dots[3] = 0;
    face.status("Connecting", (String(WIFI_SSID) + dots).c_str());
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    ipStr = WiFi.localIP().toString();
  } else {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    apMode = true;
    ipStr = WiFi.softAPIP().toString();
  }
  if (MDNS.begin("deskbot")) MDNS.addService("http", "tcp", 80);

  Serial.printf("[net] %s  IP: %s\n", apMode ? "AP mode (join 'DeskBot')" : "connected", ipStr.c_str());
  face.status(apMode ? "AP: DeskBot" : "Ready!", ipStr.c_str());
  delay(2500);   // leave the IP on screen long enough to read it
}

// ---------------- Serial ----------------
void pollSerial() {
  static String buf;
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\n' || ch == '\r') {
      if (buf.length()) { Serial.println("> " + buf + "  ->  " + runCommand(buf)); buf = ""; }
    } else if (buf.length() < 160) {
      buf += ch;
    }
  }
}

// ---------------- Arduino entry points ----------------
void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());

  u8g2.begin();
  u8g2.setBusClock(400000);
  face.status("DeskBot", "booting...");

  setupNetwork();
  setupServer();
  face.begin();

  Serial.println("Type 'help' for commands.");
}

void loop() {
  server.handleClient();
  pollSerial();

  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last >= FRAME_MS) {
    last = now;
    face.update();
  }
}
