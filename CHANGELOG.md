# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01-29

### Added
- Initial release of ESPHome IWR6843 custom component
- Dual interface support (UART for configuration, SPI for data)
- 3D People Tracking support (up to 5 persons simultaneously)
- Binary sensors for presence detection (detected/clear)
- Binary sensors for fall detection (detected/clear)
- Numeric sensors for coordinates (X, Y, Z in cm)
- Numeric sensors for velocity (mm/s)
- Configuration numbers for:
  - Ceiling height (100-500 cm)
  - Max tracks (1-5)
  - Tracking boundaries (X/Y/Z min/max in meters)
  - Presence boundaries (X/Y/Z min/max in meters)
- Button entity for hardware reset (NRST pin)
- Switch entity for flash mode control (SOP2 pin)
- Automatic configuration update via UART (sensorStop → update → sensorStart)
- Stable ID assignment (1-5) for tracked persons
- Automatic reset to zero when presence becomes clear
- Frame parsing based on TI mmWave SDK format
- Fall detection based on Z-coordinate and velocity thresholds
- Comprehensive documentation:
  - README.md with installation and configuration guide
  - HARDWARE_GUIDE.md with wiring diagrams
  - STRUCTURE.md with project architecture
  - CONTRIBUTING.md with contribution guidelines
  - DEPLOYMENT.md with deployment instructions
- Example YAML configuration
- MIT License

### Technical Details
- SPI Speed: 2 MHz (configurable)
- UART Baud Rate: 115200
- Frame Format: TI mmWave SDK standard (Magic Word + 40-byte header + TLV data)
- Compatible with ESP32 and ESP8266 (ESP32 recommended)
- Multi-device SPI bus support (separate CS pins required)

### Known Limitations
- Maximum 5 simultaneous tracks
- SPI speed limited to 2 MHz by default (hardware supports up to 10 MHz)
- Fall detection uses simplified algorithm (Z-coordinate + velocity)
- No gesture recognition in this version
- No historical data logging

### Tested Hardware
- ESP32-DevKitC
- ESP32-WROOM-32
- Texas Instruments IWR6843AOPR

### Dependencies
- ESPHome 2023.12.0 or later
- ESP-IDF framework (included with ESPHome)

## [Unreleased]

### Planned Features
- Advanced fall detection with configurable parameters
- Custom tracking zones
- Multi-radar support
- Kalman filter for smoother tracking
- Gesture recognition
- Activity classification
- OTA firmware update for IWR6843
- Web-based configuration interface

---

## Release Notes

### How to Upgrade

**From scratch:**
```yaml
external_components:
  - source: github://bytelink-ai/esphome-iwr6843@v1.0.0
    components: [ iwr6843 ]
```

**Breaking Changes:**
None (initial release)

### Migration Guide
Not applicable (initial release)

---

For detailed information about each release, see the [GitHub Releases](https://github.com/bytelink-ai/esphome-iwr6843/releases) page.

