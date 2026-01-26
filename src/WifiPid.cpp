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
    bool ok = WiFi.softAP("LineFollowerPID", "12345678");

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

void WifiPid::stop() {
    running = false;
    pid.reset();
}

bool WifiPid::isRunning() const { return running; }

float WifiPid::getKp() const { return kp; }
float WifiPid::getKi() const { return ki; }
float WifiPid::getKd() const { return kd; }
int   WifiPid::getBaseSpeed() const { return baseSpeed; }

void WifiPid::setupServer() {

    // ===== Main page =====
    server.on("/", HTTP_GET, [this]() {
        String page;
        page += "<!doctype html><html><head>";
        page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        page += "<title>LineFollower</title>";

        // FIX: auto-refresh so you can see CALIBRATING: 0s,1s,2s...
        page += "<meta http-equiv='refresh' content='1'>";

        page += "</head>";
        page += "<body style='font-family:sans-serif;max-width:520px;margin:20px'>";
        page += "<h2>Line follower</h2>";

        page += "<p>Status: <b>";
        page += (running ? "RUNNING" : "STOPPED");
        page += "</b></p>";

        // ADDED: show calibration status on page (must be BEFORE </body></html>)
        page += "<p>Calibration: <b>";
        page += calibrationStatus;
        page += "</b></p>";

        page += "<p>";
        page += "<a href='/start'><button style='padding:10px 16px;font-size:16px;'>START</button></a> ";
        page += "<a href='/stop'><button style='padding:10px 16px;font-size:16px;'>STOP</button></a> ";
        page += "<a href='/calibrate'><button style='padding:10px 16px;font-size:16px;'>CALIBRATE</button></a>";
        page += "</p><hr>";

        // PID form
        page += "<h3>PID tuning</h3>";
        page += "<form action='/set' method='get'>";
        page += "Kp: <input type='number' name='kp' step='0.0001' value='" + String(kp, 4) + "'><br><br>";
        page += "Ki: <input type='number' name='ki' step='0.0001' value='" + String(ki, 4) + "'><br><br>";
        page += "Kd: <input type='number' name='kd' step='0.0001' value='" + String(kd, 4) + "'><br><br>";
        page += "<input type='submit' value='Update PID'>";
        page += "</form>";

        // Base speed form
        page += "<hr><h3>Speed</h3>";
        page += "<form action='/setBase' method='get'>";
        page += "Base speed: <input type='number' name='base' min='0' max='150' step='1' value='" + String(baseSpeed) + "'>";
        page += "<input type='submit' value='Update speed'>";
        page += "</form>";

        page += "<p style='margin-top:20px;color:#555'>WiFi: <b>LineFollowerPID</b> (pass: <b>12345678</b>)</p>";
        page += "</body></html>";

        server.send(200, "text/html", page);
    });

    // START
    server.on("/start", HTTP_GET, [this]() {
        start();
        Serial.println("RUN -> START");
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // STOP
    server.on("/stop", HTTP_GET, [this]() {
        stop();
        Serial.println("RUN -> STOP");
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // ADDED: CALIBRATE endpoint that updates status while calibrating and when done
    server.on("/calibrate", HTTP_GET, [this]() {
        stop(); // safety: stop robot while calibrating

        calibrationStatus = "CALIBRATING: 0s";
        Serial.println("Calibration started");

        unsigned long startMs = millis();
        const unsigned long durationMs = 2000;

        // Teller i sekunder mens vi "holder på"
        while (millis() - startMs < durationMs) {
            int seconds = (int)((millis() - startMs) / 1000);
            calibrationStatus = "CALIBRATING: " + String(seconds) + "s";
            delay(50);  // holder CPU rolig
        }

        // Kjør selve kalibreringen ÉN gang
        ir.calibrate(2000);

        calibrationStatus = "DONE";
        Serial.println("Calibration finished");

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