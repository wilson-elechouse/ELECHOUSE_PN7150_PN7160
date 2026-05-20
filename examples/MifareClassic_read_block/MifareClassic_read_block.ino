/**
 * Example to read a Mifare Classic block 4 and show its information
 * Authors:
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 *
 *  March 2020
 *
 * This code is beerware; if you see me (or any other collaborator
 * member) at the local, and you've found our code helpful,
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#ifndef PN71XX_USE_SPI
#define PN71XX_USE_SPI 0
#endif

#include "Electroniccats_PN7150.h"
#include "Electroniccats_PN71xx_ExampleTransport.h"

#define BLK_NB_MFC 4                                // Block tat wants to be read
#define KEY_MFC 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // Default Mifare Classic key

PN71XX_NFC_INSTANCE(nfc); // switch transport with PN71XX_USE_SPI

void PrintBuf(const byte* data, const uint32_t numBytes) {  // Print hex data buffer in format
  uint32_t szPos;
  for (szPos = 0; szPos < numBytes; szPos++) {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos] & 0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1)) {
      Serial.print(F(" "));
    }
  }
  Serial.println();
}

bool AuthenticateMifareBlock(uint8_t sector, unsigned char *resp,
                             unsigned char *respSize,
                             uint8_t *selectedKeySelector) {
  const uint8_t keySelectors[] = {0x10, 0x11};

  for (uint8_t keyIndex = 0; keyIndex < (sizeof(keySelectors) / sizeof(keySelectors[0])); keyIndex++) {
    unsigned char Auth[] = {0x40, sector, keySelectors[keyIndex], KEY_MFC};

    for (uint8_t attempt = 0; attempt < 3; attempt++) {
      bool status = nfc.readerTagCmd(Auth, sizeof(Auth), resp, respSize);
      if ((status != NFC_ERROR) && (*respSize > 0) && (resp[*respSize - 1] == 0)) {
        if (selectedKeySelector != nullptr) {
          *selectedKeySelector = keySelectors[keyIndex];
        }
        return true;
      }

      Serial.print("Auth key ");
      Serial.print((keySelectors[keyIndex] & 0x01) ? 'B' : 'A');
      Serial.print(" attempt ");
      Serial.print(attempt + 1);
      Serial.print(" failed, status=");
      Serial.print(status);
      Serial.print(", size=");
      Serial.print(*respSize);
      if (*respSize > 0) {
        Serial.print(", last=0x");
        Serial.println(resp[*respSize - 1], HEX);
      } else {
        Serial.println(", empty");
      }

      if ((attempt < 2) && (nfc.readerReActivate() == NFC_SUCCESS)) {
        delay(30);
      } else {
        delay(30);
      }
    }
  }

  return false;
}

void PCD_MIFARE_scenario(void) {
  Serial.println("Start reading process...");
  delay(100);
  bool status;
  unsigned char Resp[256];
  unsigned char RespSize;
  /* Read block 4 */
  unsigned char Read[] = {0x10, 0x30, BLK_NB_MFC};
  uint8_t selectedKeySelector = 0;

  /* Authenticate */
  if (!AuthenticateMifareBlock(BLK_NB_MFC / 4, Resp, &RespSize,
                               &selectedKeySelector)) {
    Serial.println("Auth error!");
    return;
  }
  Serial.print("Authenticated with Key ");
  Serial.println((selectedKeySelector & 0x01) ? 'B' : 'A');

  /* Read block */
  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.println("Error reading sector!");
    return;
  }

  Serial.print("------------------------Sector ");
  Serial.print(BLK_NB_MFC / 4, DEC);
  Serial.println("-------------------------");

  PrintBuf(Resp + 1, RespSize - 2);
}

void setup() {
  Serial.begin(PN71XX_SERIAL_BAUD);
  pn71xxWaitForSerial();
  Serial.println("Read mifare classic data block 4 with PN7150/60");
  pn71xxConfigureExampleTransport(nfc);

  Serial.println("Initializing...");
  uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc);
  if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
    pn71xxPrintInitError(Serial, initStatus);
    while (1)
      ;
  }
  Serial.println("Waiting for an Mifare Classic Card...");
}

void loop() {
  if (nfc.isTagDetected()) {
    switch (nfc.remoteDevice.getProtocol()) {
      case nfc.protocol.MIFARE:
        Serial.println(" - Found MIFARE card");
        switch (nfc.remoteDevice.getModeTech()) {  // Indetify card technology
          case (nfc.tech.PASSIVE_NFCA):
            char tmp[16];
            Serial.print("\tSENS_RES = ");
            sprintf(tmp, "0x%.2X", nfc.remoteDevice.getSensRes()[0]);
            Serial.print(tmp);
            Serial.print(" ");
            sprintf(tmp, "0x%.2X", nfc.remoteDevice.getSensRes()[1]);
            Serial.print(tmp);
            Serial.println(" ");

            Serial.print("\tNFCID = ");
            PrintBuf(nfc.remoteDevice.getNFCID(), nfc.remoteDevice.getNFCIDLen());

            if (nfc.remoteDevice.getSelResLen() != 0) {
              Serial.print("\tSEL_RES = ");
              sprintf(tmp, "0x%.2X", nfc.remoteDevice.getSelRes()[0]);
              Serial.print(tmp);
              Serial.println(" ");
            }
            break;
        }
        PCD_MIFARE_scenario();
        break;

      default:
        Serial.println(" - Found a card, but it is not Mifare");
        break;
    }

    //* It can detect multiple cards at the same time if they use the same protocol
    if (nfc.remoteDevice.hasMoreTags()) {
      nfc.activateNextTagDiscovery();
    }

    Serial.println("Remove the Card");
    nfc.waitForTagRemoval();
    Serial.println("CARD REMOVED!");
  }

  Serial.println("Restarting...");
  nfc.reset();
  Serial.println("Waiting for an Mifare Classic Card...");
  delay(500);
}
