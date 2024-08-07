#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "MyTruckNetwork";
const char* password = "mytruckpassword";

// Relay and switch pins
const int ignitionPin = 2;
const int lightsPin = 3;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("SoftAP started");
  server.on("/", handleRoot);
  server.on("/ignition", handleIgnition);
  server.on("/accMode", handleAccMode);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Truck Control</h1>";
  html += "<h2>Ignition</h2>";
  html += "<label class='switch'>";
  html += "<input type='checkbox' id='ignitionToggle' onchange='updateIgnition(this.checked)'>";
  html += "<span class='slider round'></span>";
  html += "</label>";
  html += "<span>&#x26A1;</span>";
  html += "<h2>ACC Mode</h2>";
  html += "<label class='switch'>";
  html += "<input type='checkbox' id='accModeToggle' onchange='updateAccMode(this.checked)'>";
  html += "<span class='slider round'></span>";
  html += "</label>";
  html += "<span>&#x26A1;</span>";
  html += "<style>";
  html += ".switch {";
  html += "  position: relative;";
  html += "  display: inline-block;";
  html += "  width: 60px;";
  html += "  height: 34px;";
  html += "}";
  html += ".switch input {";
  html += "  opacity: 0;";
  html += "  width: 0;";
  html += "  height: 0;";
  html += "}";
  html += ".slider {";
  html += "  position: absolute;";
  html += "  cursor: pointer;";
  html += "  top: 0;";
  html += "  left: 0;";
  html += "  right: 0;";
  html += "  bottom: 0;";
  html += "  background-color: #ccc;";
  html += "  -webkit-transition: .4s;";
  html += "  transition: .4s;";
  html += "}";
  html += ".slider:before {";
  html += "  position: absolute;";
  html += "  content: '';";
  html += "  height: 26px;";
  html += "  width: 26px;";
  html += "  left: 4px;";
  html += "  bottom: 4px;";
  html += "  background-color: white;";
  html += "  -webkit-transition: .4s;";
  html += "  transition: .4s;";
  html += "}";
  html += "input:checked + .slider {";
  html += "  background-color: #2196F3;";
  html += "}";
  html += "input:focus + .slider {";
  html += "  box-shadow: 0 0 1px #2196F3;";
  html += "}";
  html += "input:checked + .slider:before {";
  html += "  -webkit-transform: translateX(26px);";
  html += "  -ms-transform: translateX(26px);";
  html += "  transform: translateX(26px);";
  html += "}";
  html += ".slider.round {";
  html += "  border-radius: 34px;";
  html += "}";
  html += ".slider.round:before {";
  html += "  border-radius: 50%;";
  html += "}";
  html += "</style>";
  html += "<script>";
  html += "function updateIgnition(checked) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/ignition?state=' + (checked ? 'on' : 'off'), true);";
  html += "  xhr.send();";
  html += "}";
  html += "function updateAccMode(checked) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/accMode?state=' + (checked ? 'on' : 'off'), true);";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleIgnition() {
  String state = server.arg("state");
  if (state == "on") {
    digitalWrite(ignitionPin, HIGH);
  } else {
    digitalWrite(ignitionPin, LOW);
  }
  server.send(200, "text/plain", "Ignition updated");
}

void handleAccMode() {
  String state = server.arg("state");
  if (state == "on") {
    digitalWrite(lightsPin, HIGH);
  } else {
    digitalWrite(lightsPin, LOW);
  }
  server.send(200, "text/plain", "ACC Mode updated");
}