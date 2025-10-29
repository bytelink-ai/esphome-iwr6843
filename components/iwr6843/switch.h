#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "iwr6843.h"

namespace esphome {
namespace iwr6843 {

class IWR6843FlashSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(IWR6843Component *parent) { this->parent_ = parent; }

 protected:
  void write_state(bool state) override {
    if (this->parent_ != nullptr) {
      this->parent_->set_flash_mode(state);
      this->publish_state(state);
    }
  }

  IWR6843Component *parent_{nullptr};
};

}  // namespace iwr6843
}  // namespace esphome

