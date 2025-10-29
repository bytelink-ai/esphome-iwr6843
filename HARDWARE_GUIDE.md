# Hardware Connection Guide

This guide explains how to connect the TI IWR6843 mmWave Radar to an ESP32 for use with the ESPHome component.

## IWR6843 Pin Overview

The IWR6843 has multiple interfaces. For this implementation, we use:
- **UART**: Configuration interface (115200 baud)
- **SPI**: High-speed data interface
- **Control Pins**: SOP2 (boot mode), NRST (reset)

## Required Connections

### Power Supply
- **VCC**: 5V (3.3V also works but 5V recommended for stability)
- **GND**: Ground (common ground with ESP32)
- **Current**: Ensure power supply can provide at least 500mA

### SPI Interface (Data Port)

| IWR6843 Pin | ESP32 Pin | Function |
|-------------|-----------|----------|
| SPI_CLK | GPIO18 | SPI Clock |
| SPI_MISO | GPIO19 | Master In Slave Out (Data from radar) |
| SPI_MOSI | GPIO23 | Master Out Slave In (optional) |
| SPI_CS | GPIO5 | Chip Select |

**Note**: The IWR6843 SPI operates at up to 10 MHz. We use 2 MHz for stability.

### UART Interface (Config Port)

| IWR6843 Pin | ESP32 Pin | Function |
|-------------|-----------|----------|
| UART_TX | GPIO16 (RX) | Transmit from radar to ESP32 |
| UART_RX | GPIO17 (TX) | Receive from ESP32 to radar |

**Note**: UART runs at 115200 baud for configuration commands.

### Control Pins

| IWR6843 Pin | ESP32 Pin | Function |
|-------------|-----------|----------|
| SOP2 | GPIO25 | Boot mode select (HIGH=functional, LOW=flash) |
| NRST | GPIO26 | Hardware reset (active LOW) |

## Pin Configuration Summary

```yaml
# In your ESPHome YAML

spi:
  clk_pin: GPIO18
  miso_pin: GPIO19
  mosi_pin: GPIO23

uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

iwr6843:
  cs_pin: GPIO5
  sop2_pin: GPIO25
  nrst_pin: GPIO26
```

## Wiring Diagram

```
ESP32                          IWR6843
=====                          =======

5V  ----------------------->  VCC (5V)
GND ----------------------->  GND

GPIO18 (SPI CLK) ---------->  SPI_CLK
GPIO19 (SPI MISO) <---------  SPI_MISO
GPIO23 (SPI MOSI) ---------->  SPI_MOSI (optional)
GPIO5  (CS) --------------->  SPI_CS

GPIO17 (TX) --------------->  UART_RX
GPIO16 (RX) <---------------  UART_TX

GPIO25 ------------------->  SOP2 (Boot Mode)
GPIO26 ------------------->  NRST (Reset)
```

## Hardware Considerations

### 1. Power Supply
- Use a **stable 5V power supply** with at least 500mA capacity
- If using 3.3V, ensure it can supply enough current
- Add a **100µF capacitor** near the IWR6843 VCC pin for stability

### 2. SPI Bus
- Keep SPI wires **as short as possible** (< 15cm recommended)
- Use twisted pair for CLK/MISO to reduce noise
- Add **10kΩ pull-up resistors** on MISO if you experience communication issues

### 3. UART
- UART pins are 3.3V TTL compatible
- No level shifting required for ESP32
- Use **twisted pair** for TX/RX if wires are long

### 4. Grounding
- Ensure **common ground** between ESP32 and IWR6843
- Use **star grounding** if possible (all grounds to one point)

### 5. Mounting
- Mount radar **at ceiling height** (2.5-3m recommended)
- **Face downward** for overhead people tracking
- Keep antenna area **clear of metal objects**

## Troubleshooting

### No SPI Communication
1. Check SPI wiring (CLK, MISO, CS)
2. Verify CS pin is correct
3. Try lowering SPI speed in code (1 MHz instead of 2 MHz)
4. Check power supply stability

### UART Not Working
1. Verify TX/RX are **not swapped**
2. Check baud rate (must be 115200)
3. Ensure UART is enabled in ESPHome config

### Radar Not Responding
1. Check NRST pin (should be HIGH during normal operation)
2. Check SOP2 pin (should be HIGH for functional mode)
3. Try hardware reset button/switch
4. Verify 5V power supply

### Intermittent Data
1. Add capacitors near VCC (100µF + 0.1µF ceramic)
2. Shorten SPI wires
3. Check for electrical noise sources nearby
4. Improve grounding

## Example Hardware Setup

### Recommended ESP32 Board
- ESP32-DevKitC (official Espressif board)
- ESP32-WROOM-32 module
- Minimum 4MB flash recommended

### Power Supply Options
1. **USB Power Bank**: Good for testing (provides stable 5V)
2. **5V Wall Adapter**: Best for permanent installation
3. **Buck Converter**: If using 12V/24V power system

### PCB Layout Tips
If designing a custom PCB:
- Keep SPI traces short and parallel
- Add ground plane under SPI traces
- Place decoupling capacitors close to IWR6843
- Separate digital and analog grounds if possible

## Safety Notes

⚠️ **Important Safety Information:**

1. The IWR6843 emits **76-81 GHz RF energy**
2. Power levels are **safe for continuous human exposure**
3. Follow local regulations for RF devices
4. Do not modify antenna or RF sections
5. Proper mounting ensures optimal performance

## References

- [IWR6843 Datasheet](https://www.ti.com/product/IWR6843)
- [ESP32 Datasheet](https://www.espressif.com/en/products/socs/esp32)
- [ESPHome Documentation](https://esphome.io/)

## Support

For hardware-specific issues, consult:
- TI E2E Forums for IWR6843 questions
- ESPHome Discord for ESP32/ESPHome issues
- GitHub Issues for component-specific problems

