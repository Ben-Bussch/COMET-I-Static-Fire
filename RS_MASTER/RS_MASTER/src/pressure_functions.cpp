#include "pressure_functions.h"
#include <Wire.h>

Adafruit_INA219 ina219A;
Adafruit_INA219 ina219B;
float ina219intercept = -23.751;
float ina219gradient = 5.7567;


// **Define the global variable here**
//float pressure_readingglobal = 0.0;
//float current_readingglobal = 0.0;

String SetupCurrentSensor()
{
    String status;
    if (!ina219A.begin() && !ina219B.begin()) {
        status = "Failed to find INA219 chip";
    } else {
        ina219A.setCalibration_32V_1A();
        ina219B.setCalibration_32V_1A();
        status = "Successfully connected to INA219A chip";
    }
    return status;
}

float CurrentToPressure(float current, float intercept, float grad)
{
    return current * grad + intercept;
}

float ReadPressureTransducer(int PT)
{
    float currentmA = 0;
    float pressure_reading = 0;
    if(PT == 1){
    currentmA = ina219A.getCurrent_mA();
    pressure_reading = CurrentToPressure(currentmA, ina219intercept, ina219gradient);
    }

    if(PT == 2){
    currentmA = ina219B.getCurrent_mA();
    pressure_reading = CurrentToPressure(currentmA, ina219intercept, ina219gradient);
    }
    
    //pressure_readingglobal = pressure_reading; // update global
    //current_readingglobal = currentmA; // update global
    return pressure_reading;
}
