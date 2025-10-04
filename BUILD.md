# WubForge Build and Distribution Guide

This comprehensive guide covers building WubForge as a VST3/AU plugin and creating installer packages for distribution.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Build Requirements](#build-requirements)
3. [Build Methods](#build-methods)
4. [Plugin Formats](#plugin-formats)
5. [Distribution and Packaging](#distribution-and-packaging)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Configuration](#advanced-configuration)

## Quick Start

### Option 1: Simple Build (Recommended for Development)
```bash
./build_minimal.sh
```

### Option 2: Full Distribution Build (Recommended for Release)
```bash
./build_and_distribute.sh
```

## Build Requirements

### Minimum Requirements
- **CMake 3.15+**
- **C++17 compatible compiler** (Clang 10+, GCC 7+, MSVC 2019+)
- **Git** (for submodules)

### macOS Specific
- **Xcode Command Line Tools** or full Xcode
- **pkgbuild/productbuild** (for installer packages)

### Optional Tools
- **VST3/AU host** (for testing)
- **Code signing certificate** (for notarized releases)

## Build Methods

### Method 1: Minimal Build Script

The `build_minimal.sh` script provides a simple, fast build process:

```bash
# Clean and build
./build_minimal.sh

# Output location
ls -la dist/1.0.0/macos-wip/
```

**Features:**
- ✅ Fast compilation with 4 parallel jobs
- ✅ Basic error logging and reporting
- ✅ Automatic distribution structure creation
- ✅ Build summary generation

**Output:**
```
dist/1.0.0/macos-wip/
├── VST3/WubForge.vst3/
├── AU/WubForge.component/
└── Standalone/WubForge.app/
```

### Method 2: Full Distribution Script

The `build_and_distribute.sh` script provides comprehensive distribution packaging:

```bash
# Full build with installer
./build_and_distribute.sh

# Output locations
ls -la dist/1.0.0/          # Plugin files
ls -la installers/           # Installer packages
```

**Features:**
- ✅ Complete dependency management
- ✅ Multiple platform support
- ✅ Professional installer packages
- ✅ Documentation generation
- ✅ Checksum verification
- ✅ Post-installation scripts

**Output:**
```
dist/1.0.0/
├── macOS/VST3/WubForge.vst3/
├── macOS/AU/WubForge.component/
├── macOS/Standalone/WubForge.app/
├── docs/
├── presets/
└── README.html

installers/
└── WubForge-1.0.0.pkg
```

### Method 3: Manual CMake Build

For advanced users or custom configurations:

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Install (optional)
make install
```

## Plugin Formats

WubForge supports multiple plugin formats:

### VST3 Plugin
- **Location:** `dist/*/VST3/WubForge.vst3/`
- **Installation:** Copy to `~/Library/Audio/Plug-Ins/VST3/`
- **Compatibility:** All modern DAWs (Cubase, Nuendo, Studio One, etc.)

### Audio Unit (AU)
- **Location:** `dist/*/AU/WubForge.component/`
- **Installation:** Copy to `~/Library/Audio/Plug-Ins/Components/`
- **Compatibility:** Logic Pro, GarageBand, MainStage

### Standalone Application
- **Location:** `dist/*/Standalone/WubForge.app/`
- **Installation:** Copy to `/Applications/`
- **Use:** For testing without a DAW

## Distribution and Packaging

### macOS Installer Package

The distribution script automatically creates a professional installer:

```bash
# Creates installer package
./build_and_distribute.sh

# Manual installer creation
pkgbuild --root pkgbuild/1.0.0 \
         --identifier com.wubforge.wubforge \
         --version 1.0.0 \
         --scripts pkgbuild/scripts \
         installers/WubForge-1.0.0.pkg
```

**Installer Features:**
- ✅ Standard macOS installation experience
- ✅ Proper plugin registration
- ✅ AU cache clearing
- ✅ Post-installation instructions

### Manual Distribution

For custom distribution needs:

```bash
# Create distribution structure
mkdir -p dist/WubForge-1.0.0
cp -r build/WubForge_artefacts/Release/* dist/WubForge-1.0.0/

# Create ZIP archive
cd dist
zip -r WubForge-1.0.0-macOS.zip WubForge-1.0.0/
```

## Troubleshooting

### Common Build Issues

#### 1. CMake Configuration Fails
```bash
# Check CMake version
cmake --version

# Clear build cache
rm -rf build/ && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### 2. Submodule Issues
```bash
# Reinitialize submodules
git submodule update --init --recursive

# Check submodule status
git submodule status
```

#### 3. JUCE Module Not Found
```bash
# Verify JUCE installation
ls -la JUCE/modules/

# Check CMake configuration
cat build/CMakeCache.txt | grep JUCE
```

#### 4. Compilation Errors
```bash
# Check compiler
c++ --version

# Clean rebuild
rm -rf build/ && ./build_minimal.sh
```

### Platform-Specific Issues

#### macOS Issues

**Code Signing:**
```bash
# Enable code signing in CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="..."

# Or disable code signing for development
cmake .. -DCMAKE_BUILD_TYPE=Release -DJUCE_NOTARIZE=OFF
```

**AU Validation:**
```bash
# Validate AU component
auval -v aufx WbFg WubF
```

**VST3 Validation:**
```bash
# Test VST3 in validator
/Applications/Steinberg/VST3PluginTestHost.app/Contents/MacOS/VST3PluginTestHost
```

## Advanced Configuration

### CMake Options

```bash
# Development build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release with optimizations
cmake .. -DCMAKE_BUILD_TYPE=Release

# Universal binary (Intel + Apple Silicon)
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Disable specific formats
cmake .. -DJUCE_BUILD_AUDIOUNIT=OFF  # Disable AU
cmake .. -DJUCE_BUILD_VST3=OFF       # Disable VST3
```

### Custom Build Types

Create custom CMake configuration:

```cmake
# In CMakeLists.txt or toolchain file
set(CMAKE_CXX_FLAGS_CUSTOM "-O2 -DNDEBUG -ffast-math")
set(CMAKE_EXE_LINKER_FLAGS_CUSTOM "-dead_strip")
```

### CI/CD Integration

**GitHub Actions Example:**
```yaml
name: Build WubForge
on: [push, pull_request]

jobs:
  build:
    runs-on: macos-latest
    steps:
    - uses: actions/checkout@v2
      with:
        submodules: recursive
    - name: Build
      run: ./build_and_distribute.sh
    - name: Upload Artifacts
      uses: actions/upload-artifact@v2
      with:
        path: dist/
```

## Distribution Checklist

### Pre-Release
- [ ] Test on multiple DAWs (Logic, Cubase, Studio One)
- [ ] Validate AU component with `auval`
- [ ] Test VST3 in multiple hosts
- [ ] Verify standalone application
- [ ] Check plugin parameters and automation
- [ ] Test with various audio formats

### Release Preparation
- [ ] Update version numbers in scripts
- [ ] Generate checksums for all files
- [ ] Create release notes
- [ ] Test installer package
- [ ] Verify plugin metadata (name, vendor, etc.)

### Post-Release
- [ ] Test installation on clean system
- [ ] Verify plugin appears in DAW menus
- [ ] Check preset loading/saving
- [ ] Validate parameter automation
- [ ] Test with various sample rates

## Support and Resources

### Getting Help
- **GitHub Issues:** Report bugs and feature requests
- **Documentation:** Check `/docs/` folder in releases
- **Build Logs:** Enable verbose logging for troubleshooting

### Resources
- [JUCE Documentation](https://docs.juce.com/)
- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_dev_portal/)
- [Audio Unit Programming Guide](https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/)

---

**WubForge** - Professional spectral bass processing for modern music production.