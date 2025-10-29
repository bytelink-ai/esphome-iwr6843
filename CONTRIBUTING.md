# Contributing to ESPHome IWR6843

Thank you for your interest in contributing! This document provides guidelines for contributing to the ESPHome IWR6843 custom component.

## How to Contribute

### Reporting Issues

If you find a bug or have a feature request:

1. **Check existing issues** to avoid duplicates
2. **Create a new issue** with:
   - Clear title and description
   - Steps to reproduce (for bugs)
   - Expected vs actual behavior
   - ESPHome version and hardware details
   - Relevant logs or error messages

### Pull Requests

We welcome pull requests! Please follow these steps:

1. **Fork the repository**
2. **Create a feature branch** (`git checkout -b feature/amazing-feature`)
3. **Make your changes**
4. **Test thoroughly** (see Testing section below)
5. **Commit your changes** (`git commit -m 'Add amazing feature'`)
6. **Push to your fork** (`git push origin feature/amazing-feature`)
7. **Open a Pull Request**

### Code Style

#### Python Code
- Follow [PEP 8](https://pep8.org/)
- Use 4 spaces for indentation
- Maximum line length: 100 characters
- Use meaningful variable names

```python
# Good
def register_presence_sensor(person_id, sensor):
    """Register a presence sensor for a specific person ID."""
    self.presence_sensors_[person_id] = sensor

# Bad
def reg_sens(id, s):
    self.ps[id] = s
```

#### C++ Code
- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use 2 spaces for indentation
- Place `{` on same line as statement
- Use `snake_case_` for member variables

```cpp
// Good
void IWR6843Component::update_sensors_() {
  for (uint8_t id = 1; id <= 5; id++) {
    if (this->presence_sensors_.count(id)) {
      this->presence_sensors_[id]->publish_state(true);
    }
  }
}

// Bad
void IWR6843Component::UpdateSensors(){
  for(uint8_t id=1;id<=5;id++){
    if(this->PresenceSensors.count(id)){
      this->PresenceSensors[id]->PublishState(true);
    }
  }
}
```

#### YAML Files
- Use 2 spaces for indentation
- Add comments for complex configurations
- Keep lines under 100 characters

### Testing

Before submitting a pull request:

1. **Compile test**
   ```bash
   esphome compile examples/basic.yaml
   ```

2. **Hardware test** (if possible)
   - Flash to ESP32
   - Verify all entities appear in Home Assistant
   - Test sensor updates
   - Test configuration changes

3. **Functional test**
   - Verify SPI communication works
   - Verify UART configuration updates work
   - Test reset button and flash mode switch
   - Verify presence and fall detection

### Documentation

Update documentation for:
- New features or entities
- Configuration changes
- Pin changes
- API changes

### Commit Messages

Use clear, descriptive commit messages:

```
Good:
- "Add velocity sensor for each tracked person"
- "Fix SPI frame parsing for TLV type 8"
- "Update README with hardware troubleshooting"

Bad:
- "fix bug"
- "update"
- "changes"
```

## Development Setup

### Prerequisites

- Python 3.9+
- ESPHome 2023.12.0+
- ESP32 development board
- IWR6843 radar module

### Local Development

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/esphome-iwr6843.git
   cd esphome-iwr6843
   ```

2. **Create test YAML**
   ```bash
   cp examples/basic.yaml test.yaml
   # Edit test.yaml with your WiFi credentials
   ```

3. **Test compilation**
   ```bash
   esphome compile test.yaml
   ```

4. **Flash to ESP32**
   ```bash
   esphome upload test.yaml
   ```

5. **Monitor logs**
   ```bash
   esphome logs test.yaml
   ```

### Debugging

#### Enable Debug Logging

In your test YAML:
```yaml
logger:
  level: DEBUG
  logs:
    iwr6843: VERBOSE
```

#### SPI Debugging

Add debug prints in `iwr6843.cpp`:
```cpp
ESP_LOGD(TAG, "SPI byte read: 0x%02X", byte);
```

#### UART Debugging

Monitor UART traffic:
```cpp
ESP_LOGD(TAG, "UART TX: %s", command.c_str());
```

## Feature Requests

We're open to new features! Before implementing:

1. **Open an issue** describing the feature
2. **Discuss the approach** with maintainers
3. **Wait for approval** before starting work
4. **Implement and test**
5. **Submit PR with documentation**

### Feature Ideas

Current feature ideas:
- [ ] Advanced fall detection parameters
- [ ] Custom tracking zones
- [ ] Historical data logging
- [ ] Multi-radar support
- [ ] Kalman filter for smoother tracking
- [ ] Gesture recognition
- [ ] Activity classification

## Code Review Process

1. **Automated checks** must pass:
   - Compilation
   - Code formatting
   - Linting

2. **Manual review** by maintainers:
   - Code quality
   - Documentation
   - Testing coverage

3. **Feedback and iteration**
   - Address review comments
   - Update PR as needed

4. **Merge**
   - Squash commits (maintainer will handle)
   - Merge to main branch

## Release Process

Releases follow semantic versioning:
- **Major** (1.0.0): Breaking changes
- **Minor** (0.1.0): New features
- **Patch** (0.0.1): Bug fixes

## Community

- **GitHub Discussions**: General questions and ideas
- **GitHub Issues**: Bug reports and feature requests
- **ESPHome Discord**: Real-time chat and support

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

If you have questions about contributing, feel free to:
- Open a discussion on GitHub
- Ask in ESPHome Discord
- Contact maintainers directly

Thank you for contributing! 🎉

