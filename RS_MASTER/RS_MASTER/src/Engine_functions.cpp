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

unsigned long pyro_start_time = 8000; //4000 for early pyro, 8800 for later pyro

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
  unsigned long filltime = clk_time - FillStartTime;
  if(filltime >= 100){
    FillServo.writeMicroseconds(FilldeltaPPM+FillStartPPM); 
  }
    
  return filltime;
}


unsigned long fireSequence(unsigned long FireStartTime, unsigned long clk_time, int PyroPin)
{
  unsigned long countdown =  clk_time - FireStartTime; //starts from zero to avoid negative long issues

  if(countdown >= 10100 ){
    IPAEngServo.writeMicroseconds(IPAEngStartPPM+IPAdeltaPPM);
  }

  else if(countdown  >= 10000){
    NoxEngServo.writeMicroseconds(NoxEngStartPPM+NoxdeltaPPM);  
  }

  else if(countdown  >= pyro_start_time && (pyro_start_time+1000) >= countdown){
    digitalWrite(PyroPin, HIGH);
  }

  else if(countdown >= 100 ){
      FillServo.writeMicroseconds(FillStartPPM);
  }

  return countdown;
}


unsigned long abortsequence(unsigned long AbortStartTime, unsigned long clk_time, int PyroPin){
  unsigned long countdown =  clk_time - AbortStartTime; //starts from zero to avoid negative long issues
  if(countdown  >= 100){
  digitalWrite(PyroPin, LOW);
  }

  if(countdown  >= 3000){ 
    FillServo.writeMicroseconds(FillStartPPM);
  }

  if(countdown  >= 3300){
    NoxEngServo.writeMicroseconds(NoxEngStartPPM+NoxdeltaPPM); 
  }
  
  return countdown;

}