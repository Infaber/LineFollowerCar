#include "WifiPid.hpp"
#include "PID.hpp"
#include "IRSensor.hpp"

#include <WiFi.h>
#include <WebServer.h>

static WebServer server(80);

static bool isValidNumber(float v) {
    return !isnan(v) && isfinite(v);
}

WifiPid::WifiPid(PID& pidRef, IRSensor& irRef, int& baseSpeedRef, float& turnGainRef,
                 float startKp, float startKi, float startKd)
    : pid(pidRef),
      ir(irRef),
      baseSpeed(baseSpeedRef),
      turnGain(turnGainRef),
      kp(startKp),
      ki(startKi),
      kd(startKd),
      running(false) {}

void WifiPid::begin() {
    Serial.println("WifiPid::begin()");

    WiFi.mode(WIFI_AP);
    bool ok = WiFi.softAP("Linjefølger G11", "12345678");

    Serial.print("softAP ok: ");
    Serial.println(ok ? "true" : "false");

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    setupServer();
    server.begin();
    Serial.println("WebServer started");
}

void WifiPid::handle() {
    server.handleClient();
}

void WifiPid::start() {
    running = true;
    pid.reset();
}

void WifiPid::stopp() {
    running = false;
    pid.reset();
}

bool WifiPid::isRunning() const { return running; }

float WifiPid::getKp() const { return kp; }
float WifiPid::getKi() const { return ki; }
float WifiPid::getKd() const { return kd; }
int   WifiPid::getBaseSpeed() const { return baseSpeed; }
float WifiPid::getTurnGain() const { return turnGain; }

void WifiPid::setupServer() {

    // ===== STATUS endpoint (JSON) =====
    server.on("/status", HTTP_GET, [this]() {
        String json = "{";
        json += "\"kjorer\":" + String(running ? "true" : "false") + ",";
        json += "\"kalibrering\":\"" + calibrationStatus + "\",";
        json += "\"kp\":" + String(kp, 4) + ",";
        json += "\"ki\":" + String(ki, 4) + ",";
        json += "\"kd\":" + String(kd, 4) + ",";
        json += "\"base\":" + String(baseSpeed) + ",";
        json += "\"gain\":" + String(turnGain, 2);
        json += "}";
        server.send(200, "application/json", json);
    });

    // ===== Hovedside =====
    server.on("/", HTTP_GET, [this]() {
        String page;
        page += "<!doctype html><html><head>";
        page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        page += "<title>Linjefølger</title>";
        page += "</head>";

        page += "<body style='font-family:sans-serif;max-width:520px;margin:20px'>";
        page += "<h2>Linjefølger</h2>";

        page += "<p>Status: <b id='kjorer'>";
        page += (running ? "KJØRER" : "STOPPET");
        page += "</b></p>";

        page += "<p>Kalibrering: <b id='kal'>";
        page += calibrationStatus;
        page += "</b></p>";

        // Buttons (ingen redirect -> mindre delay)
        page += "<p>";
        page += "<button onclick='fetch(\"/start\")' style='padding:10px 16px;font-size:16px;'>START</button> ";
        page += "<button onclick='fetch(\"/stopp\")' style='padding:10px 16px;font-size:16px;'>STOPP</button> ";
        page += "<button onclick='fetch(\"/kalibrer\")' style='padding:10px 16px;font-size:16px;'>KALIBRER</button>";
        page += "</p><hr>";

        // PID form
        page += "<h3>PID-justering</h3>";
        page += "<form action='/sett_pid' method='get'>";
        page += "Kp: <input type='number' name='kp' step='0.0001' value='" + String(kp, 4) + "'><br><br>";
        page += "Ki: <input type='number' name='ki' step='0.0001' value='" + String(ki, 4) + "'><br><br>";
        page += "Kd: <input type='number' name='kd' step='0.0001' value='" + String(kd, 4) + "'><br><br>";
        page += "<input type='submit' value='Oppdater PID'>";
        page += "</form>";

        // Base speed (0..255)
        page += "<hr><h3>Hastighet</h3>";
        page += "<form action='/sett_hastighet' method='get'>";
        page += "Grunnhastighet: <input type='number' name='base' min='0' max='255' step='1' value='" + String(baseSpeed) + "'>";
        page += "<input type='submit' value='Oppdater hastighet'>";
        page += "</form>";

        // Turn gain (0.10..3.00)
        page += "<hr><h3>Svingefølsomhet (Gain)</h3>";
        page += "<form action='/sett_gain' method='get'>";
        page += "Gain: <input type='number' name='gain' min='0.10' max='3.00' step='0.05' value='" + String(turnGain, 2) + "'>";
        page += "<input type='submit' value='Oppdater gain'>";
        page += "</form>";

        page += "<p style='margin-top:20px;color:#555'>WiFi: <b>Linjefølger G11</b> (passord: <b>12345678</b>)</p>";

        // JS: poll status (oppdater tekst uten reload)
        page += "<script>"
                "async function tick(){"
                "  try{"
                "    const r = await fetch('/status',{cache:'no-store'});"
                "    const s = await r.json();"
                "    document.getElementById('kjorer').textContent = s.kjorer ? 'KJØRER' : 'STOPPET';"
                "    document.getElementById('kal').textContent = s.kalibrering;"
                "  }catch(e){}"
                "}"
                "setInterval(tick, 250); tick();"
                "</script>";

        page += "</body></html>";
        server.send(200, "text/html", page);
    });

    // START
    server.on("/start", HTTP_GET, [this]() {
        start();
        Serial.println("KJØR -> START");
        server.send(200, "text/plain", "OK");
    });

    // STOPP
    server.on("/stopp", HTTP_GET, [this]() {
        stopp();
        Serial.println("KJØR -> STOPP");
        server.send(200, "text/plain", "OK");
    });

    // KALIBRER (blokkerer mens ir.calibrate kjører)
    server.on("/kalibrer", HTTP_GET, [this]() {
        stopp();
        calibrationStatus = "KALIBRERER...";
        Serial.println("Kalibrering startet");

        ir.calibrate(2000);

        calibrationStatus = "FERDIG";
        Serial.println("Kalibrering ferdig");
        server.send(200, "text/plain", "OK");
    });

    // Sett PID
    server.on("/sett_pid", HTTP_GET, [this]() {
        float newKp = kp, newKi = ki, newKd = kd;

        if (server.hasArg("kp")) newKp = server.arg("kp").toFloat();
        if (server.hasArg("ki")) newKi = server.arg("ki").toFloat();
        if (server.hasArg("kd")) newKd = server.arg("kd").toFloat();

        if (isValidNumber(newKp) && isValidNumber(newKi) && isValidNumber(newKd)) {
            kp = newKp; ki = newKi; kd = newKd;
            pid.setTunings(kp, ki, kd);
            pid.reset();
            Serial.printf("PID oppdatert -> Kp=%.5f Ki=%.5f Kd=%.5f\n", kp, ki, kd);
        } else {
            Serial.println("Ugyldige PID-verdier mottatt over WiFi");
        }

        server.sendHeader("Location", "/");
        server.send(302);
    });

    // Sett grunnhastighet (0..255)
    server.on("/sett_hastighet", HTTP_GET, [this]() {
        if (server.hasArg("base")) {
            int b = server.arg("base").toInt();
            b = constrain(b, 0, 255);
            baseSpeed = b;
            Serial.print("Grunnhastighet oppdatert -> ");
            Serial.println(baseSpeed);
        }
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // Sett gain (0.10..3.00)
    server.on("/sett_gain", HTTP_GET, [this]() {
        if (server.hasArg("gain")) {
            float g = server.arg("gain").toFloat();
            if (isValidNumber(g)) {
                if (g < 0.10f) g = 0.10f;
                if (g > 3.00f) g = 3.00f;
                turnGain = g;
                Serial.print("Svingefølsomhet (gain) oppdatert -> ");
                Serial.println(turnGain, 2);
            }
        }
        server.sendHeader("Location", "/");
        server.send(302);
    });
}
