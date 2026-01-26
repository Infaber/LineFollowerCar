#include "WifiPid.hpp"
#include "PID.hpp"
#include "IRSensor.hpp"

#include <WiFi.h>
#include <WebServer.h>

static WebServer server(80);

static bool isValidNumber(float v) {
    return !isnan(v) && isfinite(v);
}

WifiPid::WifiPid(PID& pidRef, IRSensor& irRef, int& baseSpeedRef,
                 float startKp, float startKi, float startKd)
    : pid(pidRef), ir(irRef), baseSpeed(baseSpeedRef),
      kp(startKp), ki(startKi), kd(startKd), running(false) {}

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

void WifiPid::setupServer() {

    // ===== STATUS endpoint (JSON) =====
    server.on("/status", HTTP_GET, [this]() {
        String json = "{";
        json += "\"kjorer\":" + String(running ? "true" : "false") + ",";
        json += "\"kalibrering\":\"" + calibrationStatus + "\",";
        json += "\"kp\":" + String(kp, 4) + ",";
        json += "\"ki\":" + String(ki, 4) + ",";
        json += "\"kd\":" + String(kd, 4) + ",";
        json += "\"base\":" + String(baseSpeed);
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

        page += "<p>";
        page += "<a href='/start'><button style='padding:10px 16px;font-size:16px;'>START</button></a> ";
        page += "<a href='/stopp'><button style='padding:10px 16px;font-size:16px;'>STOPP</button></a> ";
        page += "<a href='/kalibrer'><button style='padding:10px 16px;font-size:16px;'>KALIBRER</button></a>";
        page += "</p><hr>";

        // PID form
        page += "<h3>PID-justering</h3>";
        page += "<form action='/sett_pid' method='get'>";
        page += "Kp: <input type='number' name='kp' step='0.0001' value='" + String(kp, 4) + "'><br><br>";
        page += "Ki: <input type='number' name='ki' step='0.0001' value='" + String(ki, 4) + "'><br><br>";
        page += "Kd: <input type='number' name='kd' step='0.0001' value='" + String(kd, 4) + "'><br><br>";
        page += "<input type='submit' value='Oppdater PID'>";
        page += "</form>";

        // Base speed form
        page += "<hr><h3>Hastighet</h3>";
        page += "<form action='/sett_hastighet' method='get'>";
        page += "Grunnhastighet: <input type='number' name='base' min='0' max='150' step='1' value='" + String(baseSpeed) + "'>";
        page += "<input type='submit' value='Oppdater hastighet'>";
        page += "</form>";

        // WiFi info
        page += "<p style='margin-top:20px;color:#555'>WiFi: <b>Linjefølger G11</b> (passord: <b>12345678</b>)</p>";

        // JS: oppdater status uten reload
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
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // STOPP
    server.on("/stopp", HTTP_GET, [this]() {
        stopp();
        Serial.println("KJØR -> STOPP");
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // KALIBRER
    server.on("/kalibrer", HTTP_GET, [this]() {
        stopp(); // sikkerhet: stopp robot under kalibrering

        calibrationStatus = "KALIBRERER...";
        Serial.println("Kalibrering startet");

        // NB: Hvis ir.calibrate(2000) er blokkerende, vil status ikke oppdateres mens den kjører.
        ir.calibrate(2000);

        calibrationStatus = "FERDIG";
        Serial.println("Kalibrering ferdig");

        server.sendHeader("Location", "/");
        server.send(302);
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

    // Sett grunnhastighet
    server.on("/sett_hastighet", HTTP_GET, [this]() {
        if (server.hasArg("base")) {
            int b = server.arg("base").toInt();
            b = constrain(b, 0, 150);
            baseSpeed = b;
            Serial.print("Grunnhastighet oppdatert -> ");
            Serial.println(baseSpeed);
        }
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // Set PID
    server.on("/set", HTTP_GET, [this]() {
        float newKp = kp, newKi = ki, newKd = kd;

        if (server.hasArg("kp")) newKp = server.arg("kp").toFloat();
        if (server.hasArg("ki")) newKi = server.arg("ki").toFloat();
        if (server.hasArg("kd")) newKd = server.arg("kd").toFloat();

        if (isValidNumber(newKp) && isValidNumber(newKi) && isValidNumber(newKd)) {
            kp = newKp; ki = newKi; kd = newKd;
            pid.setTunings(kp, ki, kd);
            pid.reset();
            Serial.printf("PID updated -> Kp=%.5f Ki=%.5f Kd=%.5f\n", kp, ki, kd);
        } else {
            Serial.println("Invalid PID values received over WiFi");
        }

        server.sendHeader("Location", "/");
        server.send(302);
    });

    // Set Base speed
    server.on("/setBase", HTTP_GET, [this]() {
        if (server.hasArg("base")) {
            int b = server.arg("base").toInt();
            b = constrain(b, 0, 150);
            baseSpeed = b;
            Serial.print("Base speed updated -> ");
            Serial.println(baseSpeed);
        }
        server.sendHeader("Location", "/");
        server.send(302);
    });
}