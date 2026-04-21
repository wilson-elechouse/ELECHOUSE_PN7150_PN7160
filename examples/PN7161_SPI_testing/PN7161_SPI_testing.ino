/**
 * Basic PN7161 SPI reader test for ELECHOUSE PN7161 MINI V1 SPI.
 *
 * Default pins target ESP32 Dev Module with VSPI:
 *   SCK  = GPIO18
 *   MISO = GPIO19
 *   MOSI = GPIO23
 *   SS   = GPIO5
 * Adjust IRQ/VEN and SPI pins to match your wiring.
 *
 * PN7161 uses the PN7160-compatible code path in this library.
 */

#include <SPI.h>
#include "Electroniccats_PN7150.h"

#define PN7161_IRQ (14)
#define PN7161_VEN (13)
#define PN7161_SS (5)
#define PN7161_SCK (18)
#define PN7161_MISO (19)
#define PN7161_MOSI (23)
#define PN7161_FIXED_VBAT_3V3 (1)
#define PN7161_SPI_TRACE (1)
#define PN7161_SPI_VERBOSE_TRACE (0)
#define PRESENCE_CHECK_INTERVAL_MS (400)

Electroniccats_PN7150 nfc(PN7161_IRQ, PN7161_VEN, PN7161_SS, PN7161_SCK,
                          PN7161_MISO, PN7161_MOSI, PN7160, &SPI);

bool tagActive = false;
unsigned long lastPresenceCheckMs = 0;

String getHexRepresentation(const byte* data, const uint32_t numBytes);
void displayCardInfo();

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("Detect NFC tags with PN7161 over SPI");

#if PN7161_SPI_TRACE
  nfc.setTracePort(&Serial);
  nfc.setVerboseTrace(PN7161_SPI_VERBOSE_TRACE);
  Serial.println("PN7161 SPI event trace enabled");
#endif

#if PN7161_FIXED_VBAT_3V3
  nfc.setPn7160FixedVbat3V3(true);
  Serial.println("PN7161 3V3 mode enabled: official single-rail PMU preset");
#endif

  Serial.println("Initializing...");
  if (nfc.connectNCI()) {
    Serial.println("Error while waking PN7161 over SPI");
    while (1)
      ;
  }

  if (nfc.configureSettings()) {
    Serial.println("The Configure Settings failed!");
    while (1)
      ;
  }

  if (nfc.configMode()) {
    Serial.println("The Configure Mode failed!");
    while (1)
      ;
  }

  nfc.startDiscovery();
  Serial.println("Waiting for a card...");
}

void loop() {
  if (!tagActive && nfc.isTagDetected()) {
    displayCardInfo();

    if (nfc.remoteDevice.hasMoreTags()) {
      nfc.activateNextTagDiscovery();
      Serial.println("Multiple cards are detected!");
    }

    tagActive = true;
    lastPresenceCheckMs = millis();
    Serial.println("Remove the card");
  }

  if (tagActive &&
      (millis() - lastPresenceCheckMs) >= PRESENCE_CHECK_INTERVAL_MS) {
    lastPresenceCheckMs = millis();

    if (nfc.readerReActivate()) {
      tagActive = false;
      Serial.println("Card removed!");
      nfc.startDiscovery();
      Serial.println("Waiting for a card...");
    }
  }

  delay(20);
}

String getHexRepresentation(const byte* data, const uint32_t numBytes) {
  String hexString;

  if (numBytes == 0) {
    return "null";
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

void displayCardInfo() {
  while (true) {
    switch (nfc.remoteDevice.getProtocol()) {
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

    switch (nfc.remoteDevice.getModeTech()) {
      case (nfc.tech.PASSIVE_NFCA):
        Serial.println("\tTechnology: NFC-A");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(),
                                            nfc.remoteDevice.getSensResLen()));

        Serial.print("\tNFC ID = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getNFCID(),
                                            nfc.remoteDevice.getNFCIDLen()));

        Serial.print("\tSEL RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSelRes(),
                                            nfc.remoteDevice.getSelResLen()));
        break;

      case (nfc.tech.PASSIVE_NFCB):
        Serial.println("\tTechnology: NFC-B");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(),
                                            nfc.remoteDevice.getSensResLen()));

        Serial.println("\tAttrib RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getAttribRes(),
                                            nfc.remoteDevice.getAttribResLen()));
        break;

      case (nfc.tech.PASSIVE_NFCF):
        Serial.println("\tTechnology: NFC-F");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(),
                                            nfc.remoteDevice.getSensResLen()));

        Serial.print("\tBitrate = ");
        Serial.println((nfc.remoteDevice.getBitRate() == 1) ? "212" : "424");
        break;

      case (nfc.tech.PASSIVE_NFCV):
        Serial.println("\tTechnology: NFC-V");
        Serial.print("\tID = ");
        Serial.println(
            getHexRepresentation(nfc.remoteDevice.getID(),
                                 sizeof(nfc.remoteDevice.getID())));

        Serial.print("\tAFI = ");
        Serial.println(nfc.remoteDevice.getAFI());

        Serial.print("\tDSF ID = ");
        Serial.println(nfc.remoteDevice.getDSFID(), HEX);
        break;

      default:
        break;
    }

    if (nfc.remoteDevice.hasMoreTags()) {
      Serial.println("Multiple cards are detected!");
      if (!nfc.activateNextTagDiscovery()) {
        break;
      }
    } else {
      break;
    }
  }
}
