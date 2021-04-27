/****************************************************************
  now called
   UPSx-control.ino
   *** now using Wemos D1 Mini Lite in newer versions
   Supports UPS3 circuit card in every location

   13 Mar 2021
   changing name scheme to UPSx vice OTAx

   24 Feb 2021
   Added to github as a proper project UPSx-control
   There is also a UPSx-control.ino which was created by copying files. Don't
   use this one, except for archive purposes.

   22 Feb 2021 - v 1.5f same version
   re-enabled temperature sensor

   21 Feb 2021 - v 1.5f 
   Added code to disable temp sensor in case it is not present.
   Will keep the ESP8266 from booting.
   Need to work on this
   
   19 July 2020 - v 1.5e
   only change is to the  ESP8266WiFi library, now 2.7.2
   This supposrts the Sonoff switches on the "new" network.
   I had too many devices on the WiFi for the NETGEAR26 to support.  So I moved many to the
   old NETGEAR69 router, including Sonoff switches, the UPS controllers, and the garage ESP Easy devices.

   The Sonoff switches would no longer load via OTA.  Updating the esp8266WiFi code to 2.7.2 and 
   reloading the code via serial connection fixed the OTA problem.
   
   8 Jun 2019 - v 1.5d
   added code to support DS18B20 temperature sensor

   23 Apr 2019 - v 1.5c
   using software reset for ATTiny during LED test
   Don't forget to do a LED test at intervals, such as a crontab entry:

  5 1,13 * * * /usr/bin/curl http://ups-control-ota5./ledtest   >/dev/null 2>&1
  6 1,13 * * * /usr/bin/curl http://ups-control-ota6./ledtest   >/dev/null 2>&1
  7 1,13 * * * /usr/bin/curl http://ups-control-ota7./ledtest   >/dev/null 2>&1


   2 Mar 2019 - v 1.5b
   added text string to display ESP board type

   1 Mar 2019 - v 1.5a
   added ESP restart function

   28 Feb 2019 - v 1.5
   added code to disable reboots if reboot count is very high

   27 Feb 2019 - v 1.4g
   added MDNS text for supported host - didn't work!

   15 Feb 2019 - v 1.4f
   Changed pause function to 1 hour vice 5 minutes.

   OLD (on Breadboard) - Use board WeMos D1 R2 & Mini for OTA to work correctly

  was
     UPS-reboot-OTA.ino
     gswann
     16 May 2018

   Runs in Pace at 192.168.1.138 supporting Zero3
   Runs in Villages at 192.168.2.31 supporting Rp5
   Runs in Villages at 192.168.2.61 supporting Zero4

   Now called UPS-reboot-OTA-chg-noAIO and runs on host UPS2 supporting Zero3
   The design uses ATTiny44 and runs on the SwannCo UPS3 circuit board

   Design progression and history:
     UPS monitor and control
     Reads an Adafruit AIO button to determine whether to reboot
     an RPi3 controller.

     When signaled it does these things:
     1. Turns off the website AIO button that says to reboot
     2. Depresses the shutdown button for 7 seconds (with transistor)
     3. Turns off the power to the RPi3 for 3 seconds (with relay)
     4. Turns power back on

     As a webserver, it shows the number of reboots,
     its runtime, the battery voltage, and input power state

     This code version matches the "as-built" hardware (3 June 2018)

  ...reversed dates...

  29 nov 2018 - v 1.4e
    added supportedHost variable

  28 Nov 2018 - v 1.4d
    added voltage read to setup to avoid reporting a 0 battery voltage at startup
    changed shutdown delay to 45 vice 30 seconds

  26 Nov 2018 - v 1.4c
     lowered low battery value to 9.0 (because of RP5 batteries) and reloaded

  19 Nov 2018
     changed program name
     using #define and #ifdef to have ONE file for all hardware
     1.4b - wait until first deadman pulse seen before activating watchdog

  18 Nov 2018
     adapted to run in Villages
     Supporting rp5

  7 Nov 2018 - v 1.3
    Changed charge control to use ATTiny (bread board prototype used ESP output)
    See: ATtiny44_SS_Digitalout.ino
    UPS-control-OTA3 (at 192.168.1.138) supports zero3
    This program would not start up UPS3 card with Wemos D1 Mini

  31 Oct 2018 - v1.2
    added seconds counter for charger on/off time

  8 Oct 2018
    Moved character arrays to program memory

  30 Sept 2018
    Broke down and started working to add deadman functionality. Spent a rainy
    day making it work but I think it is pretty nice.

  29 Sept 2018
     Separated some code into tabs

  26 Sept 2018
   I have tried and tried to accept the rules based version of ESPEasy to do this
   function and now I just give up. It (ESPEasy) just crashes too often when using the rules.
   Or, maybe it's just me.
   Anyway, this may morph into a deadman/watchdog, but for now,
   I'll accept a web-directed reboot.  No AIO functions here unless
   I am ready to spring for an annual subscription. We used up all the feeds.

  5 June 2018
     Modified to allow OTA updating

****************************************************************/
// which host are we compiling for?

// last loaded 29 Nov 2018  v 1.4e
// last loaded 19 Mar 2019  v 1.5b
// last loaded 6 May 2019  v 1.5c - needs new ATTiny programmed
// OTA3 and Zero3 now running in Milton at Jerry's
// loaded November 2019
// #define OTA3

// last loaded 14 Dec 2018 v 1.4e
// last loaded 15 Feb 2019 v 1.4f
// last loaded 28 Feb 2019 v 1.5
// last loaded 1 Mar 2019 v 1.5a
// last loaded 2 Mar 2019 v 1.5b
// last loaded 23 Apr 2019 v 1.5c - ATTiny was reprogrammed
// added DS18B20 temperature sensor
// last loaded 8 Jun 2019 v 1.5d 
#define UPS5

// last loaded 30 Dec 2018 v 1.4e
// last loaded 27 Feb 2019 v 1.4f
// last loaded 1 Mar 2019 v 1.5a
// last loaded 6 Mar 2019 v 1.5b
// last loaded 23 Apr 2019 v 1.5c - ATTiny was reprogrammed
// last loaded 8 Jun 2019 v 1.5d 
// #define OTA6

// built new UPS3v2 board for rp6
// with DS18B20 temperature sensor
// loaded 8 Jun 2019 v 1.5d
// used existing ATTiny that was labelled v2 
// loaded 13 Mar 2021
// #define UPS6

// for testing with rp2 (was ha32571)
// and DC load tester
// loaded 21 Feb 2021 v 1.5f
// loaded 22 Feb 2021 v 1.5f with sensorEnabled = true
// loaded 13 Mar 2021
// #define UPS2

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <OneWire.h>
#include <DallasTemperature.h>

ESP8266WebServer server(80);

#include "secrets.h"

// steps in the shutdown process
#define IDL        0
#define SHUTDOWN1  1
#define SHUTDOWN2  4
#define SHUTDOWN3  2
#define SHUTDOWN4  3
#define WAIT       5

// wait periods
#define INTERVAL1  7000     // 7 seconds
#define INTERVAL2  45000    // sb 45 seconds
#define INTERVAL3  10000    // power off 10 seconds
#define INTERVAL4  75000    // wait 75 seconds for reboot to complete

boolean sensorEnabled = true;  //  false;

// prototype on bread board
// const char* WiFi_hostname = "UPS-control-OTA2";
// ota3 (192.168.1.138) supports zero3
// ota5 supports rp5

const char* progname = "UPSx-control";
const char* myVersion = "v1.5f";

#ifdef OTA3
const char* WiFi_hostname = "UPS-control-OTA3";
const char* supportedHost = "Zero3";
const char* txtBoard = "ESP8266_WeMos_D1_mini_Lite";
const float factor = 3.815;
#endif

#ifdef UPS5
const char* WiFi_hostname = "UPS-control-UPS5";
const char* supportedHost = "RPi5";
const char* txtBoard = "ESP8266_WeMos_D1_mini_Lite";
const float factor = 3.8946;
#endif

#ifdef OTA6
const char* WiFi_hostname = "UPS-control-OTA6";
const char* supportedHost = "zero4";
const char* txtBoard = "ESP8266_WeMos_D1_mini_Lite";
const float factor = 3.8946;
#endif

#ifdef UPS6
const char* WiFi_hostname = "UPS-control-UPS6";
const char* supportedHost = "rp6";
const char* txtBoard = "ESP8266_WeMos_D1_mini_Lite";
const float factor = 3.7514;
#endif

#ifdef UPS2
const char* WiFi_hostname = "UPS-control-UPS2";
const char* supportedHost = "rp2";
const char* txtBoard = "ESP8266_WeMos_D1_mini_Lite";
// swapped 8266 board with the one on ota6
// const float factor = 3.7550;  
const float factor = 3.833;
#endif


//const int led = LED_BUILTIN; // blue LED on the ESP board

const int inDM = D0;       // GPIO16 = deadman from RPi
// const int cctl = D1;       // GPIO5 = D1 charge control signal (inverted)
const int batOK = D2;      // GPIO4 = battery OK LED
const int warning = D5;    // GPIO14 = blue warning LED
const int inPwr   = D6;    // GPIO12 = sense state of input power - also red LED
//const int myTemp = D7;   // GPIO13 = temperature IC
const int myLed = D8;      // GPIO15 = status LED program running active blinker)

// ATTiny command word defines
#define mainOn    "51"   // main power switch
#define mainOff   "50"
#define clientOn  "41"   // client power switch
#define clientOff "40"
#define chgOn     "10"   // inverted
#define chgOff    "11"   // inverted
#define stopOn    "01"   // client halt button
#define stopOff   "00"

// changed after running only 10 minutes after power failure or so on ota5
// to be fair - those batteries were over-discharged in Pace earlier
const float batLow = 9.0;    // 9.5;
// const float batLow = 10.0;  // for testing

void setupWifi(void);
void handleRoot(void);
void handleCon(void);
void handleCoff(void);
void handleReboot(void);
void handlePause(void);
void handleStop(void);
void handleHelp(void);
void handleFreeheap(void);
void handleLEDTest(void);
void handleRestart(void);
void handleTest(void);
void handleNotFound(void);
void setupOTA(void);

unsigned int mystate = WAIT;

String strAIO = "Off";   // now used as flag to signal reboot
bool pwroff = false;     // turn everything off
unsigned int reboots = 0;
const unsigned int maxReboots = 25;

unsigned int sensorValue;
float voltage;
// float factor;
float sumVoltage = 0;
float avgVoltage = 0;

int cntSeconds;
const unsigned int period = 60;
const unsigned int dmPeriod = 120;

unsigned long currentMillis;
unsigned long previousMillis;

const long oneSecond = 1000;

unsigned long stateMillis;
unsigned long dmTimer = 0;

unsigned long idleTimer = 300000;   // was 120000

bool seenDM = false;
bool stateDM = false;
bool stateDMold = false;
bool stateCharger = false;

String strReboot;

unsigned int onBattery = 0;
int batLowctr = 0;

unsigned long sinceCharge = 0;
unsigned long mySeconds = 1;    // fixes discrepancy with initial sinceCharge

float tempF;
float tempF2;


//***************************************************
// The hardware UARTs share the same baud rate generator !!
void sendATTiny(char *SerOut) {
  Serial1.begin(300);
  Serial1.println(SerOut);
  delay(500);
  Serial.begin(115200);
}

// to support software reset
void resetATTiny() {
  Serial1.begin(300);
  for (int ii = 0 ; ii < 3; ii++) {
    Serial1.print("r");
    delay(1000);
  }
  Serial.begin(115200);
}

// Data wire is plugged into pin D7 on the ESP8266 - GPIO 13
#define ONE_WIRE_BUS 13

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);
//char temperatureCString[7];
//char temperatureFString[7];

//***************************************************
void setup() {

  pinMode(inPwr, INPUT_PULLUP);
  pinMode(inDM, INPUT);    // INPUT_PULLDOWN_16);

  // setup the LEDs
  pinMode(batOK, OUTPUT);
  //  pinMode(cctl, OUTPUT);
  pinMode(warning, OUTPUT);
  pinMode(myLed, OUTPUT);

  digitalWrite(batOK, HIGH);    // green  - on    = battery ok   blink = battery low
  //  digitalWrite(cctl, HIGH);     // yellow - on    = charger off
  digitalWrite(warning, HIGH);  // blue   - on    = reboot or shutdown in progress
  digitalWrite(myLed, HIGH);    // red    - blink = program running
  //   delay(1000);

  // Serial1 goes to ATTiny44 as power control
  Serial1.begin(300);
  delay(100);
  sendATTiny(".");       // init
  sendATTiny(mainOn);    // me on
  sendATTiny(clientOn);  // RPi on
  sendATTiny(stopOff);   // RPi reset off
  delay(2000);
  sendATTiny(stopOn);    // RPi reset on
  delay(1500);
  sendATTiny(stopOff);   // RPi reset off
  sendATTiny(chgOff);   // RPi reset off

  Serial.begin(115200);
  Serial.println();
  Serial.println("Program " + String(progname) + " starting...");
  Serial.println();


  setupWifi();

  server.on("/", handleRoot);
  server.on("/sensor", handleSensor);
  server.on("/con", handleCon);
  server.on("/coff", handleCoff);
  server.on("/reboot", handleReboot);
  server.on("/stop", handleStop);
  server.on("/pause", handlePause);
  server.on("/freeheap", handleFreeheap);
  server.on("/ledtest", handleLEDTest);
  server.on("/restart", handleRestart);
  server.on("/test", handleTest);
  server.on("/help", handleHelp);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println(F("HTTP server started"));

  setupOTA();

  Serial.println(F("Program startup complete"));
  Serial.println(F("ATTiny init complete"));

  Serial.println();
  Serial.println();

  digitalWrite(batOK, LOW);
  //digitalWrite(cctl, HIGH);
  digitalWrite(warning, LOW);
  digitalWrite(myLed, LOW);

  sensorValue = analogRead(A0);

  voltage = ((sensorValue * (factor * 3.36)) / 1024.0);

  // IC Default 9 bit. If you have troubles consider upping it 12.
  // Ups the delay giving the IC more time to process the temperature measurement
  DS18B20.begin();
  avgVoltage = voltage;

  stateDM = digitalRead(inDM);
  stateDMold = stateDM;
  strReboot = (F(" waiting for initial boot"));
  currentMillis = millis();

  getTemperature();

}

void getTemperature() {
  float tempC;
  //  float tempF;

  if (!sensorEnabled) {
    return;
  }
  
  do {
    DS18B20.requestTemperatures();
    tempC = DS18B20.getTempCByIndex(0);
    tempF = DS18B20.getTempFByIndex(0);
    delay(100);
  } while (tempC == 85.0 || tempC == (-127.0));

  tempF2 = (tempF * 10) + 0.5 ;
  int inttempF = (int)tempF2;
  tempF2 = (float) inttempF / 10;

}

//***************************************************
void loop() {

  // wait five (was two) minutes after initial bootup before starting WD monitor
  // also allow UPS pause from web server
  if ( (mystate == WAIT) && (seenDM  == true) && (millis() > idleTimer) ) {
    mystate = IDL;
    strReboot = "";
  }

  ArduinoOTA.handle();
  server.handleClient();

  currentMillis = millis();

  // every second
  if (currentMillis - previousMillis >= oneSecond) {

    ArduinoOTA.handle();
    server.handleClient();
    previousMillis = previousMillis + oneSecond;
    sensorValue = analogRead(A0) + analogRead(A0) + analogRead(A0) + analogRead(A0);
    sensorValue = sensorValue / 4;
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V):
    // scale with a 1 Mohm resistor in series with A0
    // voltage = ((3.2 * sensorValue * (((110+980)/110)) ) / 1023.0);
    // 3.815 for ota3   3.8946 for ota5

    voltage = ((sensorValue * (factor * 3.36)) / 1024.0);

    sumVoltage += voltage;
    cntSeconds += 1;

    // is wall power off?
    if (digitalRead(inPwr)) {
      onBattery = onBattery + 1;
    }

    // every 60 seconds
    if (cntSeconds == period) {
      avgVoltage = sumVoltage / period;
      if (mystate == IDL) {
        Serial.print("Voltage is ");
        Serial.println(voltage);
      }
      sumVoltage = 0;
      cntSeconds = 0;

      // blink the stop LED every minute but not if STOPping the CPU (in shutdown phase 1) or starting up
      if ((mystate != SHUTDOWN1) && (mystate != WAIT)) {
        digitalWrite(warning, HIGH);
        sendATTiny(stopOn);  // RPi reset on
        delay(500);
        digitalWrite(warning, LOW);
        sendATTiny(stopOff);  // RPi reset off
      }

      getTemperature();

    } // every 60 seconds

    // the battery LED is solid in normal operation if wall
    // power is on. When battery voltage <= batLow,
    // it blinks

    // every 5 seconds
    if ((cntSeconds % 5) == 0) {
      if (voltage <= batLow) {
        // toggle the LED, should go on or off once per second if battery voltage is low
        digitalWrite(batOK, !digitalRead(batOK));
        //sendATTiny("10");
      } else {
        digitalWrite(batOK, HIGH);
        //sendATTiny("11");
        batLowctr = 0;
      }
    }

    // toggle the red activity LED
    digitalWrite(myLed, !digitalRead(myLed));

    // if wallpower is off and battery is low, start countdown for shutdown
    if ((digitalRead(inPwr)) && (voltage <= batLow)) {
      batLowctr += 1;
      if (batLowctr >= 120) {
        strAIO = "On";
        pwroff = true;
      }
    }

    //***************************************************

    // do not start watchdog until deadman signal has been seen
    if (mystate == WAIT) {
      bool mystateDM = digitalRead(inDM);
      if (mystateDM == true) {
        seenDM = true;
      }
    }

    if (mystate == IDL ) {
      stateDM = digitalRead(inDM);
      if (stateDM != stateDMold) {
        // we see the DM signal change
        stateDMold = stateDM;
        dmTimer = millis() ;
      } else {
        if (((millis() - dmTimer) > (dmPeriod * 1000) ) && mystate == IDL ) {
          // 120 seconds without a signal, so reboot
          strAIO = "On";
          dmTimer = millis() + (dmPeriod * 3000);  // allow for 6 minutes to reboot
        }
      }
    } else if (mystate == WAIT) {
      //strReboot = (F(" waiting for initial boot"));
      dmTimer = millis();
    } else {
      // not IDL or WAIT - we are rebooting
      // so increment dmTimer to avoid a problem
      dmTimer = millis();
    }
    //***************************************************

    // how long (seconds) since charger turned on or off
    sinceCharge += 1;
    // runtime in seconds
    mySeconds += 1;

  }  // every second

  /////
  server.handleClient();
  /////

  // 28 Feb 2019
  if (reboots >= maxReboots) {
    Serial.println(F("*** Max reboots exceeded!"));
    strReboot = (F("    Max reboots exceeded"));
  }

  if (strAIO.equals("On") and (reboots < maxReboots) and (mystate == IDL || mystate == WAIT)) {
    mystate = SHUTDOWN1 ;
    stateMillis = millis();
    Serial.println(F("*** Start shutdown sequence!"));
    reboots += 1;
    strReboot = (F("    Stopping processor"));
    Serial.println(strReboot);
    digitalWrite(warning, HIGH);  // warning
    sendATTiny(stopOn);
  }
  // shutdown code

  currentMillis = millis();

  if ((mystate == SHUTDOWN1) and (currentMillis - stateMillis >= INTERVAL1)) { // 7 seconds
    mystate = SHUTDOWN2 ;
    stateMillis = millis();
    //    digitalWrite(warning, LOW);  // ATTiny off
    sendATTiny(stopOff);

    strReboot = (F("    Waiting for shutdown"));
    Serial.println(strReboot);
  }

  currentMillis = millis();

  if ((mystate == SHUTDOWN2) and (currentMillis - stateMillis >= INTERVAL2)) { // 20 seconds
    strAIO = "Off";
    mystate = SHUTDOWN3 ;
    stateMillis = millis();
    strReboot = (F("    Turning off power"));
    Serial.println(strReboot);
    //    digitalWrite(outPwr, HIGH);  // relay on, opens power
    sendATTiny(clientOff);   // turn off RPi
    if (pwroff == true) {
      pwroff = false;
      delay(2000);
      Serial.println(F("    Turning me off"));
      sendATTiny(mainOff);  // turn me off
      delay(5000);       // should never get past here
      ESP.restart();
    }
  }

  currentMillis = millis();

  if ((mystate == SHUTDOWN3) and (currentMillis - stateMillis >= INTERVAL3) ) {   // 5 seconds
    mystate = SHUTDOWN4 ;
    stateMillis = millis();
    Serial.println(F("    Turning power on"));
    //digitalWrite(outPwr, LOW); // relay off, power to RPi on
    sendATTiny(clientOn);
    strReboot = (F("    Power on, waiting for reboot to finish "));
    Serial.println(strReboot);
  }

  currentMillis = millis();

  if ((mystate == SHUTDOWN4) and (currentMillis - stateMillis >= INTERVAL4) ) {   // 60 seconds
    mystate = IDL ;
    stateMillis = millis();
    dmTimer = millis();
    Serial.println(F("    Reboot complete"));
    strReboot = "";
    strAIO = "Off";
    digitalWrite(warning, LOW);  // warning
  }

}  // end loop

//*************************************************
void setupWifi(){

  WiFi.disconnect();  //Prevent connecting to wifi based on previous configuration

  IPAddress staticIP(192, 168, 2, MYADDR); //ESP static ip
  IPAddress dns(192, 168, 2, 6);           //DNS
  IPAddress gateway(192, 168, 2, 1);       //IP Address of your WiFi Router (Gateway)
  IPAddress subnet(255, 255, 255, 0);      //Subnet mask

  WiFi.mode(WIFI_STA);
  WiFi.hostname(WiFi_hostname);

  WiFi.config(staticIP, dns, gateway, subnet);
  WiFi.begin(SSID, SSIDPWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(WiFi_hostname)) {
    Serial.println(F("MDNS responder started"));
    MDNS.addService("http", "tcp", 80);
    //    MDNS.addServiceTxt("arduino","tcp","supports",supportedHost);
  }

}

//*************************************************
void setupOTA() {

  ArduinoOTA.setHostname(WiFi_hostname);
  ArduinoOTA.onStart([]() {
    digitalWrite(warning, HIGH);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    digitalWrite(warning, !digitalRead(warning));
  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end

    for (int i = 0; i < 10; i++)
    {
      digitalWrite(myLed, HIGH);
      digitalWrite(warning, HIGH);
      delay(100);
      digitalWrite(myLed, LOW);
      digitalWrite(warning, LOW);
      delay(100);
    }

    digitalWrite(myLed, LOW);   // turn off blue LED

    // had to add on this computer to make OTA finish properly
    ESP.restart();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });

  // setup the OTA server
  ArduinoOTA.begin();

}
