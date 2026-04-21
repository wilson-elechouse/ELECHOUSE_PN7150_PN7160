/**
 * Example to write a Mifare Classic block 4 and show its information
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

#define BLK_NB_MFC 4                                // Block that wants to be read
#define KEY_MFC 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // Default Mifare Classic key

// Data to be written in the Mifare Classic block
#define DATA_WRITE_MFC 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
#define DATA_CLEAR_MFC 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

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

bool WriteMifareBlock(unsigned char *writePart1, unsigned char *writePart2,
                      unsigned char *resp, unsigned char *respSize,
                      uint8_t chipWriteAck) {
  bool status = nfc.readerTagCmd(writePart1, sizeof(writePart1[0]) * 3, resp,
                                 respSize);
  if ((status == NFC_ERROR) || (resp[*respSize - 1] != chipWriteAck)) {
    return false;
  }

  status = nfc.readerTagCmd(writePart2, sizeof(writePart2[0]) * 17, resp,
                            respSize);
  if ((status == NFC_ERROR) || (resp[*respSize - 1] != chipWriteAck)) {
    return false;
  }

  return true;
}

uint8_t PCD_MIFARE_scenario(void) {
  Serial.println("Start reading process...");
  bool status;
  unsigned char Resp[256];
  unsigned char RespSize;
  /* Authenticate sector 1 with generic keys */
  unsigned char Auth[] = {0x40, BLK_NB_MFC / 4, 0x10, KEY_MFC};
  /* Read block 4 */
  unsigned char Read[] = {0x10, 0x30, BLK_NB_MFC};
  /* Write block 4 */
  unsigned char WritePart1[] = {0x10, 0xA0, BLK_NB_MFC};
  unsigned char WriteDataPart2[] = {0x10, DATA_WRITE_MFC};
  unsigned char ClearDataPart2[] = {0x10, DATA_CLEAR_MFC};

  // Determine ChipWriteAck based on chip model
  uint8_t ChipWriteAck = (nfc.getChipModel() == PN7160) ? 0x14 : 0x00;

  /* Authenticate */
  status = nfc.readerTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.println("Auth error!");
    return 1;
  }

  /* Read block */
  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.print("Error reading block!");
    return 2;
  }
  Serial.print("------------------------Sector ");
  Serial.print(BLK_NB_MFC / 4, DEC);
  Serial.println("-------------------------");
  Serial.print("-------------------------Block ");
  Serial.print(BLK_NB_MFC, DEC);
  Serial.println("-------------------------");
  Serial.println("Original data:");
  PrintBuf(Resp + 1, RespSize - 2);

  /* Write test data */
  Serial.println("Writing test pattern...");
  if (!WriteMifareBlock(WritePart1, WriteDataPart2, Resp, &RespSize,
                        ChipWriteAck)) {
    Serial.print("Error writing test pattern!");
    return 3;
  }

  /* Read block again to see the changes */
  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.print("Error reading back test pattern!");
    return 4;
  }
  Serial.print("------------------------Sector ");
  Serial.print(BLK_NB_MFC / 4, DEC);
  Serial.println("-------------------------");
  Serial.print("----------------- Test Data in Block ");
  Serial.print(BLK_NB_MFC, DEC);
  Serial.println("-----------------");
  PrintBuf(Resp + 1, RespSize - 2);

  /* Clear block to zeros */
  Serial.println("Clearing block to zeros...");
  if (!WriteMifareBlock(WritePart1, ClearDataPart2, Resp, &RespSize,
                        ChipWriteAck)) {
    Serial.print("Error clearing block!");
    return 5;
  }

  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.print("Error reading back cleared block!");
    return 6;
  }
  Serial.print("----------------- Cleared Data in Block ");
  Serial.print(BLK_NB_MFC, DEC);
  Serial.println("-----------------");
  PrintBuf(Resp + 1, RespSize - 2);
  return 0;
}

void setup() {
  Serial.begin(PN71XX_SERIAL_BAUD);
  while (!Serial)
    ;
  Serial.println("Write mifare classic data block 4 with PN7150/60");
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
