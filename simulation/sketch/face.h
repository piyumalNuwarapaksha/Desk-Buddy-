// face.h — DeskBot animated face engine (U8g2, 128x64 mono)
// Pure drawing + animation; no WiFi/Serial here, so it can be unit-rendered on a PC.
#pragma once
#include <math.h>
#include <string.h>

enum Mood { NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED, TIRED, LOVE, SLEEP, MOOD_COUNT };
static const char* MOOD_NAMES[MOOD_COUNT] = {
  "neutral", "happy", "sad", "angry", "surprised", "tired", "love", "sleep"};

// First-order easing: each frame v moves a fraction k of the way to target t.
struct Anim {
  float v, t;
  void set(float x)  { t = x; }
  void snap(float x) { v = t = x; }
  void step(float k) { v += (t - v) * k; }
};

class Face {
public:
  explicit Face(U8G2 &disp) : d(disp) {}

  bool autoMode = true;          // idle blinking + wandering gaze
  Mood mood = NEUTRAL;

  void begin() { setMood(NEUTRAL); nextBlink = millis() + 1500; nextLook = millis() + 3000; }

  void setMood(Mood m) {
    mood = m;
    float w = 36, h = 36, hp = 0, sd = 0, an = 0, ti = 0;
    switch (m) {
      case HAPPY:     h = 34; hp = 1;          break;
      case SAD:       w = 34; h = 30; sd = 1;  break;
      case ANGRY:     h = 32; an = 1;          break;
      case SURPRISED: w = 40; h = 44;          break;
      case TIRED:     ti = 1;                  break;
      case SLEEP:     h = 3; lookX.set(0); lookY.set(0); break;
      default: break;
    }
    eyeW.set(w); eyeH.set(h);
    lidHappy.set(hp); lidSad.set(sd); lidAngry.set(an); lidTired.set(ti);
  }

  // dx in [-16,16], dy in [-8,8]; holdMs pauses auto-gaze so the user command is visible
  void look(float dx, float dy, unsigned long holdMs = 4000) {
    lookX.set(clampf(dx, -16, 16)); lookY.set(clampf(dy, -8, 8));
    lookHoldUntil = millis() + holdMs;
  }

  void blink() { if (mood == SLEEP) return; blinkA.set(0.05f); blinkEnd = millis() + 110; }

  void say(const char *text) {
    strncpy(sayText, text, sizeof(sayText) - 1); sayText[sizeof(sayText) - 1] = 0;
    d.setFont(u8g2_font_6x10_tr);
    int tw = d.getStrWidth(sayText);
    sayStart = millis();
    sayEnd = sayStart + (tw > 124 ? (unsigned long)(tw + 128) * SCROLL_MS_PER_PX + 300 : 3500);
    sayActive = true;
  }

  // Call every frame (~50 fps): advances animation state, then renders.
  void update() {
    unsigned long now = millis();

    if (blinkEnd && now > blinkEnd) { blinkA.set(1); blinkEnd = 0; }
    if (sayActive && now > sayEnd) sayActive = false;

    if (autoMode && mood != SLEEP) {
      if (now > nextBlink) { blink(); nextBlink = now + random(2000, 6000); }
      if (now > nextLook && now > lookHoldUntil) {
        switch (random(0, 10)) {
          case 5:  lookX.set(-14); lookY.set(0);  break;
          case 6:  lookX.set(14);  lookY.set(0);  break;
          case 7:  lookX.set(0);   lookY.set(-6); break;
          case 8:  lookX.set(0);   lookY.set(6);  break;
          case 9:  lookX.set(-12); lookY.set(-5); break;
          default: lookX.set(0);   lookY.set(0);  break;   // 50 % back to centre
        }
        nextLook = now + random(2500, 7000);
      }
    }

    // Eyes move up to make room for the speech line.
    eyeCY.set((sayActive ? 24 : 32) + (mood == SAD ? 3 : 0));

    const float k = 0.25f;
    eyeW.step(k); eyeH.step(k); eyeCY.step(k); lookX.step(k); lookY.step(k);
    lidHappy.step(k); lidSad.step(k); lidAngry.step(k); lidTired.step(k);
    blinkA.step(0.55f);

    draw();
  }

  // Full-screen two-line status (boot / WiFi info)
  void status(const char *l1, const char *l2) {
    d.clearBuffer();
    d.setFont(u8g2_font_7x13B_tr);
    d.drawStr((128 - d.getStrWidth(l1)) / 2, 28, l1);
    d.setFont(u8g2_font_6x10_tr);
    d.drawStr((128 - d.getStrWidth(l2)) / 2, 46, l2);
    d.sendBuffer();
  }

private:
  U8G2 &d;
  static const int SCROLL_MS_PER_PX = 25;

  Anim eyeW{36, 36}, eyeH{36, 36}, eyeCY{32, 32}, lookX{0, 0}, lookY{0, 0};
  Anim lidHappy{0, 0}, lidSad{0, 0}, lidAngry{0, 0}, lidTired{0, 0}, blinkA{1, 1};

  unsigned long blinkEnd = 0, nextBlink = 0, nextLook = 0, lookHoldUntil = 0;
  char sayText[128] = "";
  unsigned long sayStart = 0, sayEnd = 0;
  bool sayActive = false;

  static float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
  static int clampi(int x, int lo, int hi) { return x < lo ? lo : (x > hi ? hi : x); }

  void draw() {
    d.clearBuffer();
    float w = eyeW.v;
    float h = eyeH.v * blinkA.v; if (h < 2) h = 2;
    const float gap = 12;
    float cxL = 64 - gap / 2 - w / 2 + lookX.v;
    float cxR = 64 + gap / 2 + w / 2 + lookX.v;
    float cy  = eyeCY.v + lookY.v;
    // Perspective: the eye farther from the gaze direction shrinks slightly.
    float hL = h * (1 - (lookX.v > 0 ?  lookX.v : 0) / 80.0f);
    float hR = h * (1 - (lookX.v < 0 ? -lookX.v : 0) / 80.0f);

    if (mood == LOVE) {
      int s = 13 + (int)(2 * sinf(millis() / 180.0f));   // pulsing hearts
      drawHeart((int)cxL, (int)cy, s);
      drawHeart((int)cxR, (int)cy, s);
    } else {
      drawEye(cxL, cy, w, hL, true);
      drawEye(cxR, cy, w, hR, false);
    }
    if (mood == SLEEP) drawZzz();
    if (sayActive) drawSay();
    d.sendBuffer();
  }

  void drawEye(float cx, float cy, float w, float h, bool left) {
    int iw = (int)(w + 0.5f); if (iw < 4) iw = 4;
    int ih = (int)(h + 0.5f); if (ih < 2) ih = 2;
    int x = clampi((int)(cx - iw / 2.0f + 0.5f), 0, 128 - iw);
    int y = clampi((int)(cy - ih / 2.0f + 0.5f), 0, 64 - ih);
    int r = (iw < ih ? iw : ih) / 2 - 1; if (r > 8) r = 8;

    d.setDrawColor(1);
    if (r >= 1) d.drawRBox(x, y, iw, ih, r); else d.drawBox(x, y, iw, ih);
    if (ih <= 4) return;                      // closed eye: no eyelids

    // Eyelids are drawn in black (color 0) over the white eye.
    d.setDrawColor(0);
    int xL = x, xR = x + iw - 1, yT = y, yB = y + ih - 1;

    if (lidTired.v > 0.02f)                   // flat drooping upper lid
      d.drawBox(x, y, iw, (int)(ih * 0.5f * lidTired.v) + 1);

    if (lidAngry.v > 0.02f) {                 // inner corners pulled down
      int dd = (int)(ih * 0.6f * lidAngry.v);
      if (left) d.drawTriangle(xL, yT, xR + 1, yT, xR + 1, yT + dd);
      else      d.drawTriangle(xL, yT, xR + 1, yT, xL, yT + dd);
    }
    if (lidSad.v > 0.02f) {                   // outer corners pulled down
      int dd = (int)(ih * 0.6f * lidSad.v);
      if (left) d.drawTriangle(xL, yT, xR + 1, yT, xL, yT + dd);
      else      d.drawTriangle(xL, yT, xR + 1, yT, xR + 1, yT + dd);
    }
    if (lidHappy.v > 0.02f) {                 // curved "cheek" cuts the bottom → ^ ^
      int ry = ih / 2 + 2, rx = iw / 2 + 4;
      float top = yB + 1 - lidHappy.v * ih * 0.45f;
      d.drawFilledEllipse(x + iw / 2, (int)(top + ry), rx, ry, U8G2_DRAW_ALL);
    }
    d.setDrawColor(1);
  }

  void drawHeart(int cx, int cy, int s) {
    int r = s / 2;
    d.drawDisc(cx - r, cy - r / 2, r);
    d.drawDisc(cx + r, cy - r / 2, r);
    d.drawTriangle(cx - 2 * r, cy - r / 2 + 1, cx + 2 * r, cy - r / 2 + 1, cx, cy + 2 * r - 2);
  }

  void drawZzz() {
    d.setFont(u8g2_font_6x10_tr);
    unsigned long t = millis();
    for (int i = 0; i < 3; i++) {
      int ph = (int)((t / 40 + i * 25) % 75);       // three z's rising, staggered
      d.drawStr(100 + ph / 5, 30 - ph / 3, "z");
    }
  }

  void drawSay() {
    d.setFont(u8g2_font_6x10_tr);
    d.setDrawColor(1);
    int tw = d.getStrWidth(sayText);
    int x = tw <= 124 ? (128 - tw) / 2
                      : 128 - (int)((millis() - sayStart) / SCROLL_MS_PER_PX);
    d.drawStr(x, 62, sayText);   // negative x wraps in 16-bit mode; U8g2 clips it correctly
  }
};
