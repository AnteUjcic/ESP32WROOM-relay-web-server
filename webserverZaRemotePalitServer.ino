#include <WiFi.h>

const char* ssid = "GTFOmyInternet";
const char* password = "14Eccho5$";
WiFiServer server(80);

String header;
String output16State = "off";

const int relayPin = 16;  // GPIO pin connected to relay
const int inputPin = 5;   // GPIO pin for digital input

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;  // Increased timeout to ensure stability

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(inputPin, INPUT);  // Set input pin mode
  digitalWrite(relayPin, LOW);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /16/on") >= 0) {
              Serial.println("Relay on");
              output16State = "on";
              digitalWrite(relayPin, HIGH);
              delay(500);  // Simulate button press
              digitalWrite(relayPin, LOW);
            } else if (header.indexOf("GET /16/off") >= 0) {
              Serial.println("Relay off");
              output16State = "off";
              digitalWrite(relayPin, LOW);
            }

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>Relay - State " + output16State + "</p>");
            if (output16State == "off") {
              client.println("<p><a href=\"/16/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/16/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // Check the state of the input pin
  if (digitalRead(inputPin) == HIGH) {
    Serial.println("Digital input HIGH, turning on relay");
    digitalWrite(relayPin, HIGH);
    delay(500);  // Simulate button press
    digitalWrite(relayPin, LOW);  // Reset to LOW after delay
  }
}
