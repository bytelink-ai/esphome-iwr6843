# ESPHome IWR6843 Custom Component

ESPHome custom component for Texas Instruments IWR6843 mmWave Radar with 3D People Tracking and Fall Detection.

## Features

- **Dual Interface**: UART for configuration, SPI for high-speed data retrieval
- **3D People Tracking**: Track up to 5 persons simultaneously
- **Fall Detection**: Real-time fall detection for each tracked person
- **Dynamic Configuration**: Live updates of tracking boundaries and parameters
- **Home Assistant Integration**: Native sensors, numbers, buttons, and switches

## Hardware Requirements

- ESP32 (recommended) or ESP8266
- Texas Instruments IWR6843 mmWave Radar
- Connections:
  - UART: Configuration interface (115200 baud)
  - SPI: Data interface (921600 baud equivalent)
  - SOP2: Boot mode selection pin
  - NRST: Hardware reset pin

## Installation

### Method 1: External Components (Recommended)

Add to your ESPHome configuration:

```yaml
external_components:
  - source: github://yourusername/esphome-iwr6843
    components: [ iwr6843 ]
```

### Method 2: Local Installation

1. Clone this repository
2. Copy the `components/iwr6843` folder to your ESPHome `custom_components` directory

## Configuration

### Basic Example

```yaml
spi:
  clk_pin: GPIO18
  miso_pin: GPIO19
  mosi_pin: GPIO23

uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

iwr6843:
  spi_id: spi_bus
  cs_pin: GPIO5
  sop2_pin: GPIO25
  nrst_pin: GPIO26
  
  # Sensor Configuration
  ceiling_height: 290  # cm
  max_tracks: 5
  
  # Tracking Boundaries (meters)
  tracking_boundary:
    x_max: 4.0
    x_min: -4.0
    y_max: 4.0
    y_min: -4.0
    z_max: 3.0
    z_min: -0.5
  
  # Presence Boundaries (meters)
  presence_boundary:
    x_max: 4.0
    x_min: -4.0
    y_max: 4.0
    y_min: -4.0
    z_max: 2.5
    z_min: 0.5
```

### Advanced Configuration with All Entities

```yaml
iwr6843:
  # ... basic config ...
  
  # Enable all tracking IDs (1-5)
  tracking_ids:
    - id: 1
      name: "Person 1"
    - id: 2
      name: "Person 2"
    - id: 3
      name: "Person 3"
    - id: 4
      name: "Person 4"
    - id: 5
      name: "Person 5"
  
  # Update interval for SPI reads
  update_interval: 100ms
```

## Entities

### Sensors (Per ID 1-5)

| Entity | Type | Unit | Description |
|--------|------|------|-------------|
| `binary_sensor.person_id_x_presence` | Binary | - | Presence detection (detected/clear) |
| `binary_sensor.person_id_x_fall` | Binary | - | Fall detection (detected/clear) |
| `sensor.person_id_x_velocity` | Sensor | mm/s | Movement velocity |
| `sensor.person_id_x_x_coordinate` | Sensor | cm | X position |
| `sensor.person_id_x_y_coordinate` | Sensor | cm | Y position |
| `sensor.person_id_x_z_coordinate` | Sensor | cm | Z position |

### Configuration Numbers

| Entity | Type | Range | Unit | Description |
|--------|------|-------|------|-------------|
| `number.ceiling_height` | Number | 100-500 | cm | Sensor mounting height |
| `number.max_tracks` | Number | 1-5 | - | Maximum tracked persons |
| `number.tracking_boundary_x_max` | Number | -10 to 10 | m | Max X tracking boundary |
| `number.tracking_boundary_x_min` | Number | -10 to 10 | m | Min X tracking boundary |
| `number.tracking_boundary_y_max` | Number | -10 to 10 | m | Max Y tracking boundary |
| `number.tracking_boundary_y_min` | Number | -10 to 10 | m | Min Y tracking boundary |
| `number.tracking_boundary_z_max` | Number | -5 to 10 | m | Max Z tracking boundary |
| `number.tracking_boundary_z_min` | Number | -5 to 10 | m | Min Z tracking boundary |
| `number.presence_boundary_x_max` | Number | -10 to 10 | m | Max X presence boundary |
| `number.presence_boundary_x_min` | Number | -10 to 10 | m | Min X presence boundary |
| `number.presence_boundary_y_max` | Number | -10 to 10 | m | Max Y presence boundary |
| `number.presence_boundary_y_min` | Number | -10 to 10 | m | Min Y presence boundary |
| `number.presence_boundary_z_max` | Number | -5 to 10 | m | Max Z presence boundary |
| `number.presence_boundary_z_min` | Number | -5 to 10 | m | Min Z presence boundary |

### Control Entities

| Entity | Type | Description |
|--------|------|-------------|
| `button.reset_sensor` | Button | Hardware reset (NRST pin) |
| `switch.flash_mode` | Switch | Boot mode selection (SOP2 pin) |

## How It Works

### Configuration Updates

When any configuration parameter is changed (boundaries, ceiling height, max tracks), the component:

1. Sends `sensorStop` via UART
2. Sends the updated parameter command
3. Sends `sensorStart` via UART

Example UART sequence for updating tracking boundary:
```
sensorStop
boundaryBox -4 4 -4 4 -0.5 3
sensorStart
```

### Data Retrieval

- **SPI Interface**: High-speed data frames (similar to UART data port at 921600 baud)
- **Frame Structure**: Magic Word (8 bytes) + Header (40 bytes) + TLV Data
- **Parsing**: Based on TI mmWave SDK frame format
- **Update Rate**: ~8.33 FPS (120ms per frame)

### ID Management

- Maximum 5 simultaneous tracks
- Stable ID assignment (1-5)
- When presence changes to "clear", all values reset to 0

## Frame Format

The component parses TI mmWave standard frames:

```
Magic Word: 0x0201040306050807 (8 bytes)
Header: 40 bytes total
  - Version (4 bytes)
  - Total Packet Length (4 bytes)
  - Platform (4 bytes)
  - Frame Number (4 bytes)
  - Time (4 bytes)
  - Num Detected Objects (4 bytes)
  - Num TLVs (4 bytes)
  - Subframe Number (4 bytes)

TLV Data:
  - Type (4 bytes)
  - Length (4 bytes)
  - Value (variable)
```

## Pin Configuration

### Required Pins

```yaml
# SPI Pins
clk_pin: GPIOxx   # SPI Clock
miso_pin: GPIOxx  # SPI MISO (Data from radar)
mosi_pin: GPIOxx  # SPI MOSI (optional)
cs_pin: GPIOxx    # SPI Chip Select

# UART Pins
tx_pin: GPIOxx    # UART TX (to radar)
rx_pin: GPIOxx    # UART RX (from radar)

# Control Pins
sop2_pin: GPIOxx  # Boot mode (HIGH=functional, LOW=flash)
nrst_pin: GPIOxx  # Hardware reset (active LOW)
```

### IWR6843 Pin Mapping

Refer to IWR6843 datasheet for exact pin assignments.

## Troubleshooting

### No Data Received

1. Check SPI connections (CLK, MISO, CS)
2. Verify SPI speed settings
3. Check NRST and SOP2 pin states
4. Monitor ESPHome logs for frame parsing errors

### Configuration Not Updating

1. Verify UART TX/RX connections
2. Check baud rate (must be 115200)
3. Monitor UART communication in logs

### Presence Detection Not Working

1. Check presence boundary settings
2. Verify ceiling height matches physical installation
3. Ensure radar is powered correctly (5V, sufficient current)

## Development

### Project Structure

```
esphome-iwr6843/
├── components/
│   └── iwr6843/
│       ├── __init__.py           # Python component definition
│       ├── iwr6843.h             # C++ header
│       ├── iwr6843.cpp           # C++ implementation
│       ├── iwr6843_sensor.h      # Sensor definitions
│       ├── iwr6843_number.h      # Number entities
│       ├── iwr6843_button.h      # Button entities
│       ├── iwr6843_switch.h      # Switch entities
│       ├── frame_parser.h        # Frame parsing logic
│       └── fall_detection.h      # Fall detection algorithm
├── examples/
│   └── basic.yaml                # Example configuration
└── README.md
```

## License

MIT License - See LICENSE file for details

## Credits

Based on Texas Instruments IWR6843 3D People Tracking Demo:
- TI mmWave SDK
- 3D People Tracking Reference Design
- Fall Detection Algorithm

## Support

For issues and questions, please open an issue on GitHub.

