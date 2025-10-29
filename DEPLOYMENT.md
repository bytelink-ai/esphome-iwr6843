# Deployment Guide

Complete guide for deploying the ESPHome IWR6843 component to GitHub and using it in production.

## GitHub Repository Setup

### 1. Create GitHub Repository

```bash
# On GitHub, create new repository: esphome-iwr6843

# Initialize local repository
cd /path/to/esphome-iwr6843
git init
git add .
git commit -m "Initial commit: ESPHome IWR6843 custom component"

# Add remote and push
git remote add origin https://github.com/yourusername/esphome-iwr6843.git
git branch -M main
git push -u origin main
```

### 2. Repository Settings

#### Topics (for discoverability)
Add these topics to your repository:
- `esphome`
- `home-assistant`
- `iwr6843`
- `mmwave-radar`
- `people-tracking`
- `fall-detection`
- `esp32`
- `custom-component`

#### Description
```
ESPHome custom component for TI IWR6843 mmWave Radar with 3D People Tracking and Fall Detection
```

#### About Section
- Website: Link to documentation (GitHub Pages or Wiki)
- Check: ✓ Releases, ✓ Packages

### 3. Branch Protection (Optional)

For `main` branch:
- Require pull request reviews before merging
- Require status checks to pass
- Include administrators in restrictions

## Release Process

### Creating a Release

1. **Update Version**
   
   Create/update `version.py`:
   ```python
   """Version information for esphome-iwr6843"""
   __version__ = "1.0.0"
   ```

2. **Tag Release**
   ```bash
   git tag -a v1.0.0 -m "Release v1.0.0: Initial stable release"
   git push origin v1.0.0
   ```

3. **Create GitHub Release**
   - Go to Releases → Draft a new release
   - Choose tag: `v1.0.0`
   - Release title: `v1.0.0 - Initial Release`
   - Description: Include changelog and features

### Release Checklist

- [ ] All tests pass
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Version number updated
- [ ] Example YAML tested
- [ ] Hardware tested on real device

### Semantic Versioning

Follow [SemVer](https://semver.org/):

- **MAJOR** (1.0.0): Breaking changes
  - Changed API
  - Removed features
  - Different pin configurations

- **MINOR** (0.1.0): New features
  - New sensors
  - New configuration options
  - Backward compatible changes

- **PATCH** (0.0.1): Bug fixes
  - Bug fixes
  - Performance improvements
  - Documentation updates

## Using the Component

### Method 1: External Component (Recommended)

In your ESPHome YAML:

```yaml
external_components:
  - source: github://yourusername/esphome-iwr6843
    components: [ iwr6843 ]
```

Or specific version:

```yaml
external_components:
  - source: github://yourusername/esphome-iwr6843@v1.0.0
    components: [ iwr6843 ]
```

### Method 2: Local Copy

1. **Download component**
   ```bash
   cd /config/esphome
   git clone https://github.com/yourusername/esphome-iwr6843.git
   ```

2. **Reference in YAML**
   ```yaml
   external_components:
     - source: /config/esphome/esphome-iwr6843/components
       components: [ iwr6843 ]
   ```

### Method 3: Git Submodule

```bash
cd /config/esphome
git submodule add https://github.com/yourusername/esphome-iwr6843.git
git submodule update --init --recursive
```

## CI/CD Setup (GitHub Actions)

### Automated Testing

Create `.github/workflows/ci.yml`:

```yaml
name: CI

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'
      
      - name: Install ESPHome
        run: pip install esphome
      
      - name: Compile example
        run: esphome compile examples/basic.yaml
      
      - name: Check code formatting
        run: |
          pip install black flake8
          black --check components/iwr6843/*.py
          flake8 components/iwr6843/*.py
```

### Auto-Release

Create `.github/workflows/release.yml`:

```yaml
name: Release

on:
  push:
    tags:
      - 'v*'

jobs:
  release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Create Release
        uses: actions/create-release@v1
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
        with:
          tag_name: ${{ github.ref }}
          release_name: Release ${{ github.ref }}
          draft: false
          prerelease: false
```

## Documentation Deployment

### GitHub Pages

1. **Enable GitHub Pages**
   - Settings → Pages
   - Source: Deploy from branch
   - Branch: `main` / `docs` folder

2. **Create documentation site** (optional)
   ```bash
   pip install mkdocs mkdocs-material
   mkdocs new .
   # Edit mkdocs.yml and docs/
   mkdocs build
   mkdocs gh-deploy
   ```

### Wiki

Use GitHub Wiki for:
- Advanced configuration examples
- Troubleshooting guides
- Community contributions
- FAQ

## Production Deployment

### Hardware Setup

1. **Prepare hardware**
   - ESP32 board
   - IWR6843 radar
   - Power supply (5V, 1A minimum)
   - Connecting wires

2. **Wire connections**
   - Follow `HARDWARE_GUIDE.md`
   - Double-check all connections
   - Verify power supply voltage

### Software Setup

1. **Create ESPHome configuration**
   ```bash
   cd /config/esphome
   esphome wizard radar.yaml
   ```

2. **Add component configuration**
   - Copy from `examples/basic.yaml`
   - Adjust pin numbers if needed
   - Configure WiFi credentials

3. **Compile and flash**
   ```bash
   esphome run radar.yaml
   ```

### Home Assistant Integration

1. **Auto-discovery**
   - ESPHome device should appear in Integrations
   - Click "Configure" to add

2. **Manual integration**
   ```yaml
   # configuration.yaml
   esphome:
     # No configuration needed if using auto-discovery
   ```

3. **Create dashboard**
   - Add Person entities to dashboard
   - Use Plotly Graph Card for visualization
   - Set up automations for fall detection

## Monitoring and Maintenance

### Log Monitoring

```bash
# ESPHome logs
esphome logs radar.yaml

# Home Assistant logs
tail -f /config/home-assistant.log | grep iwr6843
```

### Update Process

1. **Check for updates**
   ```bash
   git fetch origin
   git log HEAD..origin/main --oneline
   ```

2. **Update component**
   - Automatic: ESPHome will pull latest on compile
   - Manual: Increment version in YAML

3. **Test update**
   - Compile with new version
   - Verify all entities still work
   - Check for breaking changes in CHANGELOG

### Backup

Backup your configuration:
```bash
# ESPHome YAML
cp radar.yaml radar.yaml.backup

# Home Assistant config
# (use Home Assistant backups)
```

## Troubleshooting Deployment

### Compilation Errors

```
Error: Component 'iwr6843' not found
```
**Solution**: Check `external_components` URL and component name

```
Error: SPI device not found
```
**Solution**: Ensure SPI configuration is present in YAML

### Runtime Errors

```
[E][iwr6843:123]: SPI frame timeout
```
**Solution**: Check SPI wiring and radar power

```
[W][iwr6843:456]: UART command failed
```
**Solution**: Check UART wiring and baud rate

### Home Assistant Issues

**Entities not appearing**
- Check ESPHome logs for errors
- Verify Home Assistant integration is active
- Restart Home Assistant

**Stale data**
- Check "Last Seen" timestamp in ESPHome dashboard
- Verify radar is powered and running
- Check network connectivity

## Support and Community

### Getting Help

1. **Check documentation**
   - README.md
   - HARDWARE_GUIDE.md
   - STRUCTURE.md

2. **Search issues**
   - GitHub Issues
   - ESPHome GitHub Issues
   - Home Assistant Community

3. **Ask for help**
   - Open GitHub Issue
   - ESPHome Discord: #custom-components
   - Home Assistant Community Forum

### Contributing Back

If you:
- Fix a bug → Submit PR
- Add a feature → Discuss in issue first
- Improve docs → PR welcome!
- Find an issue → Report it

## Security Considerations

### Network Security

- Use WPA2/WPA3 for WiFi
- Set strong OTA password
- Enable API encryption
- Use VLAN for IoT devices (recommended)

### Data Privacy

- Radar data stays local (no cloud)
- All processing on ESP32
- Home Assistant controls access

### Physical Security

- Mount radar securely
- Protect ESP32 from tampering
- Use enclosure for production deployment

## License and Credits

### License
MIT License - Free to use, modify, and distribute

### Credits
- Texas Instruments: IWR6843 radar and SDK
- ESPHome: Framework and community
- Contributors: See CONTRIBUTORS.md

## Next Steps

After deployment:

1. **Monitor stability** (first 24 hours)
2. **Fine-tune boundaries** for your space
3. **Create automations** in Home Assistant
4. **Document your setup** (photos, configs)
5. **Share your experience** (blog, forum post)

## Resources

- [TI IWR6843 Product Page](https://www.ti.com/product/IWR6843)
- [ESPHome Documentation](https://esphome.io/)
- [Home Assistant](https://www.home-assistant.io/)
- [Project GitHub](https://github.com/yourusername/esphome-iwr6843)

---

**Questions?** Open an issue on GitHub!

