
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"

// Replace with your network credentials
const char* ssid     = "...";
const char* password = "...";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

Adafruit_MCP23017 mcp;

void setup() {
  Serial.begin(9600);

  mcp.begin(0, 2); 

  for (int i = 0; i < 16; ++i){
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, LOW);
  }

  //set the wifi hostname
  WiFi.hostname("lights.local");

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("application/json");
            client.println("Connection: close");
            client.println();
            
            // Display the JSON web page
            client.print("{\"status\": 200,\"operation\": \"");

            if (header.indexOf("GET /on/") >= 0) {
                uint8_t port = header.substring(8, 10).toInt();
                if (port >= 0 && port <= 15) {
                  mcp.digitalWrite(port, HIGH);                
                  client.print(port);
                  client.print(" on\"");
                } else {
                  client.print("invalid port\"");                  
                }
            } else if (header.indexOf("GET /off/") >= 0) {
                uint8_t port = header.substring(9, 11).toInt();
                if (port >= 0 && port <= 15) {
                  mcp.digitalWrite(port, LOW);                
                  client.print(port);
                  client.print(" off\"");
                } else {
                  client.print("invalid port\"");                  
                }
            } else if (header.indexOf("GET /onall") >= 0) {
                for (int i = 0; i < 16; ++i){
                  mcp.digitalWrite(i, HIGH);  
                } 
                client.print("on all\"");
            } else if (header.indexOf("GET /offall") >= 0) {
                for (int i = 0; i < 16; ++i){
                  mcp.digitalWrite(i, LOW);  
                } 
                client.print("off all\"");
            } else {
                client.print("read\"");
            }
                    
            client.print(", \"lights\": {");

            for (int i = 0; i < 16; ++i){
              client.print("\"");
              client.print(i);
              client.print("\": ");
                if (mcp.digitalRead(i) == HIGH) {
                    client.print("\"on\"");
                } else {
                    client.print("\"off\"");
                }

                if (i < 15) client.print(",");
            }             

            client.print("}}");

            // The HTTP response ends with another blank line
            client.println();

            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
