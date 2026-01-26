#include "WifiPid.hpp"
#include "PID.hpp"

#include <WiFi.h>
#include <WebServer.h>

static WebServer server(80);

static bool isValidNumber(float v) {
    return !isnan(v) && isfinite(v);
}

WifiPid::WifiPid(PID& pidRef, float startKp, float startKi, float startKd)
    : pid(pidRef), kp(startKp), ki(startKi), kd(startKd), running(false) {}

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

bool WifiPid::isRunning() const {
    return running;
}

float WifiPid::getKp() const { return kp; }
float WifiPid::getKi() const { return ki; }
float WifiPid::getKd() const { return kd; }

void WifiPid::setupServer() {
    //Root page
    server.on("/", HTTP_GET, [this]() {
        String page;
        page += "<!doctype html><html><head>";
        page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        page += "<title>LineFollower</title></head><body style='font-family:sans-serif;max-width:520px;margin:20px'>";
        page += "<h2>Line follower</h2>";

        page += "<p>Status: <b>";
        page += (running ? "RUNNING" : "STOPPED");
        page += "</b></p>";

        page += "<p>";
        page += "<a href='/start'><button style='padding:10px 16px;font-size:16px;'>START</button></a> ";
        page += "<a href='/stop'><button style='padding:10px 16px;font-size:16px;'>STOP</button></a>";
        page += "</p><hr>";

        page += "<h3>PID tuning</h3>";
        page += "<form action='/set' method='get'>";
        page += "Kp: <input type='number' name='kp' step='0.0001' value='" + String(kp, 4) + "'><br><br>";
        page += "Ki: <input type='number' name='ki' step='0.0001' value='" + String(ki, 4) + "'><br><br>";
        page += "Kd: <input type='number' name='kd' step='0.0001' value='" + String(kd, 4) + "'><br><br>";
        page += "<input type='submit' value='Update'>";
        page += "</form>";

        page += "<p style='margin-top:20px;color:#555'>Koble til WiFi: <b>LineFollowerPID</b> (pass: <b>12345678</b>)</p>";
        page += "</body></html>";

        server.send(200, "text/html", page);
    });

    // Start
    server.on("/start", HTTP_GET, [this]() {
        start();
        Serial.println("RUN -> START");
        server.sendHeader("Location", "/");
        server.send(302);
    });

    // Stop
    server.on("/stop", HTTP_GET, [this]() {
        stop();
        Serial.println("RUN -> STOP");
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

            Serial.printf("PID updated via WiFi -> Kp=%.5f Ki=%.5f Kd=%.5f\n", kp, ki, kd);
        } else {
            Serial.println("Invalid PID values received over WiFi");
        }

        server.sendHeader("Location", "/");
        server.send(302);
    });
}