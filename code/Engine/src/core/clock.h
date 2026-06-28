#pragma once
#include "defines.hpp"


typedef struct clock
{
	f64 start_time;
	f64 elapsed_time;
}clock;

//updates the provided clock
void clock_update(clock* clock);

//starts the provided clock
void clock_start(clock* clock);

//stops the provided clock
void clock_stop(clock* clock);
