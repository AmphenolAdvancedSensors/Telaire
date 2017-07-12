
#include "LowPassFilter.h"

LPH::LPH()
{
	lastValue = 0.0;
}

void LPH::Reset()
{
	lastValue = 0.0;
}

float LPH::GetAvg()
{
	return lastValue;
}

void LPH::Update(int data)
{
	lastValue = (1-FILTER_A) * data + FILTER_A * lastValue;
}


