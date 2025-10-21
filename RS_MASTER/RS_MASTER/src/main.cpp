#include <Arduino.h>
#include <HardwareSerial.h>
#include "pressure_functions.h"
#include "Engine_functions.h"
#include <Servo.h>
#include <SD.h>

const int RS_DE_RE_SLAVE = 10;
unsigned long lastSensorReadTime = 0;
const int SENSOR_READ_INTERVAL_MS = 100;

unsigned long lastdisplay  = 0;
int DISPLAY_INTERVAL_MS  = 1000;

File logFile;
char logFileName[32];




// Fire/Fill sequence variables
int PyroPin = 21;
int FillSequPin = 36;
int FirePin = 34;

int fireSeq = 0;
int fillSeq = 0;

unsigned long clk_time = 0;

unsigned long FireStartTime = 0;
unsigned long FillStartTime = 0;
unsigned long AbortStartTime = 0;

unsigned long filltime = 0;
unsigned long launchtime = 0;
unsigned long aborttime = 0;


float Nox_pressure = 0;
float IPA_pressure = 0;




// RS485 transmit helper
void sendString(const String& data) {
    digitalWrite(RS_DE_RE_SLAVE, HIGH);  // switch to transmit
    delayMicroseconds(20);

    Serial2.print(data);
    Serial2.print('\n');
    Serial2.flush();
    digitalWrite(RS_DE_RE_SLAVE, LOW);   // back to receive
    delayMicroseconds(20);
}


void setup() {
    // turn the LED on (HIGH is the voltage level)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial2.begin(9600);

    while (!Serial2 && millis() < 5000) {}

    pinMode(RS_DE_RE_SLAVE, OUTPUT);
    digitalWrite(RS_DE_RE_SLAVE, LOW);   // default receive
    sendString(String("RS485 Slave ready."));

    //Serial2.flush();
    String PT_setup = SetupCurrentSensor();
    sendString(PT_setup);

    //Inputs and Outputs
    SetupControl(PyroPin, FirePin, FillSequPin);
    
}


void loop() {
    
  clk_time = millis();

  if(clk_time - lastSensorReadTime - SENSOR_READ_INTERVAL_MS >= 0){
   lastSensorReadTime  = clk_time;
   Nox_pressure = ReadPressureTransducer(1); //1 is Nox, 2 is IPA line 
   IPA_pressure = ReadPressureTransducer(2); //1 is Nox, 2 is IPA line 
   //sendString(String(Nox_pressure,3) + ","+String(IPA_pressure,3)+","+String(clk_time));

  }


  //Rest mode, both fire and fill are low
  if (digitalRead(FillSequPin) == LOW && digitalRead(FirePin) == LOW ){
    Rest();
    fillSeq = 0;
    FillStartTime = clk_time;


    if(lastdisplay > clk_time ){
      lastdisplay = clk_time;
    }
    if(int (clk_time - lastdisplay) - DISPLAY_INTERVAL_MS  >= 0){
      lastdisplay = clk_time; 
      sendString(String("Mode: REST "));
    }

  }

  //Fill mode, fire is low and fill is high
  if (digitalRead(FillSequPin) == HIGH && digitalRead(FirePin) == LOW){
      fireSeq = 0;
      filltime = fillSequence(FillStartTime, clk_time, fillSeq);

      if(lastdisplay > filltime ){
        lastdisplay = filltime;
      }
      if(int (filltime - lastdisplay) - DISPLAY_INTERVAL_MS >= 0){
        lastdisplay = filltime; 
        int ft = filltime/1000;

        sendString(String("Mode: FILL"));
        sendString(String("Fill Time: ")+ String(ft)+String(" s"));

    }
    
  }
  

  // Pyro safety when fire is low
  if (digitalRead(FirePin) == LOW){
    digitalWrite(PyroPin, LOW);
    fireSeq = 0;
    FireStartTime = clk_time;
    AbortStartTime = clk_time;
  }

  
  // Launch mode when fire is high and fill is high
  if (digitalRead(FirePin) == HIGH && digitalRead(FillSequPin) == HIGH){   
    fillSeq = 0; //get out of fill sequence


    launchtime = fireSequence(FireStartTime, clk_time, fireSeq, PyroPin);
    
    if(lastdisplay > launchtime){
      lastdisplay = launchtime;
    }
    if(int (launchtime - lastdisplay) - DISPLAY_INTERVAL_MS >= 0 ){
        lastdisplay = launchtime; 
        int lt = launchtime/1000 -10;
        sendString(String("Mode: FIRE"));
        sendString(String("Launch Time: ")+ String(lt) +String(" s"));
        
    }
  }

  // Abort mode when fire is high and fill is lows
  if (digitalRead(FirePin) == HIGH && digitalRead(FillSequPin) == LOW){
    fillSeq = 0; 
    fireSeq = 0;
 
    aborttime  = abortsequence(AbortStartTime, clk_time, PyroPin);

    if(lastdisplay > aborttime ){
      lastdisplay = aborttime;
    }

    if (int(aborttime - lastdisplay) - DISPLAY_INTERVAL_MS >= 0){
      lastdisplay = aborttime;
      int at = aborttime/1000 -3;
      sendString(String("Mode: ABORT"));

       if (at <= 0){
        sendString(String("ABORT IN: ")+ String(at) +String(" s"));

       }
    }

  }
}     
          
              
