/**
 * Example detect tags and show their unique ID
 * Authors:
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 *
 * Updated by Francisco Torres - Electronic Cats - electroniccats.com
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

PN71XX_NFC_INSTANCE(nfc); // switch transport with PN71XX_USE_SPI

// Function prototypes
String getHexRepresentation(const byte* data, const uint32_t numBytes);
void displayCardInfo();

void setup() {
  Serial.begin(PN71XX_SERIAL_BAUD);
  while (!Serial)
    ;
  Serial.println("Detect NFC tags with PN7150/60");
  pn71xxConfigureExampleTransport(nfc);

  Serial.println("Initializing...");
  uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc);
  if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
    pn71xxPrintInitError(Serial, initStatus);
    while (1)
      ;
  }
  Serial.println("Waiting for an Card ...");
}

void loop() {
  if (nfc.isTagDetected()) {
    displayCardInfo();

    // It can detect multiple cards at the same time if they use the same protocol
    if (nfc.remoteDevice.hasMoreTags()) {
      nfc.activateNextTagDiscovery();
      Serial.println("Multiple cards are detected!");
    }

    Serial.println("Remove the Card");
    nfc.waitForTagRemoval();
    Serial.println("Card removed!");
  }

  Serial.println("Restarting...");
  nfc.reset();
  Serial.println("Waiting for a Card...");
  delay(500);
}

String getHexRepresentation(const byte* data, const uint32_t numBytes) {
  String hexString;

  if (numBytes == 0) {
    hexString = "null";
  }

  for (uint32_t szPos = 0; szPos < numBytes; szPos++) {
    hexString += "0x";
    if (data[szPos] <= 0xF)
      hexString += "0";
    hexString += String(data[szPos] & 0xFF, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1)) {
      hexString += " ";
    }
  }
  return hexString;
}

void displayCardInfo() {  // Funtion in charge to show the card/s in te field
  char tmp[16];

  while (true) {
    switch (nfc.remoteDevice.getProtocol()) {  // Indetify card protocol
      case nfc.protocol.T1T:
      case nfc.protocol.T2T:
      case nfc.protocol.T3T:
      case nfc.protocol.ISODEP:
        Serial.print(" - POLL MODE: Remote activated tag type: ");
        Serial.println(nfc.remoteDevice.getProtocol());
        break;
      case nfc.protocol.ISO15693:
        Serial.println(" - POLL MODE: Remote ISO15693 card activated");
        break;
      case nfc.protocol.MIFARE:
        Serial.println(" - POLL MODE: Remote MIFARE card activated");
        break;
      default:
        Serial.println(" - POLL MODE: Undetermined target");
        return;
    }

    switch (nfc.remoteDevice.getModeTech()) {  // Indetify card technology
      case (nfc.tech.PASSIVE_NFCA):
        Serial.println("\tTechnology: NFC-A");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(), nfc.remoteDevice.getSensResLen()));

        Serial.print("\tNFC ID = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getNFCID(), nfc.remoteDevice.getNFCIDLen()));

        Serial.print("\tSEL RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSelRes(), nfc.remoteDevice.getSelResLen()));

        break;

      case (nfc.tech.PASSIVE_NFCB):
        Serial.println("\tTechnology: NFC-B");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(), nfc.remoteDevice.getSensResLen()));

        Serial.println("\tAttrib RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getAttribRes(), nfc.remoteDevice.getAttribResLen()));

        break;

      case (nfc.tech.PASSIVE_NFCF):
        Serial.println("\tTechnology: NFC-F");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(), nfc.remoteDevice.getSensResLen()));

        Serial.print("\tBitrate = ");
        Serial.println((nfc.remoteDevice.getBitRate() == 1) ? "212" : "424");

        break;

      case (nfc.tech.PASSIVE_NFCV):
        Serial.println("\tTechnology: NFC-V");
        Serial.print("\tID = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getID(), sizeof(nfc.remoteDevice.getID())));

        Serial.print("\tAFI = ");
        Serial.println(nfc.remoteDevice.getAFI());

        Serial.print("\tDSF ID = ");
        Serial.println(nfc.remoteDevice.getDSFID(), HEX);
        break;

      default:
        break;
    }

    // It can detect multiple cards at the same time if they are the same technology
    if (nfc.remoteDevice.hasMoreTags()) {
      Serial.println("Multiple cards are detected!");
      if (!nfc.activateNextTagDiscovery()) {
        break;  // Can't activate next tag
      }
    } else {
      break;
    }
  }
}
