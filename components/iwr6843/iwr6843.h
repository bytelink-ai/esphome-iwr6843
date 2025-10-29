#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <vector>
#include <map>

namespace esphome {
namespace iwr6843 {

// Magic word for frame detection (8 bytes)
static const uint8_t MAGIC_WORD[] = {0x02, 0x01, 0x04, 0x03, 0x06, 0x05, 0x08, 0x07};
static const size_t MAGIC_WORD_SIZE = 8;
static const size_t FRAME_HEADER_SIZE = 40;
static const size_t MAX_FRAME_SIZE = 10000;
static const uint32_t UART_BAUD_RATE = 115200;
static const uint32_t SPI_SPEED = 2000000;  // 2 MHz

// TLV Types (from TI SDK)
enum TLVType {
  TLVTYPE_DETECTED_POINTS = 1,
  TLVTYPE_TARGET_LIST = 2,
  TLVTYPE_TARGET_INDEX = 3,
  TLVTYPE_POINT_CLOUD = 6,
  TLVTYPE_TARGET_HEIGHT = 7,
  TLVTYPE_TRACKED_TARGETS = 8,
  TLVTYPE_COMPRESSED_SPHERICAL_POINTS = 9
};

// Frame Header Structure
struct FrameHeader {
  uint64_t magic_word;
  uint32_t version;
  uint32_t total_packet_len;
  uint32_t platform;
  uint32_t frame_number;
  uint32_t time_cpu_cycles;
  uint32_t num_detected_obj;
  uint32_t num_tlvs;
  uint32_t subframe_number;
};

// Track Data Structure
struct TrackData {
  uint8_t id;          // Display ID (1-5)
  uint8_t radar_id;    // Original radar ID
  float x;             // X coordinate (m)
  float y;             // Y coordinate (m)
  float z;             // Z coordinate (m)
  float vel_x;         // X velocity (m/s)
  float vel_y;         // Y velocity (m/s)
  float vel_z;         // Z velocity (m/s)
  float confidence;    // Track confidence
  bool is_present;     // Presence flag
  bool is_fallen;      // Fall detection flag
  uint32_t last_seen;  // Frame number last seen
};

// Boundary Configuration
struct BoundaryBox {
  float x_min;
  float x_max;
  float y_min;
  float y_max;
  float z_min;
  float z_max;
};

// Forward declarations
class IWR6843Component;

class IWR6843Component : public Component, public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                                   spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ>,
                         public uart::UARTDevice {
 public:
  IWR6843Component() = default;

  // Component lifecycle
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Pin configuration
  void set_cs_pin(GPIOPin *pin) { this->cs_pin_ = pin; }
  void set_sop2_pin(GPIOPin *pin) { this->sop2_pin_ = pin; }
  void set_nrst_pin(GPIOPin *pin) { this->nrst_pin_ = pin; }

  // Sensor configuration
  void set_ceiling_height(uint16_t height) { this->ceiling_height_ = height; }
  void set_max_tracks(uint8_t max_tracks) { this->max_tracks_ = max_tracks; }
  void set_tracking_boundary(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
  void set_presence_boundary(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);

  // Tracking ID management
  void add_tracking_id(uint8_t id, const std::string &name);

  // Sensor registration
  void register_presence_sensor(uint8_t id, binary_sensor::BinarySensor *sensor);
  void register_fall_sensor(uint8_t id, binary_sensor::BinarySensor *sensor);
  void register_velocity_sensor(uint8_t id, sensor::Sensor *sensor);
  void register_x_coordinate_sensor(uint8_t id, sensor::Sensor *sensor);
  void register_y_coordinate_sensor(uint8_t id, sensor::Sensor *sensor);
  void register_z_coordinate_sensor(uint8_t id, sensor::Sensor *sensor);

  // Control functions
  void reset_sensor();
  void set_flash_mode(bool enable);
  void send_config_update(const std::string &command);

 protected:
  // Hardware pins
  GPIOPin *cs_pin_{nullptr};
  GPIOPin *sop2_pin_{nullptr};
  GPIOPin *nrst_pin_{nullptr};

  // Configuration
  uint16_t ceiling_height_{290};  // cm
  uint8_t max_tracks_{5};
  BoundaryBox tracking_boundary_;
  BoundaryBox presence_boundary_;

  // Track data storage
  std::map<uint8_t, TrackData> tracks_;  // ID -> TrackData
  std::map<std::string, uint8_t> tracking_names_;  // Name -> ID

  // Sensors (indexed by ID 1-5)
  std::map<uint8_t, binary_sensor::BinarySensor *> presence_sensors_;
  std::map<uint8_t, binary_sensor::BinarySensor *> fall_sensors_;
  std::map<uint8_t, sensor::Sensor *> velocity_sensors_;
  std::map<uint8_t, sensor::Sensor *> x_coordinate_sensors_;
  std::map<uint8_t, sensor::Sensor *> y_coordinate_sensors_;
  std::map<uint8_t, sensor::Sensor *> z_coordinate_sensors_;

  // Frame parsing
  std::vector<uint8_t> spi_buffer_;
  uint32_t frame_count_{0};
  uint32_t last_frame_time_{0};

  // SPI communication
  bool find_magic_word_spi_();
  bool read_frame_header_(FrameHeader &header);
  bool read_frame_data_(const FrameHeader &header);
  bool parse_tlv_data_(const uint8_t *data, size_t length);

  // UART communication
  void send_uart_command_(const std::string &command);
  void initialize_sensor_config_();
  void update_boundary_config_(const std::string &boundary_type);

  // Data processing
  void process_track_data_(uint8_t radar_id, float x, float y, float z, float vel_x, float vel_y, float vel_z,
                           float confidence);
  void update_sensors_();
  void reset_track_data_(uint8_t id);
  void cleanup_old_tracks_();

  // Fall detection
  bool detect_fall_(const TrackData &track);
  std::map<uint8_t, uint32_t> fall_frame_counters_;  // Track ID -> frames since fall

  // Helper functions
  uint8_t assign_display_id_(uint8_t radar_id);
  bool is_within_boundary_(float x, float y, float z, const BoundaryBox &box);
};

}  // namespace iwr6843
}  // namespace esphome

