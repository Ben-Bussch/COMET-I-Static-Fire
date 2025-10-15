#include <Arduino.h>
#include <HardwareSerial.h>
#include "pressure_functions.h"
#include "Engine_functions.h"
#include <Servo.h>
#include <SD.h>

const int RS_DE_RE_SLAVE = 10;
unsigned long lastSensorReadTime = 0;
const unsigned long SENSOR_READ_INTERVAL_MS = 50;

File logFile;
char logFileName[32];
int SD_pin = 4;

// Fire/Fill sequence variables
int PyroPin = 21;
int FillSequPin = 36;
int FirePin = 34;

int fireSeq = 0;
int fillSeq = 0;

int FireStartTime = 0;
int FillStartTime = 0;
int filltime = 0;
int launchtime = 0;


float Nox_pressure = 0;
float IPA_pressure = 0;



// RS485 transmit helper
void sendString(const String& data) {
    digitalWrite(RS_DE_RE_SLAVE, HIGH);  // switch to transmit
    delayMicroseconds(100);

    Serial2.print(data);
    Serial2.print('\n');
    Serial2.flush();

    delayMicroseconds(100);
    digitalWrite(RS_DE_RE_SLAVE, LOW);   // back to receive
    //Serial.println(data);
}

int clk_time = 0;


String firingpinstatus = "";
String fillpinstatus = "";

//testing :)
//--------------- My good'ol rs485 -------------------------

void setup() {
    // turn the LED on (HIGH is the voltage level)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    //Serial.begin(9600);
    Serial2.begin(9600);
    //while (!Serial && millis() < 5000) {}
    //Serial.begin(9600); 
    //Serial.println("Hello from Teensy 4.1!");
    while (!Serial2 && millis() < 5000) {}

    //Serial2.flush();
    String PT_setup = SetupCurrentSensor();
    
    //Serial.println(PT_setup);

    pinMode(RS_DE_RE_SLAVE, OUTPUT);
    digitalWrite(RS_DE_RE_SLAVE, LOW);   // default receive

/* 
    if (!SD.begin(SD_pin)) {
  Serial.println("SD card initialization failed!");
} else {
  
  snprintf(logFileName, sizeof(logFileName), "pressure_%lu.csv", millis());
  
  logFile = SD.open(logFileName, FILE_WRITE);
  if (logFile) {
    logFile.println("Time(ms),Nox_Pressure(psi),IPA_Pressure(psi)");
    logFile.close();
    
  } 
  
}*/
    //Serial.println("RS485 Slave ready.");
    SetupControl(PyroPin, FirePin, FillSequPin);
}

void loop() {
    
  clk_time = millis();

  if(clk_time%50 == 0){
   Nox_pressure = ReadPressureTransducer(1); //1 is Nox, 2 is IPA line 
   IPA_pressure = ReadPressureTransducer(2); //1 is Nox, 2 is IPA line 
   sendString(String(Nox_pressure,3) + ","+String(IPA_pressure,3)+","+String(clk_time));

  }
/*
  logFile = SD.open(logFileName, FILE_WRITE);
if (logFile) {
  logFile.print(millis());
  logFile.print(",");
  logFile.print(Nox_pressure, 3);
  logFile.print(",");
  logFile.println(IPA_pressure, 3);
  logFile.close();
} */



  if (digitalRead(FillSequPin) == LOW && digitalRead(FirePin) == LOW ){
    Rest();
    fillSeq = 0;
  }

  if (digitalRead(FillSequPin) == HIGH && digitalRead(FirePin) == LOW){
      filltime = fillSequence(FillStartTime, clk_time, fillSeq);
      if(filltime%10000 == 0){
        sendString(String("Fill Time: ")+ String(filltime/1000, 0)+String(" s"));
    }
  }

  
  if (digitalRead(FirePin) == LOW){
    //Serial.println("Fire is LOW");
    digitalWrite(PyroPin, LOW);
    fireSeq = 0;
    FireStartTime = clk_time;
  }

  if (digitalRead(FirePin) == HIGH && digitalRead(FillSequPin) == HIGH){
    //Serial.println("Fire is HIGH");
    
    fillSeq = 0; //get out of fill sequence


    launchtime = fireSequence(FireStartTime, clk_time, fireSeq, PyroPin);
    if(launchtime%1000 == 0){
        sendString(String("Launch Time: ")+ String(launchtime/1000, 0) +String(" s"));
    }
  }
  

}

   
        
          
              


