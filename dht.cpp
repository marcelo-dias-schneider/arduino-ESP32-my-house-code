#include "dht.h"

// Define the DHT11 instance used by this sketch (single definition)
dht11 DHT11;

// Cache last successful readings so we can return a value even if a read fails
static int lastTemp = -99;
static int lastHum = -99;

DhtReading readDht()
{
  DhtReading r;
  int ck = DHT11.read(DHT11PIN);
  r.status = ck;
  if (ck == DHTLIB_OK)
  {
    lastTemp = DHT11.temperature;
    lastHum = DHT11.humidity;
  }

  r.temperature = lastTemp;
  r.humidity = lastHum;
  return r;
}