#include <Arduino.h>
#include <HardwareSerial.h>
#include "pressure_functions.h"
#include "Engine_functions.h"
#include <Servo.h>
#include <SD.h>

const int RS_DE_RE_SLAVE = 10;
int lastSensorReadTime = 0;
const int SENSOR_READ_INTERVAL_MS = 50;

File logFile;
char logFileName[32];
int SD_pin = 4;

int clk_time = 0;

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
    delayMicroseconds(200);

    Serial2.print(data);
    Serial2.print('\n');
    Serial2.flush();
    digitalWrite(RS_DE_RE_SLAVE, LOW);   // back to receive
    delayMicroseconds(200);
    //Serial.println(data);
}


//String firingpinstatus = "";
//String fillpinstatus = "";

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

    pinMode(RS_DE_RE_SLAVE, OUTPUT);
    digitalWrite(RS_DE_RE_SLAVE, LOW);   // default receive
    sendString(String("RS485 Slave ready."));

    //Serial2.flush();
    String PT_setup = SetupCurrentSensor();
    sendString(PT_setup);
    //Serial.println(PT_setup);

    //Inputs and Outputs
    SetupControl(PyroPin, FirePin, FillSequPin);
    


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

}

void loop() {
    
  clk_time = millis();

  if(clk_time - lastSensorReadTime - SENSOR_READ_INTERVAL_MS >= 0){
   lastSensorReadTime  = clk_time;
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
    FillStartTime = clk_time;
  }

  if (digitalRead(FillSequPin) == HIGH && digitalRead(FirePin) == LOW){
      filltime = fillSequence(FillStartTime, clk_time, fillSeq);
      if(filltime%1000 == 0){
        int ft = filltime/1000;
        sendString(String("Fill Time: ")+ String(ft)+String(" s"));
        //delayMicroseconds(500);
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
        int lt = launchtime/1000;
        sendString(String("Launch Time: ")+ String(lt) +String(" s"));
        //sendString(String(launchtime));
        //delayMicroseconds(500);
    }
  }
  

}

   
        
          
              


