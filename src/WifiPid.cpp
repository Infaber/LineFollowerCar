#include "WifiPid.hpp"
#include "PID.hpp"

#include <WiFi.h>
#include <WebServer.h>

static WebServer server(80);

WifiPid::WifiPid(PID& pidRef)
    : pid(pidRef), kp(0.03), ki(0.0), kd(0.10) {}

void WifiPid::begin() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("LineFollowerPID", "12345678");

    Serial.print("WiFi AP IP: ");
    Serial.println(WiFi.softAPIP());

    setupServer();
    server.begin();
}

void WifiPid::handle() {
    server.handleClient();
}

float WifiPid::getKp() const { return kp; }
float WifiPid::getKi() const { return ki; }
float WifiPid::getKd() const { return kd; }

void WifiPid::setupServer() {
    server.on("/", HTTP_GET, [this]() {
        String page =
            "<html><body>"
            "<h2>PID tuning</h2>"
            "<form action='/set'>"
            "Kp: <input name='kp' step='0.0001' value='" + String(kp, 4) + "'><br>"
            "Ki: <input name='ki' step='0.0001' value='" + String(ki, 4) + "'><br>"
            "Kd: <input name='kd' step='0.0001' value='" + String(kd, 4) + "'><br>"
            "<input type='submit' value='Update'>"
            "</form>"
            "</body></html>";

        server.send(200, "text/html", page);
    });

    server.on("/set", HTTP_GET, [this]() {
        if (server.hasArg("kp")) kp = server.arg("kp").toFloat();
        if (server.hasArg("ki")) ki = server.arg("ki").toFloat();
        if (server.hasArg("kd")) kd = server.arg("kd").toFloat();

        pid.setTunings(kp, ki, kd);
        pid.reset();

        server.sendHeader("Location", "/");
        server.send(302);
    });
}
