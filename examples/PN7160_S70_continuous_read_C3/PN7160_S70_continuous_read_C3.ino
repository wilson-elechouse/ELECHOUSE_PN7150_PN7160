#ifndef PN71XX_USE_SPI
#define PN71XX_USE_SPI 0
#endif

#include "Electroniccats_PN7150.h"
#include "Electroniccats_PN71xx_ExampleTransport.h"

PN71XX_NFC_INSTANCE(nfc);

static bool gCardActive = false;
static uint8_t gUid[10];
static uint8_t gUidLen = 0;
static uint8_t gPresenceMissCount = 0;
static unsigned long gLastUidPrintAt = 0;
static unsigned long gLastPresenceCheckAt = 0;
static unsigned long gLastSeekingPrintAt = 0;

void printUid(const uint8_t *uid, uint8_t uidLen) {
  for (uint8_t index = 0; index < uidLen; index++) {
    if (uid[index] <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(uid[index], HEX);
    if (index + 1 < uidLen) {
      Serial.print(' ');
    }
  }
  Serial.println();
}

void printSearchingIfNeeded() {
  if ((millis() - gLastSeekingPrintAt) >= 1000) {
    Serial.println("Searching for S70...");
    gLastSeekingPrintAt = millis();
  }
}

bool cacheCurrentUid() {
  if (nfc.remoteDevice.getModeTech() != nfc.tech.PASSIVE_NFCA) {
    return false;
  }

  gUidLen = nfc.remoteDevice.getNFCIDLen();
  if (gUidLen == 0 || gUidLen > sizeof(gUid)) {
    return false;
  }

  memcpy(gUid, nfc.remoteDevice.getNFCID(), gUidLen);
  return true;
}

bool restartDiscovery() {
  gCardActive = false;
  gUidLen = 0;
  gPresenceMissCount = 0;
  gLastUidPrintAt = 0;
  gLastPresenceCheckAt = 0;
  gLastSeekingPrintAt = 0;

  if (!nfc.reset()) {
    Serial.println("reset() failed, reinitializing...");
    uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc);
    if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
      pn71xxPrintInitError(Serial, initStatus);
      return false;
    }
  }

  Serial.println("Searching for S70...");
  gLastSeekingPrintAt = millis();
  return true;
}

void handleDetectedCard() {
  if (nfc.remoteDevice.getProtocol() != nfc.protocol.MIFARE) {
    Serial.println("Detected tag is not MIFARE/S70, restarting...");
    restartDiscovery();
    return;
  }

  if (!cacheCurrentUid()) {
    Serial.println("Failed to read UID, restarting...");
    restartDiscovery();
    return;
  }

  Serial.print("Card detected, UID=");
  printUid(gUid, gUidLen);

  gCardActive = true;
  gPresenceMissCount = 0;
  gLastUidPrintAt = 0;
  gLastPresenceCheckAt = 0;
}

void setup() {
  Serial.begin(PN71XX_SERIAL_BAUD);
  pn71xxWaitForSerial();

  Serial.println("PN7160 S70 continuous read test");
  pn71xxConfigureExampleTransport(nfc);

#if !PN71XX_USE_SPI && (defined(ARDUINO_ESP32C3_DEV) || defined(CONFIG_IDF_TARGET_ESP32C3))
  Serial.print("I2C SDA=");
  Serial.print(PN71XX_I2C_SDA);
  Serial.print(", SCL=");
  Serial.print(PN71XX_I2C_SCL);
  Serial.print(", IRQ=");
  Serial.print(PN71XX_I2C_IRQ);
  Serial.print(", VEN=");
  Serial.println(PN71XX_I2C_VEN);
#endif

  Serial.println("Initializing...");
  uint8_t initStatus = pn71xxInitializeDiscoveryMode(nfc);
  if (initStatus != PN71XX_EXAMPLE_INIT_OK) {
    pn71xxPrintInitError(Serial, initStatus);
    while (1) {
      delay(1000);
    }
  }

  Serial.println("Searching for S70...");
  gLastSeekingPrintAt = millis();
}

void loop() {
  if (!gCardActive) {
    if (nfc.isTagDetected()) {
      handleDetectedCard();
    } else {
      printSearchingIfNeeded();
    }
    delay(30);
    return;
  }

  if ((millis() - gLastUidPrintAt) >= 500) {
    Serial.print("UID=");
    printUid(gUid, gUidLen);
    gLastUidPrintAt = millis();
  }

  if ((millis() - gLastPresenceCheckAt) >= 250) {
    gLastPresenceCheckAt = millis();
    if (nfc.readerReActivate() == NFC_SUCCESS) {
      gPresenceMissCount = 0;
      if (cacheCurrentUid()) {
        ;
      }
    } else {
      gPresenceMissCount++;
      if (gPresenceMissCount >= 3) {
        Serial.println("Card removed");
        if (!restartDiscovery()) {
          while (1) {
            delay(1000);
          }
        }
      }
    }
  }

  delay(20);
}
