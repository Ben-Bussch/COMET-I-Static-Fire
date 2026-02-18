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

void SetupControl(int PyroPin, int FirePin, int FillSequPin){
  // Servos
  // INIT SERVOS
  NoxEngServo = rc_servo_init(NOX_ENG_PIN); 
  IPAEngServo = rc_servo_init(IPA_ENG_PIN);
  FillServo = rc_servo_init(FILL_PIN);

  // SET INIT PPM
  (void)Rest();

  // CONFIGURE GPIO PINS
  gpio_init(PyroPin);
  gpio_set_dir(PyroPin, true); // output

  gpio_init(FirePin);
  gpio_set_dir(FirePin, false); // INPUT

  gpio_init(FillSequPin);
  gpio_set_dir(FillSequPin, false); // INPUT

}

void Rest(){
  // INIT PPM
  rc_servo_set_micros(&NoxEngServo, NOX_ENG_START_PPM);
  rc_servo_set_micros(&IPAEngServo, IPA_ENG_START_PPM);
  rc_servo_set_micros(&FillServo, FILL_START_PPM);
}

unsigned long fillSequence(unsigned long FillStartTime, unsigned long clk_time, int fillSeq)
{
  unsigned long filltime = clk_time - FillStartTime;
  if(filltime >= 100){
    rc_servo_set_micros(&FillServo, FILL_DELTA_PPM+FILL_START_PPM); 
  }
    
  return filltime;
}


unsigned long fireSequence(unsigned long FireStartTime, unsigned long clk_time, int FireSeq, int PyroPin)
{
  unsigned long countdown =  clk_time - FireStartTime; //starts from zero to avoid negative long issues

  if(FireSeq == 0 && countdown >= 100 ){
      rc_servo_set_micros(&FillServo, FILL_START_PPM);
      FireSeq = 1;
  }


  if(countdown  >= 6000 && FireSeq == 1){
    gpio_put(PyroPin, 1);
    FireSeq = 2; 
  }

  if(countdown  >= 10000 && FireSeq == 2){
    rc_servo_set_micros(&NoxEngServo, NOX_ENG_START_PPM+NOX_DELTA_PPM); 
    FireSeq = 3; 
  }
 if(countdown >= 10100 && FireSeq == 3){
    rc_servo_set_micros(&IPAEngServo, IPA_ENG_START_PPM+IPA_DELTA_PPM);
    FireSeq = 4; 
  }

  return countdown;
}


unsigned long abortsequence(unsigned long AbortStartTime, unsigned long clk_time, int PyroPin){
  unsigned long countdown =  clk_time - AbortStartTime; //starts from zero to avoid negative long issues
  if(countdown  >= 100){
    gpio_put(PyroPin, 0);
  }

  if(countdown  >= 3000){ 
    rc_servo_set_micros(&FillServo, FILL_START_PPM);
  }

  if(countdown  >= 3300){
    rc_servo_set_micros(&NoxEngServo, NOX_ENG_START_PPM+NOX_DELTA_PPM); 
  }
  
  return countdown;

}