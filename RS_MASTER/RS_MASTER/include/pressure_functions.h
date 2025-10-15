#pragma once
#include <Arduino.h>
#include <Adafruit_INA219.h>


String SetupCurrentSensor();
float CurrentToPressure(float current, float intercept, float grad);
float ReadPressureTransducer(int PT);
//extern float pressure_readingglobal;
//extern float current_readingglobal;