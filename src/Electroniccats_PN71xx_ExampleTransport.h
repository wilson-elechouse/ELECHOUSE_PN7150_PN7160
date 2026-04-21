#ifndef Electroniccats_PN71xx_ExampleTransport_H
#define Electroniccats_PN71xx_ExampleTransport_H

#ifndef PN71XX_USE_SPI
#define PN71XX_USE_SPI 0
#endif

enum Pn71xxExampleInitStatus : uint8_t {
  PN71XX_EXAMPLE_INIT_OK = 0,
  PN71XX_EXAMPLE_INIT_CONNECT_FAILED = 1,
  PN71XX_EXAMPLE_INIT_SETTINGS_FAILED = 2,
  PN71XX_EXAMPLE_INIT_MODE_FAILED = 3,
  PN71XX_EXAMPLE_INIT_DISCOVERY_FAILED = 4,
};

#if PN71XX_USE_SPI

#ifndef PN71XX_SPI_IRQ
#define PN71XX_SPI_IRQ 14
#endif

#ifndef PN71XX_SPI_VEN
#define PN71XX_SPI_VEN 13
#endif

#ifndef PN71XX_SPI_SS
#define PN71XX_SPI_SS 5
#endif

#ifndef PN71XX_SPI_SCK
#define PN71XX_SPI_SCK 18
#endif

#ifndef PN71XX_SPI_MISO
#define PN71XX_SPI_MISO 19
#endif

#ifndef PN71XX_SPI_MOSI
#define PN71XX_SPI_MOSI 23
#endif

#ifndef PN71XX_SPI_FIXED_VBAT_3V3
#define PN71XX_SPI_FIXED_VBAT_3V3 1
#endif

#define PN71XX_SERIAL_BAUD 115200
#define PN71XX_NFC_INSTANCE(name)                                             \
  Electroniccats_PN7150 name(PN71XX_SPI_IRQ, PN71XX_SPI_VEN, PN71XX_SPI_SS,   \
                             PN71XX_SPI_SCK, PN71XX_SPI_MISO, PN71XX_SPI_MOSI,\
                             PN7160, &SPI)

inline void pn71xxConfigureExampleTransport(Electroniccats_PN7150 &nfc) {
#if PN71XX_SPI_FIXED_VBAT_3V3
  nfc.setPn7160FixedVbat3V3(true);
#else
  (void)nfc;
#endif
}

#else

#ifndef PN71XX_I2C_IRQ
#define PN71XX_I2C_IRQ 11
#endif

#ifndef PN71XX_I2C_VEN
#define PN71XX_I2C_VEN 13
#endif

#ifndef PN71XX_I2C_ADDR
#define PN71XX_I2C_ADDR 0x28
#endif

#define PN71XX_SERIAL_BAUD 9600
#define PN71XX_NFC_INSTANCE(name)                                             \
  Electroniccats_PN7150 name(PN71XX_I2C_IRQ, PN71XX_I2C_VEN, PN71XX_I2C_ADDR, \
                             PN7150)

inline void pn71xxConfigureExampleTransport(Electroniccats_PN7150 &nfc) {
  (void)nfc;
}

#endif

inline uint8_t pn71xxInitializeDiscoveryMode(Electroniccats_PN7150 &nfc,
                                             uint8_t mode = 1) {
  if (nfc.connectNCI()) {
    return PN71XX_EXAMPLE_INIT_CONNECT_FAILED;
  }

  if (nfc.configureSettings()) {
    return PN71XX_EXAMPLE_INIT_SETTINGS_FAILED;
  }

  if (mode == 1) {
    if (nfc.configMode()) {
      return PN71XX_EXAMPLE_INIT_MODE_FAILED;
    }

    if (nfc.startDiscovery()) {
      return PN71XX_EXAMPLE_INIT_DISCOVERY_FAILED;
    }
  } else {
    if (nfc.ConfigMode(mode)) {
      return PN71XX_EXAMPLE_INIT_MODE_FAILED;
    }

    if (nfc.StartDiscovery(mode)) {
      return PN71XX_EXAMPLE_INIT_DISCOVERY_FAILED;
    }
  }

  return PN71XX_EXAMPLE_INIT_OK;
}

inline void pn71xxPrintInitError(Stream &stream, uint8_t initStatus) {
  switch (initStatus) {
    case PN71XX_EXAMPLE_INIT_CONNECT_FAILED:
      stream.println("Error while setting up the mode, check connections!");
      break;
    case PN71XX_EXAMPLE_INIT_SETTINGS_FAILED:
      stream.println("The Configure Settings failed!");
      break;
    case PN71XX_EXAMPLE_INIT_MODE_FAILED:
      stream.println("The Configure Mode failed!");
      break;
    case PN71XX_EXAMPLE_INIT_DISCOVERY_FAILED:
      stream.println("The Start Discovery command failed!");
      break;
    default:
      break;
  }
}

#endif
