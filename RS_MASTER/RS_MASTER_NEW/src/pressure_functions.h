#pragma once
#include <string>

bool pac1934_init(void);
bool pac1934_refresh(void);
std::string SetupCurrentSensor();
float CurrentToPressure(float current, float intercept, float grad);
float ReadPressureTransducer(int PT);
// extern float pressure_readingglobal;
//extern float current_readingglobal;