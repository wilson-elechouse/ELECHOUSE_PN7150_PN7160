/**
 * Example detect tags and show their unique ID on PN7160.
 * While a tag is present, the LED blinks on ESP32 boards.
 *
 * Authors:
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 *
 * Adapted for ELECHOUSE PN7160 testing.
 */

#include "Electroniccats_PN7150.h"

#define PN7160_IRQ (14)
#define PN7160_VEN (13)
#define PN7160_ADDR (0x28)
#define LED_PIN (2)

Electroniccats_PN7150 nfc(PN7160_IRQ, PN7160_VEN, PN7160_ADDR, PN7160);

String getHexRepresentation(const byte* data, const uint32_t numBytes);
void displayCardInfo();

volatile bool tagPresent = false;

#if defined(ARDUINO_ARCH_ESP32)
void blinkTask(void* pvParameters) {
  const int blinkDelay = 167;

  while (true) {
    if (tagPresent) {
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(blinkDelay / portTICK_PERIOD_MS);
      digitalWrite(LED_PIN, LOW);
      vTaskDelay(blinkDelay / portTICK_PERIOD_MS);
    } else {
      digitalWrite(LED_PIN, LOW);
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
}
#endif

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("Detect NFC tags with PN7160");
  Serial.println("Initializing...");

  if (nfc.connectNCI()) {
    Serial.println("Error while setting up the mode, check connections!");
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

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

#if defined(ARDUINO_ARCH_ESP32)
  TaskHandle_t blinkTaskHandle;
  xTaskCreate(blinkTask, "BlinkTask", 2048, NULL, 1, &blinkTaskHandle);
#endif
}

void loop() {
  if (nfc.isTagDetected()) {
    tagPresent = true;

    displayCardInfo();

    if (nfc.remoteDevice.hasMoreTags()) {
      nfc.activateNextTagDiscovery();
      Serial.println("Multiple cards are detected!");
    }

    Serial.println("Remove the card");
    nfc.waitForTagRemoval();
    Serial.println("Card removed!");

    tagPresent = false;
    digitalWrite(LED_PIN, LOW);
  }

  Serial.println("Restarting...");
  nfc.reset();
  Serial.println("Waiting for a card...");
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
