#ifndef NFC_H
#define NFC_H

#include <Arduino.h>
#include <Wire.h>
#include "MFRC522_I2C.h"
#include "system_state.h"

extern MFRC522 mfrc522;
String nfcRead();
bool isNfcUidGranted(const String &uid);

#endif // NFC_H