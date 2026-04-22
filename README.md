# ELECHOUSE PN7160 / PN7161 NFC RFID Module Library

Arduino library for the **ELECHOUSE PN7160 NFC RFID Module**, **ELECHOUSE PN7161 NFC RFID Module-I2C**, and **ELECHOUSE PN7161 MINI V1 SPI**.

This repository is a product-focused fork of the original Electronic Cats PN7150/PN7160 library. It keeps the upstream Arduino API and source layout, while updating the documentation to match ELECHOUSE hardware, product links, and getting-started resources.

## What changed for PN7161 MINI V1 SPI

The SPI compatibility work is now integrated into the shared example flow. That means the standard examples in this repository can be used with the ELECHOUSE **PN7161 MINI V1 SPI** board instead of requiring a separate SPI-only example path.

In practice:

- The common examples now support both I2C and SPI transport selection
- The PN7161 MINI V1 SPI board can use the library `examples/` directly
- SPI builds use the PN7160-compatible code path, which is the intended compatibility path for PN7161 in this library

## Supported ELECHOUSE Modules

| Module | Product Page | Notes |
| --- | --- | --- |
| PN7160 NFC RFID Module | [ELECHOUSE PN7160](https://www.elechouse.com/product/pn7160-nfc-rfid-module/) | I2C NFC module with onboard antenna for reader/writer, card emulation, and peer-to-peer modes |
| PN7161 NFC RFID Module-I2C | [ELECHOUSE PN7161](https://www.elechouse.com/product/pn7161-nfc-rfid-module-i2c/) | Hardware/software compatible with PN7160 for this library, with Apple ECP support on the module side |
| PN7161 MINI V1 SPI | [ELECHOUSE PN7161 MINI V1 SPI](https://www.elechouse.com/product/pn7161-mini-v1-spi/) | SPI version for compact integrations, supported through the PN7160-compatible SPI code path in this library |

> For PN7161 I2C and PN7161 MINI V1 SPI, use the `PN7160` chip selection in sketches. This codebase exposes `PN7150` and `PN7160` as the available chip models, and PN7161 is used through the PN7160-compatible path.

## Highlights

- Based on NXP PN71xx NFC controller devices with I2C host communication, plus SPI host support for PN7160/PN7161 SPI variants
- Supports reader/writer mode, peer-to-peer mode, and card emulation mode
- Works with common NFC technologies used by the upstream PN7150/PN7160 stack, including ISO14443-A/B, ISO15693, MIFARE, and NDEF workflows
- Includes ready-to-run Arduino examples for tag detection, NDEF read/write, MIFARE Classic, ISO15693, and P2P discovery
- Well suited for ESP32, RP2040, STM32, MKR, and other 32-bit Arduino-compatible boards

## Arduino Quick Start

### 1. Install the library

- Clone this repository into your Arduino `libraries` folder, or
- Download the repository as a ZIP and install it from **Arduino IDE -> Sketch -> Include Library -> Add .ZIP Library...**

### 2. Wire the module

#### I2C modules: PN7160 / PN7161 NFC RFID Module-I2C

The library needs:

- `SDA`
- `SCL`
- `IRQ`
- `VEN`
- power and ground

For ESP32, ELECHOUSE documentation commonly uses:

| Module Pin | Example ESP32 Pin |
| --- | --- |
| `SDA` | `GPIO21` |
| `SCL` | `GPIO22` |
| `IRQ` | `GPIO14` |
| `VEN` | `GPIO13` |

Adjust the pins to match your board and wiring.

#### SPI module: PN7161 MINI V1 SPI

For the SPI version, connect:

- `SS`
- `SCK`
- `MISO`
- `MOSI`
- `IRQ`
- `VEN`
- power and ground

Default example mapping:

| Signal | Default Pin |
| --- | --- |
| `IRQ` | `GPIO14` |
| `VEN` | `GPIO13` |
| `SS` | `GPIO5` |
| `SCK` | `GPIO18` |
| `MISO` | `GPIO19` |
| `MOSI` | `GPIO23` |

### 3. Start with a simple sketch

Open `examples/DetectTags/DetectTags.ino` and configure it for your board.

For ELECHOUSE PN7160 or PN7161 modules, the initialization should look like this:

```cpp
#include "Electroniccats_PN7150.h"

#define PN7160_IRQ  (14)
#define PN7160_VEN  (13)
#define PN7160_ADDR (0x28)

Electroniccats_PN7150 nfc(PN7160_IRQ, PN7160_VEN, PN7160_ADDR, PN7160);
```

### 4. Upload and test

- Open the serial monitor
- Bring an NFC tag close to the module antenna
- Use the included examples to read UID, read/write NDEF, or access MIFARE/ISO15693 blocks

### 5. Use the examples over SPI

For the ELECHOUSE **PN7161 MINI V1 SPI** board, the shared examples now support a transport switch:

- Open an example such as `examples/DetectTags/DetectTags.ino`
- Change `#define PN71XX_USE_SPI 0` to `1`
- Compile for your ESP32 or other supported MCU

These defaults are provided through `src/Electroniccats_PN71xx_ExampleTransport.h` and can be overridden per sketch if needed.

If you are updating older product documentation, the key point is now:

> The PN7161 MINI V1 SPI board can use the standard library examples in this repository after enabling SPI mode.

## Included Examples

- `examples/DetectTags` - detect nearby NFC tags and print card information
- `examples/DetectingReaders` - reader discovery example
- `examples/NDEFReadMessage` - read NDEF records from a tag
- `examples/NDEFSendMessage` - write standard NDEF messages
- `examples/NDEFSendRawMessage` - write custom raw NDEF payloads
- `examples/MifareClassic_read_block` - read MIFARE Classic blocks
- `examples/MifareClassic_write_block` - write MIFARE Classic blocks
- `examples/ISO14443-3A_read_block` - read ISO14443-3A data
- `examples/ISO14443-3A_write_block` - write ISO14443-3A data
- `examples/ISO15693_read_block` - read ISO15693 blocks
- `examples/ISO15693_write_block` - write ISO15693 blocks
- `examples/P2P_Discovery` - peer-to-peer discovery demo
- `examples/PN7160_testing` - ESP32-oriented PN7160 smoke test with LED activity while a tag is present
- `examples/PN7161_SPI_testing` - experimental PN7161 SPI smoke test for ELECHOUSE PN7161 MINI V1 SPI

## Notes for ELECHOUSE Users

- The source code keeps the upstream header and class name: `Electroniccats_PN7150`
- The codebase still exposes `PN7150` and `PN7160` as the selectable chip models
- For ELECHOUSE PN7160, PN7161 NFC RFID Module-I2C, and PN7161 MINI V1 SPI, use the `PN7160` compatibility path
- The default I2C address is `0x28`
- Shared examples now support both I2C and SPI through `PN71XX_USE_SPI`
- For SPI builds, the examples use the PN7160-compatible code path and enable the 3.3V single-rail preset automatically
- ELECHOUSE also provides an address selection guide if you need to change the module I2C address

## Documentation and Resources

### Product pages

- [PN7160 NFC RFID Module](https://www.elechouse.com/product/pn7160-nfc-rfid-module/)
- [PN7161 NFC RFID Module-I2C](https://www.elechouse.com/product/pn7161-nfc-rfid-module-i2c/)
- [PN7161 MINI V1 SPI](https://www.elechouse.com/product/pn7161-mini-v1-spi/)

### Repository docs

- [Library API](API.md)
- Shared Arduino examples under [`examples/`](examples)
- SPI/I2C example transport defaults in [`src/Electroniccats_PN71xx_ExampleTransport.h`](src/Electroniccats_PN71xx_ExampleTransport.h)

### ELECHOUSE guides

- [Quick Guide (PDF)](https://www.elechouse.com/wp-content/uploads/2024/06/Quick-Guide-I2C.pdf)
- [ESP32 and PN7160 in Arduino IDE (PDF)](https://www.elechouse.com/wp-content/uploads/2024/06/ESP32-and-PN7160-in-Arduino-IDE.pdf)
- [I2C Address Setting (PDF)](https://www.elechouse.com/wp-content/uploads/2024/06/I2C-address-setting.pdf)
- [PN7160 Documentation](https://www.elechouse.com/docs/pn7160/)
- [PN7161 Documentation](https://www.elechouse.com/docs/pn7161/)

## Compatibility

This Arduino library is intended for 32-bit boards and modern Arduino-compatible platforms.

- Arduino MKR family
- STM32H747
- ESP32
- RP2040
- Renesas-based boards

**Note:** this library is not intended for the classic Arduino AVR family.

## Credits

- Original upstream library: [Electronic Cats PN7150 / PN7160 library](https://github.com/ElectronicCats/ElectronicCats-PN7150)
- ELECHOUSE hardware pages and documentation were used to adapt this README for PN7160 and PN7161 module users
