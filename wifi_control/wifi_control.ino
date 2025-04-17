#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "BLDC_Control";     // WiFi network name
const char* password = "password123";   // WiFi password

// Create web server object
WebServer server(80);

// Servo objects for ESCs
Servo ESC1;  // create servo object to control the ESC1
Servo ESC2;  // create servo object to control the ESC2

// PWM values for each motor (0-180 for servo library)
int pwmESC1 = 0;
int pwmESC2 = 0;

// PWM adjustment step size
const int PWM_STEP = 1;

// Pin definitions
const int potPin = 15;  // ESP32 Pin for reading Analog Value
const int ESCpin1 = 5;  // PWM pin for ESC1
const int ESCpin2 = 4;  // PWM pin for ESC2

// ESC arming state
bool escsArmed = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESC Calibration and Arming Sequence...");
  
  // Attach the ESCs
  ESC1.attach(ESCpin1, 1000, 2000);  // (pin, min pulse width, max pulse width in microseconds)
  ESC2.attach(ESCpin2, 1000, 2000);
  
  // ESC Arming Sequence
  // Step 1: Set to zero throttle
  ESC1.write(0);
  ESC2.write(0);
  delay(1000);
  
  Serial.println("ESCs initialized. Starting WiFi...");
  
  // Setup Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Define server endpoints
  server.on("/", handleRoot);
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/arm", HTTP_GET, handleArm);
  server.onNotFound(handleNotFound);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Connect to BLDC_Control WiFi network and go to http://192.168.4.1");
  Serial.println("You'll need to arm the ESCs before motors will respond.");
}

void loop() {
  server.handleClient();
  
  // Apply PWM values to ESCs only if armed
  if (escsArmed) {
    ESC1.write(pwmESC1);
    ESC2.write(pwmESC2);
  }
  
  delay(10);
}

// Handle root page
void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>BLDC Motor Control</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; }";
  html += "h1 { color: #333; }";
  html += ".container { max-width: 500px; margin: 0 auto; }";
  html += ".control-group { background-color: #f4f4f4; border-radius: 10px; padding: 15px; margin: 15px 0; }";
  html += ".btn { background-color: #4CAF50; border: none; color: white; padding: 10px 15px; ";
  html += "text-align: center; text-decoration: none; display: inline-block; font-size: 16px; ";
  html += "margin: 4px 2px; cursor: pointer; border-radius: 5px; }";
  html += ".btn-red { background-color: #f44336; }";
  html += ".btn-blue { background-color: #008CBA; }";
  html += ".btn-arm { background-color: #FF9800; padding: 15px 25px; font-size: 18px; }";
  html += ".btn-sync { background-color: #9C27B0; }";
  html += ".value-display { font-size: 24px; font-weight: bold; margin: 10px 0; }";
  html += ".status { padding: 10px; margin: 10px 0; border-radius: 5px; }";
  html += ".armed { background-color: #dff0d8; color: #3c763d; }";
  html += ".disarmed { background-color: #f2dede; color: #a94442; }";
  html += "</style></head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>BLDC Motor Control</h1>";
  
  // Status and arming
  html += "<div class='control-group'>";
  if (escsArmed) {
    html += "<div class='status armed'>Status: ARMED</div>";
  } else {
    html += "<div class='status disarmed'>Status: DISARMED - Press ARM button to start</div>";
    html += "<button class='btn btn-arm' onclick='armESCs()'>ARM ESCs</button>";
  }
  html += "</div>";
  
  // ESC1 Controls
  html += "<div class='control-group'>";
  html += "<h2>ESC1 Control</h2>";
  html += "<div class='value-display'>PWM: <span id='pwm1'>" + String(pwmESC1) + "</span></div>";
  html += "<button class='btn' onclick='updatePWM(1, 1)' " + String(escsArmed ? "" : "disabled") + ">+1</button>";
  html += "<button class='btn' onclick='updatePWM(1, 5)' " + String(escsArmed ? "" : "disabled") + ">+5</button>";
  html += "<button class='btn btn-red' onclick='updatePWM(1, -1)' " + String(escsArmed ? "" : "disabled") + ">-1</button>";
  html += "<button class='btn btn-red' onclick='updatePWM(1, -5)' " + String(escsArmed ? "" : "disabled") + ">-5</button>";
  html += "</div>";
  
  // ESC2 Controls
  html += "<div class='control-group'>";
  html += "<h2>ESC2 Control</h2>";
  html += "<div class='value-display'>PWM: <span id='pwm2'>" + String(pwmESC2) + "</span></div>";
  html += "<button class='btn' onclick='updatePWM(2, 1)' " + String(escsArmed ? "" : "disabled") + ">+1</button>";
  html += "<button class='btn' onclick='updatePWM(2, 5)' " + String(escsArmed ? "" : "disabled") + ">+5</button>";
  html += "<button class='btn btn-red' onclick='updatePWM(2, -1)' " + String(escsArmed ? "" : "disabled") + ">-1</button>";
  html += "<button class='btn btn-red' onclick='updatePWM(2, -5)' " + String(escsArmed ? "" : "disabled") + ">-5</button>";
  html += "</div>";
  
  // Sync Controls
  html += "<div class='control-group'>";
  html += "<h2>Sync Controls</h2>";
  html += "<button class='btn btn-sync' onclick='syncMotors()' " + String(escsArmed ? "" : "disabled") + ">Sync Motors</button>";
  html += "<button class='btn btn-blue' onclick='setMotors(0)' " + String(escsArmed ? "" : "disabled") + ">Stop Motors</button>";
  html += "</div>";
  
  // Add JavaScript
  html += "<script>";
  html += "function updatePWM(motor, step) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/update?motor=' + motor + '&step=' + step, true);";
  html += "  xhr.onload = function() {";
  html += "    if (xhr.status === 200) {";
  html += "      var response = JSON.parse(xhr.responseText);";
  html += "      document.getElementById('pwm1').innerHTML = response.pwm1;";
  html += "      document.getElementById('pwm2').innerHTML = response.pwm2;";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "function syncMotors() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/update?sync=1', true);";
  html += "  xhr.onload = function() {";
  html += "    if (xhr.status === 200) {";
  html += "      var response = JSON.parse(xhr.responseText);";
  html += "      document.getElementById('pwm1').innerHTML = response.pwm1;";
  html += "      document.getElementById('pwm2').innerHTML = response.pwm2;";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "function setMotors(value) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/update?set=' + value, true);";
  html += "  xhr.onload = function() {";
  html += "    if (xhr.status === 200) {";
  html += "      var response = JSON.parse(xhr.responseText);";
  html += "      document.getElementById('pwm1').innerHTML = response.pwm1;";
  html += "      document.getElementById('pwm2').innerHTML = response.pwm2;";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "function armESCs() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/arm', true);";
  html += "  xhr.onload = function() {";
  html += "    if (xhr.status === 200) {";
  html += "      location.reload();";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleUpdate() {
  String response = "{";
  
  // Only process updates if ESCs are armed
  if (escsArmed) {
    // Check if we're updating a specific motor
    if (server.hasArg("motor") && server.hasArg("step")) {
      int motor = server.arg("motor").toInt();
      int step = server.arg("step").toInt();
      
      if (motor == 1) {
        pwmESC1 = constrain(pwmESC1 + step, 0, 180);
      } else if (motor == 2) {
        pwmESC2 = constrain(pwmESC2 + step, 0, 180);
      }
    }
    
    // Check if we're syncing motors
    if (server.hasArg("sync")) {
      int avgPWM = (pwmESC1 + pwmESC2) / 2;
      pwmESC1 = avgPWM;
      pwmESC2 = avgPWM;
    }
    
    // Check if we're setting both motors to a specific value
    if (server.hasArg("set")) {
      int setValue = server.arg("set").toInt();
      setValue = constrain(setValue, 0, 180);
      pwmESC1 = setValue;
      pwmESC2 = setValue;
    }
    
    // Print values to serial for debugging
    Serial.print("ESC1 PWM: ");
    Serial.print(pwmESC1);
    Serial.print(", ESC2 PWM: ");
    Serial.println(pwmESC2);
  }
  
  // Send JSON response with current values
  response += "\"pwm1\":" + String(pwmESC1) + ",";
  response += "\"pwm2\":" + String(pwmESC2) + ",";
  response += "\"armed\":" + String(escsArmed ? "true" : "false");
  response += "}";
  server.send(200, "application/json", response);
}

void handleArm() {
  Serial.println("Arming ESCs...");
  
  // Standard ESC arming sequence
  ESC1.write(0);  // Set to zero throttle
  ESC2.write(0);
  delay(1000);    // Wait for ESCs to initialize
  
  escsArmed = true;
  Serial.println("ESCs Armed!");
  
  server.send(200, "text/plain", "ESCs Armed");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}