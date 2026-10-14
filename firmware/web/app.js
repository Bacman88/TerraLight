'use strict';
// TerraLight panel — vanilla JS, no external resources (must work in AP mode).

const $ = (id) => document.getElementById(id);

// ── i18n ────────────────────────────────────────────────────
const T = {
  pl: {
    setupTitle: 'Połącz z domowym Wi-Fi',
    setupHint: 'Wybierz sieć 2,4 GHz i podaj hasło. Światło działa dalej podczas konfiguracji.',
    scanning: 'Szukam sieci…', rescan: 'Odśwież', ssidManual: 'Nazwa sieci (wybierz z listy albo wpisz)', password: 'Hasło',
    showPass: 'Pokaż', hidePass: 'Ukryj', reasonCode: 'kod routera',
    whyPass: 'najczęściej złe hasło albo za słaby sygnał', whyNoAp: 'płytka nie widzi tej sieci (5 GHz? literówka w nazwie?)',
    whySec: 'niezgodne zabezpieczenia — ustaw w routerze WPA2 albo WPA2/WPA3', whyWeak: 'za słaby sygnał — przybliż płytkę do routera',
    stepWifi: 'Wi-Fi', stepLed: 'LED i zasilacz',
    back: 'Wstecz', next: 'Dalej', skip: 'Pomiń', finish: 'Zakończ',
    skipHint: 'Bez Wi-Fi światło działa, ale zegar nie ustawi się z internetu. Sieć możesz dodać później w ustawieniach.',
    wizCount: 'Ile diod LED?', wizSupply: 'Jaki zasilacz (5 V)?',
    wizEstOk: 'Pełna biel: ok. {ma} mA — zasilacz wystarczy.',
    wizEstLimit: 'Pełna biel: ok. {ma} mA — więcej niż zasilacz; jasność zostanie ograniczona automatycznie.',
    ledStepHint: 'Resztę (kolory, jasność, typ diod) ustawisz później w panelu.',
    setupFinished: 'Konfiguracja zakończona',
    connect: 'Połącz', light: 'Światło', color: 'Kolor', brightness: 'Jasność', density: 'Gęstość',
    dens1: 'Wszystkie diody', dens2: 'Co druga', dens3: 'Co trzecia', dens4: 'Co czwarta',
    module: 'Moduł LED', preset: 'Zestaw', custom: 'Własny', count: 'Liczba LED', chip: 'Układ',
    order: 'Kolejność kolorów', shape: 'Kształt', ring: 'Pierścień', arc: 'Łuk', line: 'Linia',
    maPx: 'mA / piksel', budget: 'Budżet prądu LED (mA)',
    orderHint: 'Jeśli kolory się nie zgadzają (np. zielony zamiast czerwonego), zmień kolejność kolorów.',
    save: 'Zapisz', time: 'Czas', tz: 'Strefa czasowa', timeFromPhone: 'Ustaw czas z telefonu',
    network: 'Sieć', address: 'Adres', signal: 'Sygnał', changeNet: 'Zmień sieć',
    forget: 'Zapomnij sieć i włącz tryb konfiguracji', system: 'System', maxBright: 'Maks. jasność (%)',
    softStart: 'Łagodny start (s)', hostname: 'Nazwa w sieci', profile: 'Profil', restart: 'Restart',
    defaults: 'Ustawienia fabryczne',
    // dynamic
    saved: 'Zapisano', error: 'Błąd', timeUnknown: 'Czas nieznany',
    srcNtp: 'Źródło: internet (NTP)', srcManual: 'Źródło: ustawiony z telefonu', srcNone: 'Brak czasu — połącz Wi-Fi albo ustaw z telefonu',
    wifiConnected: 'połączono', wifiConnecting: 'łączenie…', wifiWaiting: 'brak sieci — ponawiam', wifiAp: 'tryb konfiguracji',
    testing: 'Sprawdzam połączenie…', testOk: 'Połączono! Za chwilę TerraLight będzie pod adresem: ', testFail: 'Nie udało się połączyć — sprawdź hasło i sieć.',
    pickNet: 'Wybierz sieć', limited: 'ograniczono do budżetu zasilacza', ledEst: 'Pobór LED ok.',
    softStarting: 'łagodny start', confirmForget: 'Zapomnieć sieć i włączyć tryb konfiguracji?',
    confirmDefaults: 'Przywrócić ustawienia fabryczne? (Wi-Fi zostaje)', restarting: 'Restartuję…',
    restartNeeded: 'Zapisano — zmiana nazwy zadziała po restarcie', uptime: 'Czas pracy', heap: 'Wolna pamięć',
    noNets: 'Nie znaleziono sieci'
  },
  en: {
    setupTitle: 'Connect to your home Wi-Fi',
    setupHint: 'Pick a 2.4 GHz network and enter the password. The light keeps running during setup.',
    scanning: 'Scanning…', rescan: 'Rescan', ssidManual: 'Network name (pick from the list or type)', password: 'Password',
    showPass: 'Show', hidePass: 'Hide', reasonCode: 'router code',
    whyPass: 'usually a wrong password or weak signal', whyNoAp: 'the board cannot see this network (5 GHz? typo in the name?)',
    whySec: 'incompatible security — set the router to WPA2 or WPA2/WPA3', whyWeak: 'signal too weak — move the board closer to the router',
    stepWifi: 'Wi-Fi', stepLed: 'LEDs and power supply',
    back: 'Back', next: 'Next', skip: 'Skip', finish: 'Finish',
    skipHint: 'Without Wi-Fi the light works, but the clock cannot sync from the internet. You can add a network later in settings.',
    wizCount: 'How many LEDs?', wizSupply: 'Which power supply (5 V)?',
    wizEstOk: 'Full white: approx. {ma} mA — the supply is enough.',
    wizEstLimit: 'Full white: approx. {ma} mA — more than the supply; brightness will be limited automatically.',
    ledStepHint: 'Everything else (colours, brightness, LED type) can be set later in the panel.',
    setupFinished: 'Setup finished',
    connect: 'Connect', light: 'Light', color: 'Colour', brightness: 'Brightness', density: 'Density',
    dens1: 'Every pixel', dens2: 'Every 2nd', dens3: 'Every 3rd', dens4: 'Every 4th',
    module: 'LED module', preset: 'Preset', custom: 'Custom', count: 'LED count', chip: 'Chip',
    order: 'Colour order', shape: 'Shape', ring: 'Ring', arc: 'Arc', line: 'Line',
    maPx: 'mA / pixel', budget: 'LED current budget (mA)',
    orderHint: 'If colours look wrong (e.g. green instead of red), change the colour order.',
    save: 'Save', time: 'Time', tz: 'Time zone', timeFromPhone: 'Set time from this phone',
    network: 'Network', address: 'Address', signal: 'Signal', changeNet: 'Change network',
    forget: 'Forget network and start setup mode', system: 'System', maxBright: 'Max brightness (%)',
    softStart: 'Soft start (s)', hostname: 'Network name', profile: 'Profile', restart: 'Restart',
    defaults: 'Factory defaults',
    saved: 'Saved', error: 'Error', timeUnknown: 'Time unknown',
    srcNtp: 'Source: internet (NTP)', srcManual: 'Source: set from phone', srcNone: 'No time — connect Wi-Fi or set it from the phone',
    wifiConnected: 'connected', wifiConnecting: 'connecting…', wifiWaiting: 'no network — retrying', wifiAp: 'setup mode',
    testing: 'Testing connection…', testOk: 'Connected! TerraLight will soon be at: ', testFail: 'Could not connect — check the password and network.',
    pickNet: 'Pick a network', limited: 'limited to the supply budget', ledEst: 'LED current approx.',
    softStarting: 'soft start', confirmForget: 'Forget the network and start setup mode?',
    confirmDefaults: 'Restore factory defaults? (Wi-Fi is kept)', restarting: 'Restarting…',
    restartNeeded: 'Saved — the new name works after a restart', uptime: 'Uptime', heap: 'Free memory',
    noNets: 'No networks found'
  }
};
let lang = 'pl';
const t = (k) => (T[lang] && T[lang][k]) || T.pl[k] || k;

function applyI18n() {
  document.documentElement.lang = lang;
  document.querySelectorAll('[data-i18n]').forEach((el) => { el.textContent = t(el.dataset.i18n); });
  document.querySelectorAll('[data-i18n-ph]').forEach((el) => { el.placeholder = t(el.dataset.i18nPh); });
  $('langBtn').textContent = lang === 'pl' ? 'EN' : 'PL';
}

// ── Time zones (POSIX rules, kept in the browser to save ESP memory) ──
const ZONES = [
  ['Europe/Warsaw', 'CET-1CEST,M3.5.0,M10.5.0/3'],
  ['Europe/Berlin', 'CET-1CEST,M3.5.0,M10.5.0/3'],
  ['Europe/Prague', 'CET-1CEST,M3.5.0,M10.5.0/3'],
  ['Europe/London', 'GMT0BST,M3.5.0/1,M10.5.0'],
  ['Europe/Dublin', 'IST-1GMT0,M10.5.0,M3.5.0/1'],
  ['Europe/Kyiv', 'EET-2EEST,M3.5.0/3,M10.5.0/4'],
  ['Europe/Helsinki', 'EET-2EEST,M3.5.0/3,M10.5.0/4'],
  ['Europe/Lisbon', 'WET0WEST,M3.5.0/1,M10.5.0'],
  ['America/New_York', 'EST5EDT,M3.2.0,M11.1.0'],
  ['America/Chicago', 'CST6CDT,M3.2.0,M11.1.0'],
  ['America/Denver', 'MST7MDT,M3.2.0,M11.1.0'],
  ['America/Los_Angeles', 'PST8PDT,M3.2.0,M11.1.0'],
  ['Australia/Sydney', 'AEST-10AEDT,M10.1.0,M4.1.0/3'],
  ['Australia/Adelaide', 'ACST-9:30ACDT,M10.1.0,M4.1.0/3'],
  ['Australia/Brisbane', 'AEST-10'],
  ['Asia/Tokyo', 'JST-9'],
  ['UTC', 'UTC0']
];

// ── API ─────────────────────────────────────────────────────
async function api(path, body) {
  const opt = body === undefined ? {} : {
    method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body)
  };
  const r = await fetch(path, opt);
  const j = await r.json().catch(() => ({ ok: false, error: 'HTTP ' + r.status }));
  if (!r.ok || j.ok === false) throw new Error(j.error || 'HTTP ' + r.status);
  return j;
}

let toastTimer;
function toast(msg) {
  const el = $('toast');
  el.textContent = msg;
  el.classList.add('show');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.remove('show'), 2400);
}

let cfg = null;
let status = null;

// ── Config → form ───────────────────────────────────────────
function fillForm(c) {
  cfg = c;
  if (c.system && c.system.lang && c.system.lang !== lang) { lang = c.system.lang; applyI18n(); }
  $('lightOn').checked = c.light.on;
  $('lightColor').value = c.light.color;
  $('lightBright').value = c.light.brightness;
  $('brightVal').textContent = c.light.brightness + '%';
  $('lightDensity').value = String(c.light.density);

  $('modPreset').value = c.module.preset;
  $('modCount').value = c.module.count;
  $('modCount').max = c.module.maxCount;
  $('modChip').value = c.module.chip;
  $('modOrder').value = c.module.order;
  $('modShape').value = c.module.shape;
  $('modMa').value = c.module.maPerPixel;
  $('modBudget').value = c.module.budgetMa;

  const sel = $('tzSelect');
  if (!sel.options.length) {
    ZONES.forEach(([name, rule]) => sel.add(new Option(name.replace('_', ' '), name)));
  }
  sel.value = c.time.tzName;

  $('outMax').value = c.output.maxBrightness;
  $('outSoft').value = c.output.softStartSec;
  $('sysHost').value = c.system.hostname;
  $('sysProfile').value = c.profile;
}

async function saveConfig(patch, quiet) {
  try {
    const r = await api('/api/config', patch);
    fillForm(r.config);
    if (r.restartRequired) toast(t('restartNeeded'));
    else if (!quiet) toast(t('saved'));
    return true;
  } catch (e) { toast(t('error') + ': ' + e.message); return false; }
}

function saveModule(quiet) {
  return saveConfig({
    module: {
      preset: $('modPreset').value, count: +$('modCount').value, chip: $('modChip').value,
      order: $('modOrder').value, shape: $('modShape').value, maPerPixel: +$('modMa').value,
      budgetMa: +$('modBudget').value
    }
  }, quiet);
}

// Live light controls — debounced so slider drags don't flood the ESP.
let lightTimer;
function pushLight() {
  clearTimeout(lightTimer);
  lightTimer = setTimeout(() => saveConfig({
    light: {
      on: $('lightOn').checked,
      color: $('lightColor').value,
      brightness: +$('lightBright').value,
      density: +$('lightDensity').value
    }
  }, true), 150);
}

// ── Status ──────────────────────────────────────────────────
function renderStatus(s) {
  status = s;
  $('ver').textContent = 'v' + s.version + ' · ' + (s.profile === 'aqua' ? 'Aqua' : 'Terra');

  const w = s.wifi;
  const pw = $('pillWifi');
  const wifiText = { connected: t('wifiConnected'), connecting: t('wifiConnecting'), waiting: t('wifiWaiting'), ap: t('wifiAp') }[w.state] || w.state;
  pw.textContent = 'Wi-Fi: ' + wifiText;
  pw.className = 'pill ' + (w.state === 'connected' ? 'ok' : 'warn');

  const pt = $('pillTime');
  if (s.time.valid && s.time.local) {
    pt.textContent = s.time.local.slice(11, 16);
    pt.className = 'pill ok';
    $('timeNow').textContent = s.time.local;
  } else {
    pt.textContent = '--:--';
    pt.className = 'pill warn';
    $('timeNow').textContent = t('timeUnknown');
  }
  $('timeSrc').textContent = { ntp: t('srcNtp'), manual: t('srcManual') }[s.time.source] || t('srcNone');

  // Setup card in AP mode; Wi-Fi card otherwise. Move the form between them.
  const form = $('wifiForm');
  $('setupCard').classList.toggle('hidden', !w.apActive);
  $('wifiCard').classList.toggle('hidden', w.apActive);
  const host = w.apActive ? $('setupCard') : $('wifiFormStation');
  if (form.parentElement !== host) host.appendChild(form);

  $('wSsid').textContent = w.ssid || '—';
  $('wAddr').textContent = w.state === 'connected' ? `http://${w.hostname} · ${w.ip}` : '—';
  $('wRssi').textContent = w.state === 'connected' ? `${w.rssi} dBm` : '—';

  const msg = $('wifiMsg');
  if (w.apActive) {
    if (w.attempt === 'trying') { msg.textContent = t('testing'); msg.className = 'msg'; }
    else if (w.attempt === 'ok') { msg.textContent = t('testOk') + `http://${w.hostname} (${w.ip})`; msg.className = 'msg ok'; }
    else if (w.attempt === 'failed') { msg.textContent = t('testFail') + reasonText(w); msg.className = 'msg err'; }
  }

  const l = s.led;
  const meter = $('ledMeter');
  let txt = `${t('ledEst')} ${l.estimatedMa} mA · ${l.count} LED`;
  if (l.limited) txt += ' · ' + t('limited');
  if (+l.softStart < 1) txt += ' · ' + t('softStarting') + ' ' + Math.round(l.softStart * 100) + '%';
  meter.textContent = txt;
  meter.className = 'meter' + (l.limited ? ' warn' : '');

  const up = s.uptimeSec;
  const upTxt = up > 86400 ? Math.floor(up / 86400) + ' d ' + Math.floor(up % 86400 / 3600) + ' h' : Math.floor(up / 3600) + ' h ' + Math.floor(up % 3600 / 60) + ' min';
  $('sysInfo').textContent = `${t('uptime')}: ${upTxt} · ${t('heap')}: ${Math.round(s.heapFree / 1024)} kB`;
  wizRender();
}

// ── First-run wizard (setup AP only): Wi-Fi (skippable) → LEDs + supply ─
// Everything else keeps its defaults and is changed later in the panel.
const WIZ_STEPS = 2;
let wizStep = 0;  // 0 = wizard off, normal panel

function wizFillLed() {
  if (!cfg) return;
  $('wizCount').value = cfg.module.count;
  // Pick the supply whose LED budget is closest to the saved one.
  const opts = [...$('wizSupply').options];
  const best = opts.reduce((a, o) => Math.abs(o.value - cfg.module.budgetMa) < Math.abs(a.value - cfg.module.budgetMa) ? o : a);
  $('wizSupply').value = best.value;
  wizEstimate();
}

function wizEstimate() {
  const ma = (+$('wizCount').value || 0) * (cfg ? cfg.module.maPerPixel : 60);
  const over = ma > +$('wizSupply').value;
  $('wizEst').textContent = t(over ? 'wizEstLimit' : 'wizEstOk').replace('{ma}', ma);
  $('wizEst').className = 'meter' + (over ? ' warn' : '');
}

function wizShow(step) {
  wizStep = step;
  document.body.classList.toggle('wizard', step > 0);
  $('wizHead').classList.toggle('hidden', !step);
  $('wizFoot').classList.toggle('hidden', !step);
  document.querySelectorAll('main > .card').forEach((c) => c.classList.toggle('wizOn', +c.dataset.step === step));
  document.querySelectorAll('#wizHead li').forEach((li, i) => {
    li.classList.toggle('on', i + 1 === step);
    li.classList.toggle('done', i + 1 < step);
  });
  wizRender();
  window.scrollTo(0, 0);
}

function wizRender() {
  if (!wizStep) return;
  $('wizBack').classList.toggle('hidden', wizStep === 1);
  // Wi-Fi step: "Next" once connected (or the test in AP mode succeeded), otherwise "Skip".
  const w = status && status.wifi;
  const online = w && (w.state === 'connected' || w.attempt === 'ok');
  let next = 'next', hint = '';
  if (wizStep === 1 && !online) { next = 'skip'; hint = t('skipHint'); }
  if (wizStep === WIZ_STEPS) { next = 'finish'; hint = t('ledStepHint'); wizEstimate(); }
  $('wizNext').textContent = t(next);
  $('wizHint').textContent = hint;
}

async function wizNext() {
  if (wizStep < WIZ_STEPS) {
    if (wizStep === 1) wizFillLed();
    wizShow(wizStep + 1);
    return;
  }
  // 'custom' keeps the default chip/order/shape and only takes these two values.
  const ok = await saveConfig({
    module: { preset: 'custom', count: +$('wizCount').value, budgetMa: +$('wizSupply').value },
    system: { setupDone: true }
  }, true);
  if (ok) {
    wizShow(0);
    toast(t('setupFinished'));
  }
}

// Router's disconnect reason (ESP-IDF wifi_err_reason_t) → short hint.
const REASON_HINT = {
  2: 'whyPass', 14: 'whyPass', 15: 'whyPass', 202: 'whyPass', 204: 'whyPass',
  201: 'whyNoAp', 210: 'whySec', 211: 'whySec', 200: 'whyWeak', 212: 'whyWeak'
};
function reasonText(w) {
  if (!w.reason) return '';
  const hint = REASON_HINT[w.reason];
  return ` (${t('reasonCode')} ${w.reason}: ${w.reasonName}${hint ? ' — ' + t(hint) : ''})`;
}

async function pollStatus() {
  try { renderStatus(await api('/api/status')); } catch (e) { /* device restarting or offline */ }
}

// ── Wi-Fi scan / connect ────────────────────────────────────
async function scan(refresh) {
  const sel = $('ssidSelect');
  sel.innerHTML = '';
  sel.add(new Option(t('scanning'), ''));
  for (let i = 0; i < 20; i++) {
    let r;
    try { r = await api('/api/wifi/scan' + (refresh && i === 0 ? '?refresh=1' : '')); } catch (e) { break; }
    if (!r.running) {
      sel.innerHTML = '';
      const nets = (r.networks || []).sort((a, b) => b.rssi - a.rssi);
      const seen = new Set();
      sel.add(new Option(nets.length ? t('pickNet') : t('noNets'), ''));
      nets.forEach((n) => {
        if (seen.has(n.ssid)) return;
        seen.add(n.ssid);
        sel.add(new Option(`${n.ssid}  (${n.rssi} dBm${n.secure ? ' 🔒' : ''})`, n.ssid));
      });
      return;
    }
    await new Promise((res) => setTimeout(res, 1000));
  }
}

async function connect() {
  const ssid = $('ssidManual').value.trim() || $('ssidSelect').value;
  if (!ssid) { toast(t('pickNet')); return; }
  try {
    await api('/api/wifi', { ssid, password: $('wifiPass').value });
    $('wifiMsg').textContent = t('testing');
    $('wifiMsg').className = 'msg';
  } catch (e) { toast(t('error') + ': ' + e.message); }
}

// ── Wiring ──────────────────────────────────────────────────
function bind() {
  $('lightOn').addEventListener('change', pushLight);
  $('lightColor').addEventListener('input', pushLight);
  $('lightBright').addEventListener('input', () => { $('brightVal').textContent = $('lightBright').value + '%'; pushLight(); });
  $('lightDensity').addEventListener('change', pushLight);

  $('modPreset').addEventListener('change', () => saveConfig({ module: { preset: $('modPreset').value } }));
  ['modCount', 'modChip', 'modShape', 'modMa'].forEach((id) =>
    $(id).addEventListener('change', () => { $('modPreset').value = 'custom'; }));
  $('modSave').addEventListener('click', () => saveModule(false));
  $('wizBack').addEventListener('click', () => wizShow(wizStep - 1));
  $('wizNext').addEventListener('click', wizNext);
  $('wizCount').addEventListener('input', wizEstimate);
  $('wizSupply').addEventListener('change', wizEstimate);

  $('tzSelect').addEventListener('change', () => {
    const z = ZONES.find(([n]) => n === $('tzSelect').value);
    if (z) saveConfig({ time: { tzName: z[0], tz: z[1] } });
  });
  $('timeFromPhone').addEventListener('click', async () => {
    try { await api('/api/time', { epoch: Math.floor(Date.now() / 1000) }); toast(t('saved')); pollStatus(); }
    catch (e) { toast(t('error') + ': ' + e.message); }
  });

  $('rescanBtn').addEventListener('click', () => scan(true));
  $('ssidSelect').addEventListener('change', () => {
    if ($('ssidSelect').value) $('ssidManual').value = $('ssidSelect').value;
  });
  $('passToggle').addEventListener('click', () => {
    const p = $('wifiPass');
    const show = p.type === 'password';
    p.type = show ? 'text' : 'password';
    $('passToggle').dataset.i18n = show ? 'hidePass' : 'showPass';
    $('passToggle').textContent = t($('passToggle').dataset.i18n);
  });
  $('connectBtn').addEventListener('click', connect);
  document.querySelector('#wifiCard details').addEventListener('toggle', (e) => { if (e.target.open) scan(true); });
  $('forgetBtn').addEventListener('click', async () => {
    if (!confirm(t('confirmForget'))) return;
    try { await api('/api/wifi/forget', {}); toast(t('saved')); } catch (e) { toast(t('error') + ': ' + e.message); }
  });

  $('sysSave').addEventListener('click', () => saveConfig({
    output: { maxBrightness: +$('outMax').value, softStartSec: +$('outSoft').value },
    system: { hostname: $('sysHost').value.trim() }
  }));
  $('restartBtn').addEventListener('click', async () => {
    try { await api('/api/system/restart', {}); toast(t('restarting')); } catch (e) { toast(t('error')); }
  });
  $('defaultsBtn').addEventListener('click', async () => {
    if (!confirm(t('confirmDefaults'))) return;
    try { const r = await api('/api/system/defaults', { profile: cfg ? cfg.profile : 'terra' }); fillForm(r.config); toast(t('saved')); }
    catch (e) { toast(t('error') + ': ' + e.message); }
  });

  $('langBtn').addEventListener('click', () => {
    lang = lang === 'pl' ? 'en' : 'pl';
    applyI18n();
    saveConfig({ system: { lang } }, true);
    if (status) renderStatus(status);
  });
}

async function init() {
  applyI18n();
  bind();
  try { fillForm((await api('/api/config')).config); } catch (e) { toast(t('error') + ': ' + e.message); }
  await pollStatus();
  // The wizard is the setup-AP experience; on the home network use the panel.
  if (cfg && !cfg.system.setupDone && status && status.wifi.apActive) wizShow(1);
  if (status && status.wifi.apActive) scan(false);
  setInterval(pollStatus, 3000);
}

init();
