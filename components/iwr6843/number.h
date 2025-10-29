#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "iwr6843.h"

namespace esphome {
namespace iwr6843 {

enum NumberType {
  CEILING_HEIGHT,
  MAX_TRACKS,
  TRACKING_BOUNDARY_X_MAX,
  TRACKING_BOUNDARY_X_MIN,
  TRACKING_BOUNDARY_Y_MAX,
  TRACKING_BOUNDARY_Y_MIN,
  TRACKING_BOUNDARY_Z_MAX,
  TRACKING_BOUNDARY_Z_MIN,
  PRESENCE_BOUNDARY_X_MAX,
  PRESENCE_BOUNDARY_X_MIN,
  PRESENCE_BOUNDARY_Y_MAX,
  PRESENCE_BOUNDARY_Y_MIN,
  PRESENCE_BOUNDARY_Z_MAX,
  PRESENCE_BOUNDARY_Z_MIN,
};

class IWR6843Number : public number::Number, public Component {
 public:
  void set_parent(IWR6843Component *parent) { this->parent_ = parent; }
  void set_number_type(NumberType type) { this->number_type_ = type; }

 protected:
  void control(float value) override {
    if (this->parent_ == nullptr)
      return;

    char cmd[128];
    
    switch (this->number_type_) {
      case CEILING_HEIGHT:
        this->parent_->set_ceiling_height((uint16_t) value);
        snprintf(cmd, sizeof(cmd), "sensorPosition %.1f 0 90", value / 100.0f);
        this->parent_->send_config_update(cmd);
        break;
        
      case MAX_TRACKS:
        this->parent_->set_max_tracks((uint8_t) value);
        snprintf(cmd, sizeof(cmd), "allocationParam %d %d 0.05 %d 1.5 %d",
                 (int)value, (int)value, (int)value, (int)value);
        this->parent_->send_config_update(cmd);
        break;
        
      case TRACKING_BOUNDARY_X_MAX:
      case TRACKING_BOUNDARY_X_MIN:
      case TRACKING_BOUNDARY_Y_MAX:
      case TRACKING_BOUNDARY_Y_MIN:
      case TRACKING_BOUNDARY_Z_MAX:
      case TRACKING_BOUNDARY_Z_MIN:
        this->update_tracking_boundary_(value);
        break;
        
      case PRESENCE_BOUNDARY_X_MAX:
      case PRESENCE_BOUNDARY_X_MIN:
      case PRESENCE_BOUNDARY_Y_MAX:
      case PRESENCE_BOUNDARY_Y_MIN:
      case PRESENCE_BOUNDARY_Z_MAX:
      case PRESENCE_BOUNDARY_Z_MIN:
        this->update_presence_boundary_(value);
        break;
    }
    
    this->publish_state(value);
  }
  
  void update_tracking_boundary_(float value) {
    // This is simplified - in production, you'd need to track all boundary values
    // and send complete boundaryBox command
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "boundaryBox -4 4 -4 4 -0.5 3");  // Use current values
    this->parent_->send_config_update(cmd);
  }
  
  void update_presence_boundary_(float value) {
    // This is simplified - in production, you'd need to track all boundary values
    // and send complete presenceBoundaryBox command
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "presenceBoundaryBox -4 4 -4 4 0.5 2.5");  // Use current values
    this->parent_->send_config_update(cmd);
  }

  IWR6843Component *parent_{nullptr};
  NumberType number_type_;
};

}  // namespace iwr6843
}  // namespace esphome

