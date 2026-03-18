#include "pico/stdlib.h"
#include "pico/rc.h" // servos stuff
#include "engine_functions.h"

#define NOX_ENG_PIN 6
#define IPA_ENG_PIN 9
#define FILL_PIN 11

#define NOX_ENG_START_PPM 980 // 980 us might be too low?
#define IPA_ENG_START_PPM 1400
#define FILL_START_PPM 1050

#define NOX_DELTA_PPM 800
#define IPA_DELTA_PPM 600
#define FILL_DELTA_PPM 750

unsigned PPM_POS = 0; 
unsigned COUNT = 0;

rc_servo NoxEngServo; 
rc_servo IPAEngServo;
rc_servo FillServo;

// Setup for servos
unsigned short SetupControl(int PyroPin, int FirePin, int FillSeqPin){
  // Servos
  // INIT SERVOS
  NoxEngServo = rc_servo_init(NOX_ENG_PIN); 
  IPAEngServo = rc_servo_init(IPA_ENG_PIN);
  FillServo = rc_servo_init(FILL_PIN);

  // SET INIT PPM
  (void)Rest();

  // CONFIGURE GPIO PINS
  gpio_init(PyroPin);
  gpio_set_dir(PyroPin, 1); // output

  gpio_init(FirePin);
  gpio_set_dir(FirePin, 0); // INPUT

  gpio_init(FillSeqPin);
  gpio_set_dir(FillSeqPin, 0); // INPUT

  return (unsigned short)1; // 1 means successful
}

// Rest state of servos
void Rest(){
  // INIT PPM
  rc_servo_set_micros(&NoxEngServo, NOX_ENG_START_PPM);
  rc_servo_set_micros(&IPAEngServo, IPA_ENG_START_PPM);
  rc_servo_set_micros(&FillServo, FILL_START_PPM);
}


unsigned short fillSequence(/*unsigned long FillStartTime, unsigned long clk_time*/ unsigned long filltime, unsigned short* fillSeq){
  //unsigned long filltime = clk_time - FillStartTime;

  if(filltime >= 100 && *fillSeq == 0){
    rc_servo_set_micros(&FillServo, FILL_DELTA_PPM+FILL_START_PPM); 
    *fillSeq = 1; // 1 if fill successful
    return 1;
  }
    
  return 0;
}

// This function is wrong, FireSeq must be pointer otherwise it's not updated/returned and will do whatever idk and sequence won't work
// so am changing it

// Fire sequence, needs to be called in a loop until completed as countdown (clk_time) needs constant updating
// returns 1 for stage 1 completed, 2 for stage 2 and so on final being 4 when fire sequence completed
unsigned short fireSequence(/*unsigned long FireStartTime, unsigned long clk_time*/ unsigned long countdown, int* FireSeq, int PyroPin){
  //unsigned long countdown =  clk_time - FireStartTime; //starts from zero to avoid negative long issues

  // stage 1
  if(*FireSeq == 0 && countdown >= 100 ){
    rc_servo_set_micros(&FillServo, FILL_START_PPM);
    *FireSeq = 1;
  }
  // stage 2
  else if(countdown  >= 6000 && *FireSeq == 1){
    gpio_put(PyroPin, 1);
    *FireSeq = 2; 
  }
  // stage 3
  else if(countdown  >= 10000 && *FireSeq == 2){
    rc_servo_set_micros(&NoxEngServo, NOX_ENG_START_PPM+NOX_DELTA_PPM); 
    *FireSeq = 3; 
  }
  // stage 4
  else if(countdown >= 10100 && *FireSeq == 3){
    rc_servo_set_micros(&IPAEngServo, IPA_ENG_START_PPM+IPA_DELTA_PPM);
    *FireSeq = 4; 
  }

  return 1;
}


unsigned short abortsequence(/*unsigned long AbortStartTime, unsigned long clk_time*/ unsigned long countdown, int* abortSeq, int PyroPin){
  //unsigned long countdown =  clk_time - AbortStartTime; //starts from zero to avoid negative long issues

  if(countdown >= 3300){
    rc_servo_set_micros(&NoxEngServo, NOX_ENG_START_PPM + NOX_DELTA_PPM);
    *abortSeq = 3;
  }
  else if(countdown >= 3000){
    rc_servo_set_micros(&FillServo, FILL_START_PPM);
    *abortSeq = 2;
  }
  else if(countdown >= 100){
    gpio_put(PyroPin, 0);
    *abortSeq = 1;
  }
  
  return 1;
}