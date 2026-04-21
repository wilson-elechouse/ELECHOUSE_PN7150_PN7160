/**
 * Example to write a ISO14443-3A(Tag Type 2 - T2T) block 5 and show its information
 * Authors:
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 *
 *  November 2020
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

#define BLK_NB_ISO14443_3A (5)                         // Block to be read it
#define DATA_WRITE_ISO14443_3A 0x11, 0x22, 0x33, 0x44  // Data to write

PN71XX_NFC_INSTANCE(nfc); // switch transport with PN71XX_USE_SPI

void PrintBuf(const byte* data, const uint32_t numBytes) {  // Print hex data buffer in format
  uint32_t szPos;
  for (szPos = 0; szPos < numBytes; szPos++) {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos] & 0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
      Serial.print(F(" "));
  }
  Serial.println();
}

void PCD_ISO14443_3A_scenario(void) {
  bool status;
  unsigned char Resp[256];
  unsigned char RespSize;
  /* Read block */
  unsigned char ReadBlock[] = {0x30, BLK_NB_ISO14443_3A};
  /* Write block */
  unsigned char WriteBlock[] = {0xA2, BLK_NB_ISO14443_3A, DATA_WRITE_ISO14443_3A};

  // Determine ChipWriteAck based on chip model
  uint8_t ChipWriteAck = (nfc.getChipModel() == PN7160) ? 0x14 : 0x00;

  // Write
  status = nfc.readerTagCmd(WriteBlock, sizeof(WriteBlock), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != ChipWriteAck)) {
    Serial.print("Error writing block: ");
    Serial.print(ReadBlock[1], HEX);
    Serial.print(" with error: ");
    Serial.print(Resp[RespSize - 1], HEX);
    return;
  }
  Serial.print("Wrote: ");
  Serial.println(WriteBlock[1]);

  // Read block
  status = nfc.readerTagCmd(ReadBlock, sizeof(ReadBlock), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0x00)) {
    Serial.print("Error reading block: ");
    Serial.print(ReadBlock[1], HEX);
    Serial.print(" with error: ");
    Serial.print(Resp[RespSize - 1], HEX);
    return;
  }
  Serial.print("------------------------Block ");
  Serial.print(BLK_NB_ISO14443_3A, HEX);
  Serial.println("-------------------------");
  PrintBuf(Resp, 4);
}

void setup() {
  Serial.begin(PN71XX_SERIAL_BAUD);
  while (!Serial)
    ;
  Serial.println("Write ISO14443-3A(T2T) data block 5 with PN7150/60");
  pn71xxConfigureExampleTransport(nfc);

  Serial.println("Initializing...");
  uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc);
  if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
    pn71xxPrintInitError(Serial, initStatus);
    while (1)
      ;
  }
  Serial.println("Waiting for an ISO14443-3A Card...");
}

void loop() {
  if (nfc.isTagDetected()) {
    switch (nfc.remoteDevice.getProtocol()) {
      case PROT_T2T:
        Serial.println(" - Found ISO14443-3A(T2T) card");
        switch (nfc.remoteDevice.getModeTech()) {  // Indetify card technology
          case (MODE_POLL | TECH_PASSIVE_NFCA):
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
        PCD_ISO14443_3A_scenario();
        break;

      default:
        Serial.println(" - Found a card, but it is not ISO14443-3A(T2T)!");
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
  Serial.println("Waiting for an ISO14443-3A Card...");
  delay(500);
}
