#include "dht.h"

// Define the DHT11 instance used by this sketch (single definition)
dht11 DHT11;

int ck = DHT11.read(DHT11PIN);
int readTemperature()
{
  if (ck != DHTLIB_OK)
    return -99; // Error reading DHT11; return invalid value
  return DHT11.temperature;
}

int readHumidity()
{
  if (ck != DHTLIB_OK)
    return -99; // Error reading DHT11; return invalid value
  return DHT11.humidity;
}