#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define PRINT(var) Serial.print(#var ":"); Serial.print(var);
#define PRINTLN(var) Serial.print(#var ":"); Serial.println(var)

/**
 * Since we would be using two different power supply for relay and nodemcu, vcc(relay) and gpio pins will be used to calculate the relay signals from nodemcu.
 * This is why the off state of relay is set to HIGH, and on state of relay is set to LOW. If theres a need to use only one power source polarity can be simply reversed in 
 * `setNeutralPins(), setGoForwardPins(), setGoBackwardPins(), setGoLeftPins(), setGoRightPins()
 */
/*Put your SSID & Password*/
const char* ssid = "Wifi";  // Enter SSID here
const char* password = "WifiPass123";  //Enter Password here

ESP8266WebServer server(80);

uint8_t firstRelayPin;
uint8_t secondRelayPin;
uint8_t thirdRelayPin;
uint8_t fourthRelayPin;

bool firstRelayStatus;
bool secondRelayStatus;
bool thirdRelayStatus;
bool fourthRelayStatus;

bool reversedPolarity = false;

typedef enum {Forward, Backward, Left, Right, Stop} Operation;

void setup() {
  Serial.begin(115200);
  delay(100);
  connectWifi();
  setupPins();
  startServer();
}

void loop() {
  server.handleClient();
  writeAllPinsByStatus();
}

void setupPins() {
  associateHardwarePins(false);
  setNeutralPins();
  pinMode(firstRelayPin, OUTPUT);
  pinMode(secondRelayPin, OUTPUT);
  pinMode(thirdRelayPin, OUTPUT);
  pinMode(fourthRelayPin, OUTPUT);
  printPinsWithMessage("pins_setup");
}

void associateHardwarePins(bool reverse) {
  if (reverse == false) {
    firstRelayPin = D1;
    secondRelayPin = D2;
    thirdRelayPin = D5;
    fourthRelayPin = D6;
  } else {
    firstRelayPin = D5;
    secondRelayPin = D6;
    thirdRelayPin = D1;
    fourthRelayPin = D2;
  }
  
}

void connectWifi() {
  Serial.print("Connecing to ");
  Serial.println(ssid);
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
}

void startServer() {
  server.on("/", goNeutral);
  server.on("/forward", goForward);
  server.on("/backward", goBackward);
  server.on("/left", goLeft);
  server.on("/right", goRight);
  server.on("/reversePolarity", reversePolarity);
  server.onNotFound(handle404);
  server.begin();
  Serial.println("HTTP server started");
}

void writeAllPinsByStatus() {
  digitalWrite(firstRelayPin, firstRelayStatus);
  digitalWrite(secondRelayPin, secondRelayStatus);
  digitalWrite(thirdRelayPin, thirdRelayStatus);
  digitalWrite(fourthRelayPin, fourthRelayStatus);
}

void goNeutral() {
  setNeutralPins();
  printPinsWithMessage("going_neutral");
  server.send(200, "text/html", renderHtml(false, false, false, false, true));
}

void goForward() {
  setGoForwardPins();
  printPinsWithMessage("going_forward");
  server.send(200, "text/html", renderHtml(true, false, false, false, false));
}

void goBackward() {
  setGoBackwardPins();
  printPinsWithMessage("going_backward");
  server.send(200, "text/html", renderHtml(false, true, false, false, false));
}

void goLeft() {
  setGoLeftPins();
  printPinsWithMessage("going_left");
  server.send(200, "text/html", renderHtml(false, false, true, false, false));
}

void goRight() {
  setGoRightPins();
  printPinsWithMessage("going_right");
  server.send(200, "text/html", renderHtml(false, false, false, true, false));
}

void reversePolarity() {
  reversePinsPolarity();
  printPinsWithMessage("reversing_polarity");
  server.send(200, "text/html", renderHtml(false, false, false, false, true));
}

void handle404(){
  server.send(404, "text/plain", "Not found");
}

void setNeutralPins() {
  firstRelayStatus = HIGH;
  secondRelayStatus = HIGH;
  thirdRelayStatus = HIGH;
  fourthRelayStatus = HIGH;
}

void setGoForwardPins() {
  firstRelayStatus = LOW;
  secondRelayStatus = HIGH;
  thirdRelayStatus = HIGH;
  fourthRelayStatus = LOW;
}

void setGoBackwardPins() {
  firstRelayStatus = HIGH;
  secondRelayStatus = LOW;
  thirdRelayStatus = LOW;
  fourthRelayStatus = HIGH;
}

void setGoLeftPins() {
  firstRelayStatus = LOW;
  secondRelayStatus = HIGH;
  thirdRelayStatus = HIGH;
  fourthRelayStatus = HIGH;
}

void setGoRightPins() {
  firstRelayStatus = HIGH;
  secondRelayStatus = HIGH;
  thirdRelayStatus = HIGH;
  fourthRelayStatus = LOW;
}

void reversePinsPolarity() {
  setNeutralPins();
  if (reversedPolarity == false) {
    associateHardwarePins(true);
  } else {
     associateHardwarePins(false);
  }
  reversedPolarity = !reversedPolarity;
}

void printPinsWithMessage(char* msg) {
   Serial.print("operation:");
   Serial.print(msg);
   printRelayPins("firstRelay", firstRelayPin, firstRelayStatus);
   printRelayPins("secondRelay", secondRelayPin, secondRelayStatus);
   printRelayPins("thirdRelay", thirdRelayPin, thirdRelayStatus);
   printRelayPins("fourthRelay", fourthRelayPin, fourthRelayStatus);
   Serial.println("");
}

void printRelayPins(char * relayMsg, uint8_t relayPin, bool relayStatus) {
  Serial.print(", ");
  Serial.print(relayMsg);
  Serial.print(":");
  Serial.print(relayPin);
  Serial.print("|");
  Serial.print(relayStatus);
}

String renderHtml(boolean isForwardDisabled, boolean isBackwardDisabled, boolean isLeftDisabled, boolean isRightDisabled, boolean isStopDisabled){
  String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  html = html + "<title>RC car</title><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} body{margin-top: 50px;}";
  html = html + ".button { box-shadow:inset 0px -3px 7px 0px #29bbff; background:linear-gradient(to bottom, #2dabf9 5%, #0688fa 100%);background-color:#2dabf9;border-radius:10px;border:1px solid #0b0e07;display:inline-block;cursor:pointer;color:#ffffff;font-family:Arial;font-size:35px;padding:17px 34px;text-decoration:none;text-shadow:0px 1px 0px #263666;&:hover {background:linear-gradient(to bottom, #0688fa 5%, #2dabf9 100%);background-color:#0688fa;} &:active {position:relative;top:1px;} } ";
  html = html + ".button-dsiabled { background: unset; background-color:#1d3778;}";
  html = html + "</style></head><body>";
  html = html + "<p>" + "<a href=\"/reversePolarity\" class=\"button\">Reverse Polarity</a> ";
  html = html + "<p>" + renderButton(Forward, isForwardDisabled);
  html = html + "<p>" + renderButton(Left, isLeftDisabled);
  html = html + renderButton(Stop, isStopDisabled);
  html = html + renderButton(Right, isRightDisabled);
  html = html + "<p>" + renderButton(Backward, isBackwardDisabled);
  html = html + "</body></html>";
 return html;
}

String renderButton(Operation operation, bool disabled) {
  if (disabled) {
    switch(operation) {
      case Forward:
       return "<a class=\"button button-dsiabled\">&uarr;</a> ";
      case Left:
       return "<a class=\"button button-dsiabled\">&larr;</a> ";
      case Stop:
       return "<a class=\"button button-dsiabled\">&FilledSmallSquare;</a> ";
      case Right:
       return "<a class=\"button button-dsiabled\">&rarr;</a> ";
      case Backward:
       return "<a class=\"button button-dsiabled\">&darr;</a> ";
    }
  } else {
    switch(operation) {
      case Forward:
       return "<a href=\"/forward\" class=\"button\">&uarr;</a> ";
      case Left:
       return "<a href=\"/left\" class=\"button\">&larr;</a> ";
      case Stop:
       return "<a href=\"/\" class=\"button\">&FilledSmallSquare;</a> ";
      case Right:
       return "<a href=\"/right\" class=\"button\">&rarr;</a> ";
      case Backward:
       return "<a href=\"/backward\" class=\"button\">&darr;</a> ";
    }
  }
}
