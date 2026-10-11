# DeskBot web interface

`index.html` is the phone control page the ESP32 serves at `http://<bot-ip>/`.

It is a standalone copy of the page embedded in [`../simulation/sketch/web_ui.h`](../simulation/sketch/web_ui.h) — the firmware compiles `web_ui.h`, so **edit that file** (or copy changes back into it) for them to reach the bot.

Every button calls `GET /cmd?c=<command>` on the bot; opening `index.html` directly in a browser shows the layout but stays "offline" since there's no bot to talk to. To try it live, run the simulation with the Wokwi VS Code extension and open http://localhost:8180 (see [`../simulation/README.md`](../simulation/README.md)).

> Work in progress — not finalized.
