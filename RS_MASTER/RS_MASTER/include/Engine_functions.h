#pragma once
#include <Arduino.h>

void SetupControl(int PyroPin, int FirePin, int FillSequPin);
void Rest();
unsigned long fillSequence(unsigned long FillStartTime, unsigned long clk_time, int fillSeq);
unsigned long fireSequence(unsigned long FireStartTime, unsigned long clk_time, int FireSeq, int PyroPin);
unsigned long abortsequence(unsigned long AbortStartTime, unsigned long clk_time, int PyroPin);

