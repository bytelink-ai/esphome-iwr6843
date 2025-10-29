# Project Structure

Complete overview of the ESPHome IWR6843 Custom Component repository structure.

```
esphome-iwr6843/
│
├── README.md                          # Main documentation
├── LICENSE                            # MIT License
├── HARDWARE_GUIDE.md                  # Hardware connection guide
├── STRUCTURE.md                       # This file
├── .gitignore                         # Git ignore rules
│
├── components/                        # ESPHome components
│   └── iwr6843/                       # IWR6843 component
│       ├── __init__.py                # Python component definition
│       │                              # - Defines CONFIG_SCHEMA
│       │                              # - Registers component with ESPHome
│       │                              # - Handles configuration validation
│       │
│       ├── iwr6843.h                  # C++ header file
│       │                              # - Class definition: IWR6843Component
│       │                              # - Data structures (TrackData, FrameHeader, etc.)
│       │                              # - Function declarations
│       │                              # - Constants (MAGIC_WORD, frame sizes)
│       │
│       ├── iwr6843.cpp                # C++ implementation
│       │                              # - setup(): Initialize hardware
│       │                              # - loop(): Read SPI frames
│       │                              # - SPI communication functions
│       │                              # - UART configuration functions
│       │                              # - Frame parsing logic
│       │                              # - Sensor update logic
│       │
│       ├── sensor.py                  # Sensor platform (coordinates, velocity)
│       │                              # - X/Y/Z coordinate sensors
│       │                              # - Velocity sensor
│       │                              # - Units: cm, mm/s
│       │
│       ├── binary_sensor.py           # Binary sensor platform
│       │                              # - Presence detection (detected/clear)
│       │                              # - Fall detection (detected/clear)
│       │
│       ├── number.py                  # Number platform (configuration)
│       │                              # - Ceiling height
│       │                              # - Max tracks
│       │                              # - Boundary configurations
│       │
│       ├── number.h                   # Number entity C++ header
│       │                              # - IWR6843Number class
│       │                              # - Configuration update logic
│       │
│       ├── button.py                  # Button platform
│       │                              # - Reset button
│       │
│       ├── button.h                   # Button entity C++ header
│       │                              # - IWR6843ResetButton class
│       │                              # - Hardware reset implementation
│       │
│       ├── switch.py                  # Switch platform
│       │                              # - Flash mode switch
│       │
│       └── switch.h                   # Switch entity C++ header
│                                      # - IWR6843FlashSwitch class
│                                      # - SOP2 pin control
│
└── examples/                          # Example configurations
    └── basic.yaml                     # Basic example YAML
                                       # - Full sensor configuration
                                       # - All 5 person IDs
                                       # - Number entities for boundaries
                                       # - Control entities (button/switch)
```

## File Descriptions

### Core Component Files

#### `__init__.py`
Python configuration file that:
- Defines the component schema (pins, boundaries, etc.)
- Validates user configuration
- Generates C++ code from YAML config
- Registers with ESPHome build system

#### `iwr6843.h`
Main C++ header file containing:
- `IWR6843Component` class definition
- Data structures for tracks and frames
- SPI/UART interface declarations
- Sensor registration functions

#### `iwr6843.cpp`
Main C++ implementation:
- **Hardware initialization**: SPI, UART, GPIO pins
- **Frame reading**: Magic word detection, header parsing
- **TLV parsing**: Extract track data from radar frames
- **Sensor updates**: Publish data to Home Assistant
- **Configuration**: Send UART commands to radar

### Platform Files

#### Sensor Platform (`sensor.py`)
- Coordinate sensors (X, Y, Z) in centimeters
- Velocity sensor in mm/s
- Registers sensors with parent component

#### Binary Sensor Platform (`binary_sensor.py`)
- Presence detection (occupancy device class)
- Fall detection (safety device class)
- Binary states: on/off

#### Number Platform (`number.py` + `number.h`)
- Configuration numbers (ceiling height, max tracks)
- Boundary configuration (tracking and presence)
- Live update via UART (sensorStop → command → sensorStart)

#### Button Platform (`button.py` + `button.h`)
- Hardware reset button
- Triggers NRST pin (active LOW pulse)

#### Switch Platform (`switch.py` + `switch.h`)
- Flash mode switch
- Controls SOP2 pin (LOW = flash, HIGH = functional)

### Documentation Files

#### `README.md`
- Project overview
- Installation instructions
- Configuration guide
- Entity reference table

#### `HARDWARE_GUIDE.md`
- Wiring diagram
- Pin connections
- Power supply requirements
- Troubleshooting tips

#### `STRUCTURE.md` (this file)
- Project organization
- File descriptions
- Data flow explanation

## Data Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                          IWR6843 Radar                          │
│                                                                 │
│  ┌──────────────┐                         ┌──────────────┐     │
│  │     UART     │  (Configuration)        │     SPI      │     │
│  │  115200 baud │  <─────────────         │  (Data Port) │     │
│  └──────────────┘                         └──────────────┘     │
│         │                                         │             │
└─────────┼─────────────────────────────────────────┼─────────────┘
          │                                         │
          │ Commands:                               │ Frame Data:
          │ - sensorStop                            │ - Magic Word
          │ - boundaryBox ...                       │ - Header (40B)
          │ - sensorStart                           │ - TLV Data
          │                                         │
          ▼                                         ▼
┌─────────────────────────────────────────────────────────────────┐
│                     ESP32 (ESPHome)                             │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │            IWR6843Component (C++)                        │  │
│  │                                                          │  │
│  │  ┌───────────┐    ┌─────────────┐    ┌──────────────┐  │  │
│  │  │   UART    │    │     SPI     │    │    Sensors   │  │  │
│  │  │  Handler  │    │   Parser    │    │   Handler    │  │  │
│  │  └───────────┘    └─────────────┘    └──────────────┘  │  │
│  │       │                  │                    │         │  │
│  │       │ Config           │ Frames             │ Updates │  │
│  │       ▼                  ▼                    ▼         │  │
│  │  ┌───────────────────────────────────────────────────┐ │  │
│  │  │          Track Data Management (1-5 IDs)         │ │  │
│  │  └───────────────────────────────────────────────────┘ │  │
│  └──────────────────────────────────────────────────────────┘  │
│                            │                                    │
│                            │ Publish States                     │
│                            ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Home Assistant API (ESPHome Native API)                 │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                             │
                             │ MQTT/API
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Home Assistant                             │
│                                                                 │
│  Entities:                                                      │
│  - binary_sensor.person_1_presence                              │
│  - binary_sensor.person_1_fall                                  │
│  - sensor.person_1_x_coordinate                                 │
│  - sensor.person_1_y_coordinate                                 │
│  - sensor.person_1_z_coordinate                                 │
│  - sensor.person_1_velocity                                     │
│  - number.ceiling_height                                        │
│  - number.tracking_boundary_x_max                               │
│  - ...                                                          │
│  - button.reset_sensor                                          │
│  - switch.flash_mode                                            │
└─────────────────────────────────────────────────────────────────┘
```

## Configuration Flow

### Initial Setup (setup())

1. **Initialize GPIOs**
   - CS pin (SPI)
   - SOP2 pin (flash mode control)
   - NRST pin (reset control)

2. **Reset Sensor**
   - Assert NRST (LOW)
   - Wait 100ms
   - Release NRST (HIGH)
   - Wait 500ms

3. **Send Configuration via UART**
   - sensorStop
   - flushCfg
   - All radar configuration commands
   - sensorStart

### Runtime Loop (loop())

1. **Find Magic Word on SPI**
   - Read bytes until `0x0201040306050807` found

2. **Read Frame Header**
   - Parse 40-byte header
   - Extract frame length and metadata

3. **Read Frame Data**
   - Read remaining bytes based on header
   - Parse TLV structures

4. **Process Track Data**
   - Extract X, Y, Z coordinates
   - Calculate velocities
   - Assign stable IDs (1-5)
   - Detect falls

5. **Update Sensors**
   - Publish presence status
   - Publish coordinates (cm)
   - Publish velocity (mm/s)
   - Publish fall detection

6. **Cleanup**
   - Reset inactive tracks to 0
   - Clear old IDs (every 50 frames)

### Configuration Update Flow

When user changes a number entity:

1. **Number entity changed**
   ```yaml
   number.ceiling_height: 300
   ```

2. **Trigger update in number.h**
   ```cpp
   void control(float value) override {
       // Stop sensor
       parent_->send_config_update("sensorStop");
       
       // Send update
       parent_->send_config_update("sensorPosition 3.0 0 90");
       
       // Start sensor
       parent_->send_config_update("sensorStart");
   }
   ```

3. **UART transmission**
   ```
   UART TX: "sensorStop\n"
   UART TX: "sensorPosition 3.0 0 90\n"
   UART TX: "sensorStart\n"
   ```

4. **Radar applies new config**

## Key Features

### 1. Dual Interface Architecture
- **UART** (115200 baud): Configuration and control
- **SPI** (2 MHz): High-speed data retrieval
- Separate interfaces prevent data/config conflicts

### 2. Stable ID Management
- Radar IDs are dynamic and can change
- Component assigns stable display IDs (1-5)
- Position-based tracking for continuity

### 3. Automatic Reset to Zero
- When presence = clear, all values → 0
- Prevents stale data in Home Assistant
- Clean state transitions

### 4. Fall Detection
- Z-coordinate threshold (< 0.5m)
- Velocity threshold (< -1.0 m/s downward)
- Multi-frame confirmation (3+ frames)

### 5. Live Configuration
- All boundaries adjustable via Home Assistant UI
- Changes applied without firmware recompile
- Instant effect (via UART command sequence)

## Development Notes

### Adding New Sensors

1. Add to `iwr6843.h`:
   ```cpp
   void register_my_sensor(uint8_t id, sensor::Sensor *sensor);
   std::map<uint8_t, sensor::Sensor *> my_sensors_;
   ```

2. Add to `iwr6843.cpp`:
   ```cpp
   void IWR6843Component::register_my_sensor(uint8_t id, sensor::Sensor *sensor) {
       this->my_sensors_[id] = sensor;
   }
   ```

3. Update in `update_sensors_()`:
   ```cpp
   if (this->my_sensors_.count(id)) {
       this->my_sensors_[id]->publish_state(value);
   }
   ```

4. Create `my_sensor.py` platform file

### Modifying Frame Parser

The frame parser in `iwr6843.cpp` (`parse_tlv_data_()`) follows TI's mmWave SDK format:

- **TLV Header**: Type (4B) + Length (4B)
- **TLV Value**: Variable length based on type
- **Track Data**: 68 bytes per track (ID, X, Y, Z, velocities, etc.)

Refer to TI mmWave SDK documentation for TLV structure details.

## Testing

### Unit Tests (Future Work)
- Frame parser validation
- ID assignment logic
- Boundary checking functions

### Integration Tests
- Full YAML compilation
- SPI communication simulation
- UART command verification

### Hardware Tests
- Real radar connection
- Frame rate measurement
- Latency profiling

## License

MIT License - See LICENSE file for details.

