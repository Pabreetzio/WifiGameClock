#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <TM1637Display.h>


#ifndef STASSID
#define STASSID "WifiGameClock"
#define STAPSK  "chronometro"
#endif


const int CLK = 4; //Set the CLK pin connection to the display
const int DIO = 5; //Set the DIO pin connection to the display
const int buttonPin = 2; 
const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
WebSocketsClient webSocketClient;
const int led = 13;
int TimeLeft = 0; //in millis
int LastLoop = 0;
bool IsRunning = false;
String nextClockIP = "";
const int wifiTimeout = 20000; //millis
TM1637Display display(CLK, DIO);  //set up the 4-Digit Display.
uint8_t previousClock = 0x00;
String lastClockIP = "";
String myIPAddress = "";
void handleRoot() {
  digitalWrite(led, 1);
  char temp[500];
  int minutes = 0;
  int seconds = 0;
  if(server.arg("minutes") != ""){
    minutes = server.arg("minutes").toInt();
  }
  if(server.arg("seconds") != ""){ 
    seconds = server.arg("seconds").toInt();
  }
  if(server.arg("nextClockIP") != ""){
    nextClockIP = server.arg("nextClockIP");
    HTTPClient http;
    http.begin("http://" + nextClockIP + "/?lastClockIP=" + myIPAddress  + "&minutes=" + minutes + "&seconds="+seconds);
    http.GET();
    webSocketClient.begin(nextClockIP, 81, "/");
    webSocketClient.onEvent(webSocketClientEvent);
    
  }
  if(server.arg("lastClockIP") != ""){
    nextClockIP = server.arg("lastClockIP");
  }
  if(minutes != 0 || seconds != 0){
    IsRunning = false;
    LastLoop = 0;
    TimeLeft = (minutes * 60 + seconds) * 1000;
    showTimeLeft();
  }
  
  snprintf(temp, 500,"<html><head><title>ESP8266 Demo</title><style>body { font-family: Arial, Helvetica, Sans-Serif; Color: #000008; }</style></head>  <body>  <h1>Game Clock Setup</h1>  <form id=\"clock-setup\" action=\"\" method=\"get\">\
  <label>Other Clock\'s IP Address</label><input name=\"nextClockIP\" type=\"text\" /><br /><label>Minutes</label> <input name=\"minutes\" type=\"number\" min=\"0\" max=\"99\" />    <label>Seconds</label> <input name=\"seconds\" type=\"number\" min=\"0\" max=\"59\" />    <button type=\"submit\">Start</button>  </form>  </body></html>");
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  
  display.setBrightness(0x0a);  //set the diplay to maximum brightness
  pinMode(led, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  IPAddress myIP;
  while (WiFi.status() != WL_CONNECTED && millis() < wifiTimeout ) {
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED){
    
      WiFi.softAP("ESPap", password);
      myIP = WiFi.softAPIP();
  } 
  else{
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    myIP = WiFi.localIP();
  }
    startWebSocket();
    Serial.print("IP address: ");
    Serial.println(myIP);
    myIPAddress = myIP.toString();
    //would be nice to figure out how to display ip on 4 digit display
    display.showNumberDec(0);
  
  
  if (MDNS.begin("patricksesp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/go", StartClock);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void StartClock(){
    LastLoop = millis();
    IsRunning = true;
}
void showTimeLeft(){
  int secondsLeft = (TimeLeft)/ 1000;
  int numberToDisplay = 100 * (secondsLeft / 60) + secondsLeft % 60;
  display.showNumberDecEx(numberToDisplay, 0b01000000, true);
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
        Serial.printf("[WSc] Connected to url: %s\n", payload);
      }
      break;
    case WStype_TEXT:{
      Serial.printf("[WSc] get text: %s\n", payload);
              LastLoop = millis();
        IsRunning = true;
      }
      break;
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        previousClock = num;
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (strncmp((const char*)payload, "StartTimer", 10) == 0) {
        StartClock();
      }
      break;
  }
}

void loop(void) {
  server.handleClient();
  if(nextClockIP != ""){
    webSocketClient.loop();
  }
  else{
    webSocket.loop();
  }
  MDNS.update();
  
  if(digitalRead(buttonPin) == HIGH) //todo: add debounce
  {
    if(nextClockIP != ""){
      Serial.println("nextClockIP = " + nextClockIP);
          IsRunning = false;
          HTTPClient http;
          http.begin("http://" + nextClockIP + "/go");
          http.GET();
        //webSocketClient.sendTXT("StartTimer");
    }
    else if(previousClock != 0x00){
      if(IsRunning == true){
        IsRunning = false;
        webSocket.sendTXT(previousClock, "StartTimer");
      }
    }
    else{
      Serial.printf("Button Press");
      IsRunning = false;
    }
    //todo: send message to nextClock websocket
  }
  if(TimeLeft > 0 and IsRunning == true){
    int thisLoop = millis();
    if(LastLoop == 0){
      LastLoop = thisLoop;
    }
    TimeLeft = TimeLeft - (thisLoop - LastLoop);
    LastLoop = thisLoop;
    showTimeLeft();
  
  }
  
  
}
