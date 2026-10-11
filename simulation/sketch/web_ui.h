// web_ui.h — phone control page served by the ESP at http://<ip>/
// Every button just calls GET /cmd?c=<command>, the same commands the Serial monitor accepts.
#pragma once

static const char INDEX_HTML[] = R"rawliteral(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>DeskBot</title>
<style>
:root{--bg:#0e1116;--card:#181d25;--fg:#e8ecf2;--mut:#8a94a6;--acc:#4fd1c5}
*{box-sizing:border-box}body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--fg);padding:16px}
h1{font-size:20px;margin:0 0 4px}#st{color:var(--mut);font-size:13px;margin-bottom:14px}
.card{background:var(--card);border-radius:14px;padding:14px;margin-bottom:12px}
.card h2{font-size:13px;color:var(--mut);margin:0 0 10px;font-weight:600;text-transform:uppercase;letter-spacing:.06em}
.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
.pad{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:240px;margin:auto}
button{background:#232a35;color:var(--fg);border:0;border-radius:10px;padding:12px 4px;font-size:15px}
button:active{background:var(--acc);color:#000}button.on{outline:2px solid var(--acc)}
.row{display:flex;gap:8px}input{flex:1;background:#232a35;color:var(--fg);border:0;border-radius:10px;padding:12px;font-size:15px}
</style></head><body>
<h1>DeskBot</h1><div id="st">connecting…</div>
<div class="card"><h2>Mood</h2><div class="grid" id="moods"></div></div>
<div class="card"><h2>Look</h2><div class="pad">
<span></span><button onclick="c('look up')">▲</button><span></span>
<button onclick="c('look left')">◀</button><button onclick="c('look center')">●</button><button onclick="c('look right')">▶</button>
<span></span><button onclick="c('look down')">▼</button><span></span></div></div>
<div class="card"><h2>Say</h2><div class="row"><input id="t" maxlength="120" placeholder="Hello!">
<button style="padding:0 18px" onclick="say()">Send</button></div></div>
<div class="card"><div class="row"><button style="flex:1" onclick="c('blink')">Blink</button>
<button style="flex:1" id="au" onclick="c(auto?'auto off':'auto on')">Auto</button></div></div>
<script>
const M={neutral:'😐',happy:'😊',sad:'😢',angry:'😠',surprised:'😮',tired:'😩',love:'😍',sleep:'😴'};
let auto=true,mood='';
const g=document.getElementById('moods');
for(const k in M){const b=document.createElement('button');b.id='m_'+k;b.textContent=M[k];b.title=k;b.onclick=()=>c('mood '+k);g.appendChild(b)}
async function c(cmd){try{const r=await fetch('/cmd?c='+encodeURIComponent(cmd));const j=await r.json();show(j)}catch(e){st.textContent='offline'}}
function say(){const v=t.value.trim();if(v){c('say '+v);t.value=''}}
t.addEventListener('keydown',e=>{if(e.key==='Enter')say()});
function show(j){if(!j.state)return;auto=j.state.auto;mood=j.state.mood;
st.textContent=j.state.ip+' · '+mood+(auto?' · auto':'');au.classList.toggle('on',auto);
for(const k in M)document.getElementById('m_'+k).classList.toggle('on',k===mood)}
fetch('/state').then(r=>r.json()).then(s=>show({state:s})).catch(()=>{});
</script></body></html>)rawliteral";
