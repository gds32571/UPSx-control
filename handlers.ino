//*************************************************
void handleRoot() {
  unsigned int myVal;
  myVal =  (millis() - dmTimer) / 1000;
  if (myVal < dmPeriod) {
    myVal = dmPeriod - myVal;
  } else {
    myVal = 0;
  }

  String message = "This is " + String(WiFi_hostname) + " at " + WiFi.localIP().toString() +" supporting host " 
                   + String(supportedHost) + "\n"
                   + "Running " + String(progname) + " " + String(myVersion) 
                   + " on a " + txtBoard +  "\n"
                   + "Reboots: "
                   + String(reboots) + strReboot + "\n"
                   + "Running for " + String(mySeconds) + " seconds"
                   + "  DM time to go: "
                   + String(myVal) + "\n" // + "  Flag = " + strAIO + " dmTimer " + dmTimer +"\n"
                   + "Battery voltage(avg): "
                   + String(avgVoltage) + " @ " + String(tempF2,1)+ " degrees F\n"
                   + "Supply power is ";
  if (!digitalRead(inPwr)) {
    message += "on\n";
  } else {
    message += "off\n";
  }
  message += "On battery: " + String(onBattery) + " seconds\n";

  message += "Charger is ";
  if (stateCharger) {
    message += "on";
  } else {
    message += "off";
  }

  message += " for " + String(sinceCharge) + " seconds\n";

//  message += "temp = " + ;

  // end message creation

  server.send(200, "text/plain", message);
}

void handleSensor() {

  sensorEnabled = true;

  // show the webserver is being accessed
  digitalWrite(myLed, 1);
  String message = "This is the UPS webserver \n";
  message += "Sensor enabled\n";

  server.send(200, "text/plain", message);
  digitalWrite(myLed, 0);
}

//*************************************************
void handleCon() {                        // If a GET request is made to URI /con
//  digitalWrite(cctl, LOW);                // Turn charger on with a low
  sendATTiny(chgOn);
  stateCharger = true;
  sinceCharge = 0;
  server.sendHeader("Location", "/");     // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                       // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//*************************************************
void handleCoff() {                       // If a GET request is made to URI /coff
//  digitalWrite(cctl, HIGH);               // Turn charger off with a high
  sendATTiny(chgOff);
  stateCharger = false;
  sinceCharge = 0;
  server.sendHeader("Location", "/");     // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                       // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//*************************************************
void handleReboot() {                     // If a GET request is made to URI /reboot
  strAIO = "On";
  server.sendHeader("Location", "/");     // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                       // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//*************************************************
void handleStop() {                     // If a GET request is made to URI /reboot
  strAIO = "On";
  pwroff = true;
  strReboot = (F("  Pausing "));
  server.sendHeader("Location", "/");     // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                       // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//*************************************************
void handlePause() {                      // If a GET request is made to URI /reboot
  //idleTimer = millis() + (5 * 60 * 1000);     //   go idle for 5 minutes
  idleTimer = millis() + (1 * 60 * 60 * 1000); //   go idle for 1 hour
  mystate = WAIT;
  strReboot = (F("  UPS paused"));
  server.sendHeader("Location", "/");     // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                       // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//*************************************************
void handleLEDTest() {
  int tdel = 100;
//  bool chgState = digitalRead(cctl);   // so we can leave charger as we found it

  for (int ii = 0; ii < 5 ; ii++) {
//    digitalWrite(cctl, LOW);
//    delay(tdel);
    digitalWrite(warning, HIGH);
    delay(tdel);
    digitalWrite(myLed, HIGH);
    delay(tdel);
    digitalWrite(batOK, HIGH);
//    digitalWrite(cctl, HIGH);
//    delay(tdel);
    digitalWrite(warning, LOW);
    delay(tdel);
    digitalWrite(myLed, LOW);
    delay(tdel);
    digitalWrite(batOK, LOW);
  }

// resync attiny during LED test  
// 23 Apr 2019
    resetATTiny();
  
  for (int ii = 0; ii < 5 ; ii++) {
    sendATTiny(chgOn);
    sendATTiny(stopOn);
    sendATTiny(chgOff);
    sendATTiny(stopOff);
  }

  // put charger back where it was
  if (stateCharger){
    sendATTiny(chgOn);
  }else{
    sendATTiny(chgOff);
  }  
  
  server.sendHeader("Location", "/help");   // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//*************************************************
void handleFreeheap() {
  long  fh = ESP.getFreeHeap();
  char  fhc[20];

  ltoa(fh, fhc, 10);
  //  String freeHeap = String(fhc);

  String strFree = "This is " + String(WiFi_hostname) + "\n"
                   + "Running " + String(progname) + " " + String(myVersion) + "\n\n"
                   + "Heap " + String(fhc) + "\n";
  server.send(200, "text/plain", strFree);
}

//*************************************************
void handleTest() {
  String message = "<html>This is <h3>" + String(WiFi_hostname) + "</h3><br>"
                   + "Running " + String(progname) + " " + String(myVersion) + "<br><br>"
                   + "<html> Test URLs<br>"
                   + "<a href = ./freeheap>Memory</a><br>"
                   + "<a href = ./help>Help</a><br>"
                   + "<br></html>";

  server.send(200, "text/html", message);
}

//*************************************************

void handleRestart(){
  strReboot = (F("  Restarting ESP in 10 seconds"));
  server.sendHeader("Location", "/");     // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                       // Send it back to the browser with an HTTP status 303 (See Other) to redirect
  delay(10000);
  ESP.restart();
}

//*************************************************
void handleHelp() {                     // If a GET request is made to URI /help

  String message = "<html>This is " + String(WiFi_hostname) + "<br>"
                   + "Running " + String(progname) + " " + String(myVersion) + "<br><br>"
                   + "Add these to base URL:<br>"
                   + "<a href=./sensor>" + "/sensor</a>  enable temperature sensor<br>"
                   + "<a href=./ledtest>" + "/ledtest</a>  blinks status LEDs (but not power LEDS)<br>"
                   + "<a href=./freeheap>" + "/freeheap</a> displays heap space<br>"
                   + "<a href=./con>" + "/con</a>      turns charger on<br>"
                   + "<a href=./coff>" + "/coff</a>     turns charger off<br>"
                   + "<a href=./pause>" + "/pause</a>     pauses watchdog for 1 hour<br>"
                   + "<a href=./reboot>" + "/reboot</a>   to reboot the client computer<br>"
                   + "<a href=./restart>" + "/restart</a>   to restart the ESP controller<br>"
                   + "<a href=./stop>" + "/stop</a>     to halt the client, and then power off the client and the UPS (ESP8266)<br>"
                   + "<a href=./help>" + "/help</a>     to display this page<br></html>";

  server.send(200, "text/html", message);
}

//*************************************************
void handleNotFound() {
  //digitalWrite(led, 1);
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
  //  digitalWrite(led, 0);
}
