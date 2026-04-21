/**
 * Example detect tags and show their unique ID
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

#include <Electroniccats_PN7150.h>
#include <Electroniccats_PN71xx_ExampleTransport.h>

PN71XX_NFC_INSTANCE(nfc); // switch transport with PN71XX_USE_SPI

void setup() {
  Serial.begin(PN71XX_SERIAL_BAUD);
  while (!Serial)
    ;
  Serial.println("Detect NFC readers with PN7150/60");
  pn71xxConfigureExampleTransport(nfc);
  Serial.println("Initializing...");

  uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc, 2);
  if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
    pn71xxPrintInitError(Serial, initStatus);
    while (true)
      ;
  }
  Serial.print("Waiting for a reader...");
}

void loop() {
  Serial.print(".");

  if (nfc.isReaderDetected()) {
    Serial.println("\nReader detected!");
    nfc.handleCardEmulation();
    nfc.closeCommunication();
    Serial.print("\nWaiting for a reader");
  }
}
