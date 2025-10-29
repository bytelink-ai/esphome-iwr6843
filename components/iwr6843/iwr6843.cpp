#include "iwr6843.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome {
namespace iwr6843 {

static const char *const TAG = "iwr6843";

void IWR6843Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up IWR6843...");

  // Initialize SPI interface
  this->spi_setup();
  ESP_LOGCONFIG(TAG, "SPI interface initialized");
  
  // Initialize SOP2 pin (functional mode)
  if (this->sop2_pin_ != nullptr) {
    this->sop2_pin_->setup();
    this->sop2_pin_->digital_write(true);  // HIGH = functional mode
    ESP_LOGCONFIG(TAG, "SOP2 pin set to functional mode (HIGH)");
  }

  // Initialize NRST pin
  if (this->nrst_pin_ != nullptr) {
    this->nrst_pin_->setup();
    this->nrst_pin_->digital_write(true);  // HIGH = not in reset
    ESP_LOGCONFIG(TAG, "NRST pin initialized (HIGH)");
  }

  // Reset sensor to ensure clean state
  this->reset_sensor();
  delay(500);

  // Initialize sensor configuration via UART
  this->initialize_sensor_config_();

  ESP_LOGCONFIG(TAG, "IWR6843 setup complete");
}

void IWR6843Component::loop() {
  static uint32_t last_debug_time = 0;
  uint32_t current_time = millis();
  
  // Debug: Log every 5 seconds if no data
  if (current_time - last_debug_time > 5000) {
    ESP_LOGD(TAG, "SPI Loop active, frame_count=%u, last_frame_time=%u ms ago", 
             this->frame_count_, current_time - this->last_frame_time_);
    last_debug_time = current_time;
  }
  
  // Read frame from SPI
  if (this->find_magic_word_spi_()) {
    ESP_LOGD(TAG, "Magic word found!");
    FrameHeader header;
    if (this->read_frame_header_(header)) {
      ESP_LOGD(TAG, "Frame header read: frame=%u, length=%u, tlvs=%u", 
               header.frame_number, header.total_packet_len, header.num_tlvs);
      if (this->read_frame_data_(header)) {
        this->frame_count_++;
        this->update_sensors_();
        ESP_LOGD(TAG, "Frame %u processed successfully", this->frame_count_);
        
        // Cleanup old tracks every 50 frames
        if (this->frame_count_ % 50 == 0) {
          this->cleanup_old_tracks_();
        }
      }
    }
  }

  // Reset all tracks to 0 if no valid frames received for 2 seconds
  if (current_time - this->last_frame_time_ > 2000) {
    for (auto &pair : this->tracks_) {
      this->reset_track_data_(pair.first);
    }
    this->update_sensors_();
  }
}

void IWR6843Component::dump_config() {
  ESP_LOGCONFIG(TAG, "IWR6843 mmWave Radar:");
  ESP_LOGCONFIG(TAG, "  Ceiling Height: %d cm", this->ceiling_height_);
  ESP_LOGCONFIG(TAG, "  Max Tracks: %d", this->max_tracks_);
  ESP_LOGCONFIG(TAG, "  Tracking Boundary: X[%.1f, %.1f] Y[%.1f, %.1f] Z[%.1f, %.1f]",
                this->tracking_boundary_.x_min, this->tracking_boundary_.x_max,
                this->tracking_boundary_.y_min, this->tracking_boundary_.y_max,
                this->tracking_boundary_.z_min, this->tracking_boundary_.z_max);
  ESP_LOGCONFIG(TAG, "  Presence Boundary: X[%.1f, %.1f] Y[%.1f, %.1f] Z[%.1f, %.1f]",
                this->presence_boundary_.x_min, this->presence_boundary_.x_max,
                this->presence_boundary_.y_min, this->presence_boundary_.y_max,
                this->presence_boundary_.z_min, this->presence_boundary_.z_max);
}

// Configuration functions
void IWR6843Component::set_tracking_boundary(float x_min, float x_max, float y_min, float y_max, float z_min,
                                              float z_max) {
  this->tracking_boundary_ = {x_min, x_max, y_min, y_max, z_min, z_max};
}

void IWR6843Component::set_presence_boundary(float x_min, float x_max, float y_min, float y_max, float z_min,
                                              float z_max) {
  this->presence_boundary_ = {x_min, x_max, y_min, y_max, z_min, z_max};
}

void IWR6843Component::add_tracking_id(uint8_t id, const std::string &name) {
  this->tracking_names_[name] = id;
  
  // Initialize track data
  TrackData track = {0};
  track.id = id;
  track.is_present = false;
  track.is_fallen = false;
  track.last_seen = 0;
  this->tracks_[id] = track;
}

// Sensor registration
void IWR6843Component::register_presence_sensor(uint8_t id, binary_sensor::BinarySensor *sensor) {
  this->presence_sensors_[id] = sensor;
}

void IWR6843Component::register_fall_sensor(uint8_t id, binary_sensor::BinarySensor *sensor) {
  this->fall_sensors_[id] = sensor;
}

void IWR6843Component::register_velocity_sensor(uint8_t id, sensor::Sensor *sensor) {
  this->velocity_sensors_[id] = sensor;
}

void IWR6843Component::register_x_coordinate_sensor(uint8_t id, sensor::Sensor *sensor) {
  this->x_coordinate_sensors_[id] = sensor;
}

void IWR6843Component::register_y_coordinate_sensor(uint8_t id, sensor::Sensor *sensor) {
  this->y_coordinate_sensors_[id] = sensor;
}

void IWR6843Component::register_z_coordinate_sensor(uint8_t id, sensor::Sensor *sensor) {
  this->z_coordinate_sensors_[id] = sensor;
}

// Control functions
void IWR6843Component::reset_sensor() {
  if (this->nrst_pin_ == nullptr)
    return;

  ESP_LOGI(TAG, "Resetting sensor...");
  this->nrst_pin_->digital_write(false);  // Assert reset (active LOW)
  delay(100);
  this->nrst_pin_->digital_write(true);  // Release reset
  delay(500);
}

void IWR6843Component::set_flash_mode(bool enable) {
  if (this->sop2_pin_ == nullptr)
    return;

  // SOP2: LOW = flash mode, HIGH = functional mode
  this->sop2_pin_->digital_write(!enable);
  ESP_LOGI(TAG, "Flash mode: %s", enable ? "enabled" : "disabled");
}

// UART communication
void IWR6843Component::send_uart_command_(const std::string &command) {
  if (!this->available()) {
    ESP_LOGW(TAG, "UART not available for command: %s", command.c_str());
    return;
  }

  ESP_LOGD(TAG, "Sending UART command: %s", command.c_str());
  this->write_str(command.c_str());
  uart::UARTDevice::write_byte('\n');  // Explicitly use UART write_byte
  delay(50);  // Wait for command processing
}

void IWR6843Component::send_config_update(const std::string &command) {
  ESP_LOGI(TAG, "Updating configuration: %s", command.c_str());
  
  // Stop sensor
  this->send_uart_command_("sensorStop");
  delay(100);
  
  // Send update command
  this->send_uart_command_(command);
  delay(100);
  
  // Start sensor
  this->send_uart_command_("sensorStart");
  delay(200);
  
  ESP_LOGI(TAG, "Configuration updated successfully");
}

void IWR6843Component::initialize_sensor_config_() {
  ESP_LOGI(TAG, "Initializing sensor configuration...");
  
  // Send initial configuration commands
  this->send_uart_command_("sensorStop");
  delay(200);
  
  this->send_uart_command_("flushCfg");
  delay(100);
  
  // Send all configuration commands
  this->send_uart_command_("dfeDataOutputMode 1");
  this->send_uart_command_("channelCfg 15 7 0");
  this->send_uart_command_("adcCfg 2 1");
  this->send_uart_command_("adcbufCfg -1 0 1 1 1");
  this->send_uart_command_("lowPower 0 0");
  
  // Chirp configuration
  this->send_uart_command_("chirpCfg 0 0 0 0 0 0 0 1");
  this->send_uart_command_("chirpCfg 1 1 0 0 0 0 0 2");
  this->send_uart_command_("chirpCfg 2 2 0 0 0 0 0 4");
  
  // Frame configuration
  this->send_uart_command_("frameCfg 0 2 224 0 120.00 1 0");
  
  // CFAR configuration
  this->send_uart_command_("dynamicRACfarCfg -1 10 1 1 1 8 8 6 4 4.00 6.00 0.50 1 1");
  this->send_uart_command_("staticRACfarCfg -1 4 4 2 2 8 16 4 6 6.00 13.00 0.50 0 0");
  
  // Angle configuration
  this->send_uart_command_("dynamicRangeAngleCfg -1 7.000 0.0010 2 0");
  this->send_uart_command_("dynamic2DAngleCfg -1 5 1 1 1.00 15.00 2");
  this->send_uart_command_("staticRangeAngleCfg -1 0 1 1");
  
  // Antenna geometry
  this->send_uart_command_("antGeometry0 -1 -1 0 0 -3 -3 -2 -2 -1 -1 0 0");
  this->send_uart_command_("antGeometry1 -1 0 -1 0 -3 -2 -3 -2 -3 -2 -3 -2");
  this->send_uart_command_("antPhaseRot 1 -1 1 -1 1 -1 1 -1 1 -1 1 -1");
  
  // FOV configuration
  this->send_uart_command_("fovCfg -1 64.0 64.0");
  this->send_uart_command_("compRangeBiasAndRxChanPhase 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0");
  
  // Boundaries (use configured values)
  char boundary_cmd[256];
  snprintf(boundary_cmd, sizeof(boundary_cmd), "boundaryBox %.1f %.1f %.1f %.1f %.1f %.1f",
           this->tracking_boundary_.x_min, this->tracking_boundary_.x_max,
           this->tracking_boundary_.y_min, this->tracking_boundary_.y_max,
           this->tracking_boundary_.z_min, this->tracking_boundary_.z_max);
  this->send_uart_command_(boundary_cmd);
  
  snprintf(boundary_cmd, sizeof(boundary_cmd), "presenceBoundaryBox %.1f %.1f %.1f %.1f %.1f %.1f",
           this->presence_boundary_.x_min, this->presence_boundary_.x_max,
           this->presence_boundary_.y_min, this->presence_boundary_.y_max,
           this->presence_boundary_.z_min, this->presence_boundary_.z_max);
  this->send_uart_command_(boundary_cmd);
  
  // Sensor position (ceiling height in meters, 90° tilt)
  float height_m = this->ceiling_height_ / 100.0f;
  char sensor_pos_cmd[64];
  snprintf(sensor_pos_cmd, sizeof(sensor_pos_cmd), "sensorPosition %.1f 0 90", height_m);
  this->send_uart_command_(sensor_pos_cmd);
  
  // Tracking configuration
  this->send_uart_command_("gatingParam 3 2 2 3 4");
  this->send_uart_command_("stateParam 3 3 6 20 3 1000");
  char alloc_cmd[64];
  snprintf(alloc_cmd, sizeof(alloc_cmd), "allocationParam %d %d 0.05 %d 1.5 %d",
           this->max_tracks_, this->max_tracks_, this->max_tracks_, this->max_tracks_);
  this->send_uart_command_(alloc_cmd);
  this->send_uart_command_("maxAcceleration 1 0.1 1");
  this->send_uart_command_("trackingCfg 1 4 800 20 37 33 120 1");
  
  // Start sensor
  delay(200);
  this->send_uart_command_("sensorStart");
  delay(500);
  
  ESP_LOGI(TAG, "Sensor configuration complete");
}

void IWR6843Component::update_boundary_config_(const std::string &boundary_type) {
  char cmd[256];
  
  if (boundary_type == "tracking") {
    snprintf(cmd, sizeof(cmd), "boundaryBox %.1f %.1f %.1f %.1f %.1f %.1f",
             this->tracking_boundary_.x_min, this->tracking_boundary_.x_max,
             this->tracking_boundary_.y_min, this->tracking_boundary_.y_max,
             this->tracking_boundary_.z_min, this->tracking_boundary_.z_max);
  } else if (boundary_type == "presence") {
    snprintf(cmd, sizeof(cmd), "presenceBoundaryBox %.1f %.1f %.1f %.1f %.1f %.1f",
             this->presence_boundary_.x_min, this->presence_boundary_.x_max,
             this->presence_boundary_.y_min, this->presence_boundary_.y_max,
             this->presence_boundary_.z_min, this->presence_boundary_.z_max);
  }
  
  this->send_config_update(cmd);
}

// SPI Frame Reading
bool IWR6843Component::find_magic_word_spi_() {
  static uint32_t last_log_time = 0;
  static uint32_t attempt_count = 0;
  
  // Read bytes and search for magic word
  uint8_t buffer[128];
  size_t bytes_read = 0;
  
  // CS is automatically handled by SPIDevice::enable() and disable()
  this->enable();
  
  // Search for magic word
  while (bytes_read < 128) {
    this->spi_read_array_(&buffer[bytes_read], 1);  // Read 1 byte via SPI (explicit)
    bytes_read++;
    
    // Check if we found the magic word
    if (bytes_read >= MAGIC_WORD_SIZE) {
      bool found = true;
      for (size_t i = 0; i < MAGIC_WORD_SIZE; i++) {
        if (buffer[bytes_read - MAGIC_WORD_SIZE + i] != MAGIC_WORD[i]) {
          found = false;
          break;
        }
      }
      
      if (found) {
        // Found magic word, store remaining bytes in buffer
        this->spi_buffer_.clear();
        for (size_t i = bytes_read - MAGIC_WORD_SIZE; i < bytes_read; i++) {
          this->spi_buffer_.push_back(buffer[i]);
        }
        ESP_LOGV(TAG, "Magic word found after %u bytes", bytes_read);
        return true;
      }
    }
  }
  
  // Disable CS
  this->disable();
  
  // Debug logging every 10 seconds
  attempt_count++;
  uint32_t now = millis();
  if (now - last_log_time > 10000) {
    ESP_LOGW(TAG, "No magic word found in %u attempts (last 10s). First 16 bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
             attempt_count,
             buffer[0], buffer[1], buffer[2], buffer[3],
             buffer[4], buffer[5], buffer[6], buffer[7],
             buffer[8], buffer[9], buffer[10], buffer[11],
             buffer[12], buffer[13], buffer[14], buffer[15]);
    last_log_time = now;
    attempt_count = 0;
  }
  
  return false;
}

bool IWR6843Component::read_frame_header_(FrameHeader &header) {
  // Read rest of header (40 - 8 = 32 bytes)
  size_t bytes_needed = FRAME_HEADER_SIZE - this->spi_buffer_.size();
  
  for (size_t i = 0; i < bytes_needed; i++) {
    uint8_t byte;
    this->spi_read_array_(&byte, 1);  // Read 1 byte via SPI (explicit)
    this->spi_buffer_.push_back(byte);
  }
  
  // Parse header
  if (this->spi_buffer_.size() < FRAME_HEADER_SIZE) {
    return false;
  }
  
  // Extract header fields (little-endian)
  memcpy(&header.magic_word, &this->spi_buffer_[0], 8);
  memcpy(&header.version, &this->spi_buffer_[8], 4);
  memcpy(&header.total_packet_len, &this->spi_buffer_[12], 4);
  memcpy(&header.platform, &this->spi_buffer_[16], 4);
  memcpy(&header.frame_number, &this->spi_buffer_[20], 4);
  memcpy(&header.time_cpu_cycles, &this->spi_buffer_[24], 4);
  memcpy(&header.num_detected_obj, &this->spi_buffer_[28], 4);
  memcpy(&header.num_tlvs, &this->spi_buffer_[32], 4);
  memcpy(&header.subframe_number, &this->spi_buffer_[36], 4);
  
  // Sanity check
  if (header.total_packet_len > MAX_FRAME_SIZE || header.total_packet_len < FRAME_HEADER_SIZE) {
    ESP_LOGW(TAG, "Invalid frame length: %d", header.total_packet_len);
    return false;
  }
  
  return true;
}

bool IWR6843Component::read_frame_data_(const FrameHeader &header) {
  // Read remaining frame data
  size_t remaining_bytes = header.total_packet_len - FRAME_HEADER_SIZE;
  
  for (size_t i = 0; i < remaining_bytes; i++) {
    uint8_t byte;
    this->spi_read_array_(&byte, 1);  // Read 1 byte via SPI (explicit)
    this->spi_buffer_.push_back(byte);
  }
  
  // Disable CS
  this->disable();
  
  // Parse TLV data
  bool success = this->parse_tlv_data_(&this->spi_buffer_[FRAME_HEADER_SIZE], remaining_bytes);
  
  if (success) {
    this->last_frame_time_ = millis();
  }
  
  return success;
}

bool IWR6843Component::parse_tlv_data_(const uint8_t *data, size_t length) {
  size_t offset = 0;
  
  while (offset + 8 <= length) {
    // Read TLV header
    uint32_t tlv_type, tlv_length;
    memcpy(&tlv_type, &data[offset], 4);
    memcpy(&tlv_length, &data[offset + 4], 4);
    offset += 8;
    
    // Sanity check
    if (offset + tlv_length > length) {
      ESP_LOGW(TAG, "TLV length exceeds frame boundary");
      return false;
    }
    
    // Process TLV based on type
    if (tlv_type == TLVTYPE_TRACKED_TARGETS) {
      // Parse track data
      size_t num_tracks = tlv_length / 68;  // Each track is 68 bytes
      
      for (size_t i = 0; i < num_tracks && i < this->max_tracks_; i++) {
        size_t track_offset = offset + (i * 68);
        
        // Extract track data (68 bytes per track)
        uint32_t radar_id_raw;
        float x, y, z, vel_x, vel_y, vel_z, confidence;
        
        memcpy(&radar_id_raw, &data[track_offset], 4);
        uint8_t radar_id = (uint8_t) radar_id_raw;
        memcpy(&x, &data[track_offset + 4], 4);
        memcpy(&y, &data[track_offset + 8], 4);
        memcpy(&z, &data[track_offset + 12], 4);
        memcpy(&vel_x, &data[track_offset + 16], 4);
        memcpy(&vel_y, &data[track_offset + 20], 4);
        memcpy(&vel_z, &data[track_offset + 24], 4);
        memcpy(&confidence, &data[track_offset + 44], 4);  // Confidence at offset 44
        
        // Process track
        this->process_track_data_(radar_id, x, y, z, vel_x, vel_y, vel_z, confidence);
      }
    }
    
    offset += tlv_length;
  }
  
  return true;
}

// Data processing
void IWR6843Component::process_track_data_(uint8_t radar_id, float x, float y, float z, float vel_x, float vel_y,
                                            float vel_z, float confidence) {
  // Assign display ID (1-5)
  uint8_t display_id = this->assign_display_id_(radar_id);
  
  if (display_id == 0 || display_id > 5) {
    return;  // Invalid ID
  }
  
  // Check if within presence boundary
  bool is_present = this->is_within_boundary_(x, y, z, this->presence_boundary_);
  
  // Update track data
  TrackData &track = this->tracks_[display_id];
  track.radar_id = radar_id;
  track.x = x;
  track.y = y;
  track.z = z;
  track.vel_x = vel_x;
  track.vel_y = vel_y;
  track.vel_z = vel_z;
  track.confidence = confidence;
  track.is_present = is_present;
  track.last_seen = this->frame_count_;
  
  // Fall detection
  track.is_fallen = this->detect_fall_(track);
  
  ESP_LOGD(TAG, "Track ID %d (Radar %d): X=%.2f Y=%.2f Z=%.2f Vel=%.2f Present=%d Fallen=%d",
           display_id, radar_id, x, y, z, vel_z, is_present, track.is_fallen);
}

void IWR6843Component::update_sensors_() {
  // Update all sensor values
  for (uint8_t id = 1; id <= 5; id++) {
    auto it = this->tracks_.find(id);
    
    if (it != this->tracks_.end() && it->second.is_present) {
      const TrackData &track = it->second;
      
      // Update binary sensors
      if (this->presence_sensors_.count(id)) {
        this->presence_sensors_[id]->publish_state(true);
      }
      if (this->fall_sensors_.count(id)) {
        this->fall_sensors_[id]->publish_state(track.is_fallen);
      }
      
      // Update numeric sensors (convert to cm and mm/s)
      if (this->x_coordinate_sensors_.count(id)) {
        this->x_coordinate_sensors_[id]->publish_state(track.x * 100.0f);  // m to cm
      }
      if (this->y_coordinate_sensors_.count(id)) {
        this->y_coordinate_sensors_[id]->publish_state(track.y * 100.0f);  // m to cm
      }
      if (this->z_coordinate_sensors_.count(id)) {
        this->z_coordinate_sensors_[id]->publish_state(track.z * 100.0f);  // m to cm
      }
      if (this->velocity_sensors_.count(id)) {
        this->velocity_sensors_[id]->publish_state(track.vel_z * 1000.0f);  // m/s to mm/s
      }
    } else {
      // Track not present - reset to 0
      this->reset_track_data_(id);
    }
  }
}

void IWR6843Component::reset_track_data_(uint8_t id) {
  // Set presence to clear
  if (this->presence_sensors_.count(id)) {
    this->presence_sensors_[id]->publish_state(false);
  }
  if (this->fall_sensors_.count(id)) {
    this->fall_sensors_[id]->publish_state(false);
  }
  
  // Set all numeric values to 0
  if (this->x_coordinate_sensors_.count(id)) {
    this->x_coordinate_sensors_[id]->publish_state(0.0f);
  }
  if (this->y_coordinate_sensors_.count(id)) {
    this->y_coordinate_sensors_[id]->publish_state(0.0f);
  }
  if (this->z_coordinate_sensors_.count(id)) {
    this->z_coordinate_sensors_[id]->publish_state(0.0f);
  }
  if (this->velocity_sensors_.count(id)) {
    this->velocity_sensors_[id]->publish_state(0.0f);
  }
  
  // Reset track data in memory
  if (this->tracks_.count(id)) {
    this->tracks_[id].is_present = false;
    this->tracks_[id].is_fallen = false;
  }
}

void IWR6843Component::cleanup_old_tracks_() {
  // Remove tracks not seen in last 50 frames
  for (auto &pair : this->tracks_) {
    if (this->frame_count_ - pair.second.last_seen > 50) {
      pair.second.is_present = false;
    }
  }
}

// Fall detection (simplified version)
bool IWR6843Component::detect_fall_(const TrackData &track) {
  // Simple fall detection based on Z coordinate drop and velocity
  const float FALL_Z_THRESHOLD = 0.5f;  // meters
  const float FALL_VEL_THRESHOLD = -1.0f;  // m/s (downward)
  
  // Check if track has fallen based on height and velocity
  if (track.z < FALL_Z_THRESHOLD && track.vel_z < FALL_VEL_THRESHOLD) {
    // Increment fall counter
    this->fall_frame_counters_[track.id]++;
    
    // Trigger fall detection if fallen for 3+ frames
    if (this->fall_frame_counters_[track.id] >= 3) {
      return true;
    }
  } else {
    // Reset fall counter
    this->fall_frame_counters_[track.id] = 0;
  }
  
  return false;
}

// Helper functions
uint8_t IWR6843Component::assign_display_id_(uint8_t radar_id) {
  // Simple ID assignment: use sequential IDs 1-5
  // In a real implementation, use position-based tracking for stability
  
  // Find existing mapping
  for (const auto &pair : this->tracks_) {
    if (pair.second.radar_id == radar_id) {
      return pair.first;
    }
  }
  
  // Assign new ID (find first available)
  for (uint8_t id = 1; id <= 5; id++) {
    if (this->tracks_[id].radar_id == 0 || !this->tracks_[id].is_present) {
      return id;
    }
  }
  
  return 0;  // No available IDs
}

bool IWR6843Component::is_within_boundary_(float x, float y, float z, const BoundaryBox &box) {
  return (x >= box.x_min && x <= box.x_max && y >= box.y_min && y <= box.y_max && z >= box.z_min && z <= box.z_max);
}

}  // namespace iwr6843
}  // namespace esphome

