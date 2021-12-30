// ?? works!
// use https://ipaddr/main
// tries- spiffs fs auto-send-files


#include <Arduino.h>
#include "cert.h"
#include "private_key.h"
#include <WiFi.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <SPIFFS.h>
#include <FS.h>
// We use JSON as data format. Make sure to have the lib available
#include <ArduinoJson.h>
// Working with c++ strings
#include <string>
char contentTypes[][2][32] = {
  {".html", "text/html"},
  {".css",  "text/css"},
  {".js",   "application/javascript"},
  {".json", "application/json"},
  {".png",  "image/png"},
  {".jpg",  "image/jpg"},
  {"", ""}
};



#include <ESPmDNS.h>
#define DOMAINname "esp"

#include <WiFiMulti.h>
#include <WiFiAP.h>
#define ssid1       "Crawford House"
#define password1   "19031903"
#define ssid2       "Temporary"
#define password2   "girlcat1"
#define ssid3       "the chu home"
#define password3   "14141414"

// #define ssid      "esp32"
// #define password  "girlcat1"
// #define password   "girlcat1"

WiFiMulti wifiMulti;

// #define WIFI_SSID "Crawford House"
// #define WIFI_PASS "19031903"
#define WIFI_SSID "Temporary"
#define WIFI_PASS "girlcat1"
// #define WIFI_SSID "the chu home"
// #define WIFI_PASS "14141414"

#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <util.hpp>
using namespace httpsserver;

#include <WebsocketHandler.hpp>
#include <sstream>
#define MAX_CLIENTS 4


void handleSPIFFS(HTTPRequest * req, HTTPResponse * res);
void handleGetEvents(HTTPRequest * req, HTTPResponse * res);
// void handleGetUptime(HTTPRequest * req, HTTPResponse * res);
// void handlePostEvent(HTTPRequest * req, HTTPResponse * res);
// void handleDeleteEvent(HTTPRequest * req, HTTPResponse * res);

SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);
HTTPSServer secureServer = HTTPSServer(&cert, 443, MAX_CLIENTS);

// As websockets are more complex, they need a custom class that is derived from WebsocketHandler
class ChatHandler : public WebsocketHandler {
public:
  // This method is called by the webserver to instantiate a new handler for each
  // client that connects to the websocket endpoint
  static WebsocketHandler* create();

  // This method is called when a message arrives
  void onMessage(WebsocketInputStreambuf * input);

  // Handler function on connection close
  void onClose();
};

// Simple array to store the active clients:
ChatHandler* activeClients[MAX_CLIENTS];

void handle404(HTTPRequest * req, HTTPResponse * res);
void handleRoot(HTTPRequest * req, HTTPResponse * res);

void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>Esp32 - 404 Not Found</h1><p>(from esp) The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");
  res->print("<p>Your server is running for ");
  // A bit of dynamic data: Show the uptime
  res->print((int)(millis()/1000), DEC);
  res->println(" seconds.</p>");
  res->println("</body>");
  res->println("</html>");
}


//  ********* OVER RIDE these 3 functions for WebSocket **********
// In the create function of the handler, we create a new Handler and keep track
// of it using the activeClients array
WebsocketHandler * ChatHandler::create() {
  Serial.println("Creating new chat client!");
  ChatHandler * handler = new ChatHandler();
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == nullptr) {
      activeClients[i] = handler;
      break;
    }
  }
  return handler;
}

// When the websocket is closing, we remove the client from the array
void ChatHandler::onClose() {
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == this) {
      activeClients[i] = nullptr;
    }
  }
}

// Finally, passing messages around. If we receive something, we send it to all
// other clients
void ChatHandler::onMessage(WebsocketInputStreambuf * inbuf) {
  // Get the input message
  std::ostringstream ss;
  std::string msg;
  ss << inbuf;
  msg = ss.str();

  // Send it back to every client
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] != nullptr) {
      activeClients[i]->send(msg, SEND_TYPE_TEXT);
    }
  }
}
// *******************

void handleSPIFFS(HTTPRequest * req, HTTPResponse * res) {
  if (req->getMethod() == "GET") {
    // Redirect /main to /main.html
    std::string reqFile = req->getRequestString()=="/main" ? "/main.html" : req->getRequestString();
    // Serial.println("handle method GET");
    // Try to open the file
    std::string filename = reqFile;
    // Serial.println(reqFile.c_str());
    // Serial.println(SPIFFS.exists(reqFile.c_str()));
    // Check if the file exists
    if (!SPIFFS.exists(filename.c_str())) {
      // Send "404 Not Found" as response, as the file doesn't seem to exist
      res->setStatusCode(404);
      res->setStatusText("Not found");
      res->println("(from SPIFFS handler) - 404 Not Found");
      return;
    }

    File file = SPIFFS.open(filename.c_str());

    // Set length
    res->setHeader("Content-Length", httpsserver::intToString(file.size()));

    // Content-Type is guessed using the definition of the contentTypes-table defined above
    int cTypeIdx = 0;
    do {
      if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
        res->setHeader("Content-Type", contentTypes[cTypeIdx][1]);
        break;
      }
      cTypeIdx+=1;
    } while(strlen(contentTypes[cTypeIdx][0])>0);

    // Read the file and write it to the response
    uint8_t buffer[256];
    size_t length = 0;
    do {
      length = file.read(buffer, 256);
      res->write(buffer, length);
    } while (length > 0);

    file.close();
  } else {
    // If there's any body, discard it
    req->discardRequestBody();
    // Send "405 Method not allowed" as response
    res->setStatusCode(405);
    res->setStatusText("Method not allowed");
    res->println("405 Method not allowed");
  }
}


void setup() {
  // For logging
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  // WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("WIFI-MULTI");
  wifiMulti.addAP (ssid1, password1);
  wifiMulti.addAP (ssid2, password2);
  wifiMulti.addAP (ssid3, password3);
  wifiMulti.run();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("updated");
  Serial.print("Connected.  -----> IP=");
  Serial.println(WiFi.localIP());
  Serial.println("next is AP setup");

  WiFi.softAP("esp32", "girlcat1");
  Serial.print("my apIP = ");
  Serial.println(WiFi.softAPIP());

  if (!MDNS.begin(DOMAINname)) {
      Serial.println("Error setting up MDNS responder!");
      while(1) {
          delay(1000);
          Serial.println("Error setting up MDNS responder!");
      }
  }
  Serial.println("mDNS responder started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("https", "tcp", 443);


  if (!SPIFFS.begin(false)) {
    Serial.println("Mounting SPIFFS failed. Try formatting ... ");
    if (!SPIFFS.begin(true)){
      Serial.println("Formatting failed.  Will STOP from here...");
      while (1);
    }
    Serial.println("Formatting SUCCESS");
  }
  Serial.println("SPIFFS MOUNTED.");

  // ResourceNode * nodeRoot    = new ResourceNode("/main", "GET", &handleRoot);
  // ResourceNode * nodeRoot    = new ResourceNode("", "", &handleRoot);
  ResourceNode * spiffsNode = new ResourceNode("", "", &handleSPIFFS);
  // ResourceNode * node404     = new ResourceNode("", "GET", &handle404);


  // secureServer.registerNode(nodeRoot);
  // secureServer.setDefaultNode(nodeRoot);
  secureServer.setDefaultNode(spiffsNode);
  // secureServer.setDefaultNode(node404);

  
  // ResourceNode * getEventsNode = new ResourceNode("/main", "GET", &handleGetEvents);
  // secureServer.registerNode(getEventsNode);


  // The websocket handler can be linked to the server by using a WebsocketNode:
  // (Note that the standard defines GET as the only allowed method here,
  // so you do not need to pass it explicitly)
  WebsocketNode * chatNode = new WebsocketNode("/chat", &ChatHandler::create);
  // Adding the node to the server works in the same way as for all other nodes
  secureServer.registerNode(chatNode);

  Serial.println("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");
  }



  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();


}

void loop() {
  secureServer.loop();
  ArduinoOTA.handle();
  delay(1);
}


