/**
 * Example to detect P2P device
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

PN71XX_NFC_INSTANCE(nfc); // switch transport with PN71XX_USE_SPI

RfIntf_t RfInterface;                                              //Intarface to save data for multiple tags

uint8_t mode = 3;                                                  // modes: 1 = Reader/ Writer, 2 = Emulation, 3 = Peer to peer P2P

void ResetMode(){                                  //Reset the configuration mode after each reading
  Serial.println("Re-initializing...");
  nfc.ConfigMode(mode);
  nfc.StartDiscovery(mode);
}


void setup(){
  Serial.begin(PN71XX_SERIAL_BAUD);
  while(!Serial);
  Serial.println("Detect P2P devices with PN7150/60");
  pn71xxConfigureExampleTransport(nfc);

  Serial.println("Initializing...");
  uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc, mode);
  if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
    pn71xxPrintInitError(Serial, initStatus);
    while (1);
  }
  Serial.println("Waiting for a P2P device...");
}

void loop(){
  if(!nfc.WaitForDiscoveryNotification(&RfInterface)){ // Waiting to detect
    if (RfInterface.Interface == INTF_NFCDEP) {
      if ((RfInterface.ModeTech & MODE_LISTEN) == MODE_LISTEN)
        Serial.println(" - P2P TARGET MODE: Activated from remote Initiator");
      else
        Serial.println(" - P2P INITIATOR MODE: Remote Target activated");

      /* Process with SNEP for NDEF exchange */
      nfc.processP2pMode(RfInterface);
      Serial.println("Peer lost!");
    }
    ResetMode();
  }
  delay(500);
}
