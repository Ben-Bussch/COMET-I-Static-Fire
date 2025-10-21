#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include "Engine_functions.h"

Servo NoxEngServo;
Servo IPAEngServo;
Servo FillServo;

int NoxEngPin = 6;
int IPAEngPin = 9;
int FillPin = 11;


int NoxEngStartPPM = 980;
int IPAEngStartPPM = 1400;
int FillStartPPM = 1050;


int NoxdeltaPPM = 800;
int IPAdeltaPPM = 600;
int FilldeltaPPM = 750;

int PPMpos = 0; 
int count = 0;

void SetupControl(int PyroPin, int FirePin, int FillSequPin){
  //Servos
  NoxEngServo.attach(NoxEngPin);
  NoxEngServo.writeMicroseconds(NoxEngStartPPM); 

  IPAEngServo.attach(IPAEngPin);
  IPAEngServo.writeMicroseconds(IPAEngStartPPM); 

  FillServo.attach(FillPin);
  FillServo.writeMicroseconds(FillStartPPM); 
     
  pinMode(PyroPin, OUTPUT);
  pinMode(FirePin, INPUT);
  pinMode(FillSequPin,  INPUT);

}

void Rest(){
   NoxEngServo.writeMicroseconds(NoxEngStartPPM);
   IPAEngServo.writeMicroseconds(IPAEngStartPPM); 
   FillServo.writeMicroseconds(FillStartPPM); 
}

unsigned long fillSequence(unsigned long FillStartTime, unsigned long clk_time, int fillSeq)
{
  unsigned long filltime = - FillStartTime + clk_time;
  FillServo.writeMicroseconds(FilldeltaPPM+FillStartPPM); 
  /*
  if(fillSeq == 0){
      NoxEngServo.writeMicroseconds(NoxEngStartPPM);
      IPAEngServo.writeMicroseconds(IPAEngStartPPM);
      FillServo.writeMicroseconds(FillStartPPM);
      PPMpos = 0;
      fillSeq = 1;
  }
  if(fillSeq == 1 && filltime%5 == 0){
    // waits 5ms for the servo to reach the position
    while(PPMpos <= (FilldeltaPPM)){
      PPMpos += 3;
      FillServo.writeMicroseconds(PPMpos+FillStartPPM); 
      }
    if(PPMpos >= (FilldeltaPPM)){
      fillSeq = 2;  
      }

    }
    */
  
  return filltime;
}


unsigned long fireSequence(unsigned long FireStartTime, unsigned long clk_time, int FireSeq, int PyroPin)
{
  unsigned long countdown =  clk_time - FireStartTime ;

  if(FireSeq == 0){
      FillServo.writeMicroseconds(FillStartPPM);
      FireSeq = 1;
  }


  if(countdown  >= 5000 && FireSeq == 1){
    digitalWrite(PyroPin, HIGH);
    FireSeq = 2; 
  }

  if(countdown  >= 10000 && FireSeq == 2){
    NoxEngServo.writeMicroseconds(NoxEngStartPPM+NoxdeltaPPM); 
    FireSeq = 3; 
  }
 if(countdown >= 10200 && FireSeq == 3){
    IPAEngServo.writeMicroseconds(IPAEngStartPPM+IPAdeltaPPM);
    FireSeq = 4; 
  }

  return countdown;
}


unsigned long abortsequence(unsigned long AbortStartTime, unsigned long clk_time, int PyroPin){
  unsigned long countdown =  clk_time - AbortStartTime;
  if(clk_time -  AbortStartTime  >= 100){
  digitalWrite(PyroPin, LOW);
  }

  if(countdown  >= 3000){
    NoxEngServo.writeMicroseconds(NoxEngStartPPM+NoxdeltaPPM); 
  }
  
  return countdown;

}