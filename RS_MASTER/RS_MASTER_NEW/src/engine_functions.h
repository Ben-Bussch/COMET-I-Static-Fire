#pragma once

unsigned short SetupControl(int PyroPin, int FirePin, int FillSequPin);
void Rest();
unsigned short fillSequence(/*unsigned long FillStartTime, unsigned long clk_time*/ unsigned long filltime, int fillSeq);
unsigned short fireSequence(/*unsigned long FireStartTime, unsigned long clk_time*/ unsigned long countdown, int* FireSeq, int PyroPin);
unsigned short abortsequence(/*unsigned long AbortStartTime, unsigned long clk_time*/ unsigned long countdown, int PyroPin);
