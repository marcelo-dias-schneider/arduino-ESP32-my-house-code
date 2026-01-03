#include "nfc.h"

// Define the global MFRC522 instance for the sketch
MFRC522 mfrc522(0x28);

String nfcRead()
{
  String uidString = "";
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    return ""; // No new card present
  }

  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();

  return uidString;
}


// Check whether an NFC UID is in the granted list
bool isNfcUidGranted(const String &uid)
{
  size_t count = sizeof(NFC_GRANTED_UIDS) / sizeof(NFC_GRANTED_UIDS[0]);
  for (size_t i = 0; i < count; ++i)
  {
    if (NFC_GRANTED_UIDS[i] == uid)
      return true;
  }
  return false;
}