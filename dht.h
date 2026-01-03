#ifndef DHT_H
#define DHT_H

#include <Arduino.h>
#include "system_state.h"
#include <dht11.h>

extern dht11 DHT11;

struct DhtReading
{
	int temperature;
	int humidity;
	int status; // DHTLIB_OK or error code
};

DhtReading readDht();

#endif
