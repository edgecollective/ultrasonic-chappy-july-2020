// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

// the onewire code uses the recommended libraries here: https://lastminuteengineers.com/ds18b20-arduino-tutorial/


#define deviceName "wnf_remote_101"
#define devEUI "101"

#include <SPI.h>
#include <RH_RF95.h>

#include <Wire.h>

#include "RTCZero.h" // https://github.com/arduino-libraries/RTCZero
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

#define VBATPIN A7
#define LED 13
#define POWERSWITCH 6
#define INTERVALSWITCH 10

const unsigned SHORT_INTERVAL = 10; // 10 second interval
const unsigned LONG_INTERVAL = 300; // 5 minute interval

int TX_INTERVAL = LONG_INTERVAL; // default short interval

//const unsigned TX_INTERVAL = 300; //default is 30 minutes

// for feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define numSamples 5
#define sampleInterval 2 // seconds

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

#define RTC_SLEEP 1

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

RTCZero rtc;

StaticJsonDocument<200> doc;

double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}





void setup() 
{

   //analogReadResolution(12);
  pinMode(POWERSWITCH, OUTPUT);
    digitalWrite(POWERSWITCH, HIGH);


   pinMode(LED, OUTPUT);
  

if(RTC_SLEEP) {
      // Initialize RTC
    rtc.begin();
    // Use RTC as a second timer instead of calendar
    rtc.setEpoch(0);
}




/*
    if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");
*/

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);

  /*
  while (!Serial) {
    delay(1);
  }
  */
  
  

  delay(100);

  Serial.println("Feather LoRa TX Test!");

  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission



void loop()
{


pinMode(LED, OUTPUT);

int sleepmode = digitalRead(INTERVALSWITCH);
Serial.print("sleep button:");
Serial.println(sleepmode);

/*
if(sleepmode) {

TX_INTERVAL = LONG_INTERVAL;

}
else {

TX_INTERVAL = SHORT_INTERVAL;

}
*/

  Serial.print("sleep interval = ");
  Serial.print(TX_INTERVAL);
  Serial.println(" seconds");
  
  // battery measurement
   float measuredvbat = analogRead(VBATPIN);
measuredvbat *= 2;    // we divided by 2, so multiply back
measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
measuredvbat /= 1024; // convert to voltage

    digitalWrite(POWERSWITCH, LOW);

//delay(3000);

float sensorValue = 0;

for (int s=0;s<numSamples;s++) {
  
sensorValue += float(analogRead(A0));
delay(sampleInterval*1000);
Serial.println('sample');
}
sensorValue=sensorValue/float(numSamples);

//float voltage = sensorValue*3.3/4096;
//float distance = voltage/.002*2; // in cm


    digitalWrite(POWERSWITCH, HIGH);

doc["devEUI"] = devEUI;
doc["deviceName"]=deviceName;
doc["ultrasonic"]=sensorValue;
doc["BatV"] = round2(measuredvbat);

  
  delay(1000); // Wait 1 second between transmits, could also 'sleep' here!
  Serial.println("Transmitting..."); // Send a message to rf95_server
  
  char radiopacket[140] = "Hello World #      ";

  serializeJson(doc,radiopacket,140);
  
  //itoa(packetnum++, radiopacket+13, 10);
  Serial.print("Sending "); Serial.println(radiopacket);
  //radiopacket[19] = 0;
  
  Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)radiopacket, 140);

  Serial.println("Waiting for packet to complete..."); 
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply

  rf95.sleep();

digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);               // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(100);               // wait for a second
  
digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);               // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(100);               // wait for a second


//delay(2000); // to make sure we wait before sleeping

  if(RTC_SLEEP) {

/*
pinMode(0,INPUT_PULLUP);
    pinMode(1,INPUT_PULLUP);
    pinMode(A0,INPUT_PULLUP);
    pinMode(A1,INPUT_PULLUP);
    pinMode(A2,INPUT_PULLUP);
    pinMode(A3,INPUT_PULLUP);
    pinMode(A4,INPUT_PULLUP);
    pinMode(A5,INPUT_PULLUP);
    pinMode(0,INPUT_PULLUP);
    pinMode(1,INPUT_PULLUP);
    pinMode(5,INPUT_PULLUP);
    pinMode(9,INPUT_PULLUP);
    pinMode(10,INPUT_PULLUP);
    pinMode(11,INPUT_PULLUP);
    pinMode(12,INPUT_PULLUP);
    */
    //pinMode(13,INPUT_PULLUP);
    
            // Sleep for a period of TX_INTERVAL using single shot alarm
            rtc.setAlarmEpoch(rtc.getEpoch() + TX_INTERVAL);
            rtc.enableAlarm(rtc.MATCH_YYMMDDHHMMSS);
            rtc.attachInterrupt(alarmMatch);
            
            // USB port consumes extra current
            USBDevice.detach();
           
            // Enter sleep mode
            rtc.standbyMode();
            
            
            // Reinitialize USB for debugging
            USBDevice.init();
            USBDevice.attach();
            }
            else {

              delay(TX_INTERVAL*1000); // delay regular way if not sleeping
            }
  
}

void alarmMatch()
{

}
