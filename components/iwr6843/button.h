#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "iwr6843.h"

namespace esphome {
namespace iwr6843 {

class IWR6843ResetButton : public button::Button, public Component {
 public:
  void set_parent(IWR6843Component *parent) { this->parent_ = parent; }

 protected:
  void press_action() override {
    if (this->parent_ != nullptr) {
      this->parent_->reset_sensor();
    }
  }

  IWR6843Component *parent_{nullptr};
};

}  // namespace iwr6843
}  // namespace esphome

