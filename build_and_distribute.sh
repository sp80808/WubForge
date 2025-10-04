#!/bin/bash

# WubForge Distribution Script
# Builds and packages WubForge for multiple platforms and formats

set -e  # Exit on any error

# Configuration
PROJECT_NAME="WubForge"
VERSION_MAJOR="1"
VERSION_MINOR="0"
VERSION_PATCH="0"
VERSION_STRING="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
BUNDLE_ID="com.wubforge.wubforge"
VENDOR_NAME="WubForge"
VENDOR_WEBSITE="https://github.com/your-repo/wubforge"
DESCRIPTION="Spectral Bass Processor with Real-time Visualization"
COMPANY_NAME="WubForge Audio"

# Platform detection
OS_TYPE=$(uname -s)
ARCH_TYPE=$(uname -m)

echo "🍺 WubForge Distribution Script"
echo "================================"
echo "Version: ${VERSION_STRING}"
echo "Platform: ${OS_TYPE}/${ARCH_TYPE}"
echo "Date: $(date)"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

log_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

log_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

log_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Check for required tools
check_requirements() {
    log_info "Checking required tools..."

    command -v cmake >/dev/null 2>&1 || { log_error "cmake is required but not installed."; exit 1; }
    command -v make >/dev/null 2>&1 || { log_error "make is required but not installed."; exit 1; }

    if [[ "$OS_TYPE" == "Darwin" ]]; then
        command -v pkgbuild >/dev/null 2>&1 || { log_error "pkgbuild is required for macOS packages."; exit 1; }
        command -v productbuild >/dev/null 2>&1 || { log_error "productbuild is required for macOS packages."; exit 1; }
    fi

    log_success "All required tools are available"
}

# Initialize and update submodules
prepare_submodules() {
    log_info "Initializing and updating Git submodules..."

    git submodule update --init --recursive --depth 1 JUCE
    git submodule update --init --recursive --depth 1 chowdsp_utils
    git submodule update --init --recursive --depth 1 JUMP
    git submodule update --init --recursive --depth 1 mda-plugins-juce

    log_success "Submodules updated"
}

# Clean previous builds
clean_build() {
    log_info "Cleaning previous builds..."

    rm -rf build/
    rm -rf dist/
    rm -rf installers/

    log_success "Clean completed"
}

# Configure CMake build
configure_build() {
    local build_type=$1
    local build_dir="build/${build_type,,}"

    log_info "Configuring ${build_type} build..."

    mkdir -p "$build_dir"

    pushd "$build_dir"

    cmake ../.. \
        -DCMAKE_BUILD_TYPE="$build_type" \
        -DCMAKE_PREFIX_PATH="$(brew --prefix)" \
        -DJUCE_BUILD_EXTRAS=ON \
        -DJUCE_BUILD_EXAMPLES=OFF \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

    popd

    log_success "${build_type} build configured"
}

# Build the project
build_project() {
    local build_type=$1
    local build_dir="build/${build_type,,}"

    log_info "Building ${build_type} configuration..."

    pushd "$build_dir"

    if [[ "$OS_TYPE" == "Darwin" ]]; then
        make -j$(sysctl -n hw.ncpu) # Use all available CPU cores
    else
        make -j$(nproc) # Linux
    fi

    popd

    log_success "${build_type} build completed"
}

# Create distribution directory structure
create_dist_structure() {
    log_info "Creating distribution structure..."

    mkdir -p "dist/${VERSION_STRING}/macOS/VST3"
    mkdir -p "dist/${VERSION_STRING}/macOS/AU"
    mkdir -p "dist/${VERSION_STRING}/macOS/Standalone"
    mkdir -p "dist/${VERSION_STRING}/docs"
    mkdir -p "dist/${VERSION_STRING}/presets"

    log_success "Distribution structure created"
}

# Copy built plugins to distribution directory
copy_artifacts() {
    local build_type=$1
    local build_dir="build/${build_type,,}"

    log_info "Copying built artifacts..."

    # VST3 Plugin
    if [[ -d "${build_dir}/WubForge_artefacts/${build_type}/VST3/${PROJECT_NAME}.vst3" ]]; then
        cp -r "${build_dir}/WubForge_artefacts/${build_type}/VST3/${PROJECT_NAME}.vst3" \
              "dist/${VERSION_STRING}/macOS/VST3/"
        log_success "VST3 plugin copied"
    fi

    # AU Plugin
    if [[ -d "${build_dir}/WubForge_artefacts/${build_type}/AU/${PROJECT_NAME}.component" ]]; then
        cp -r "${build_dir}/WubForge_artefacts/${build_type}/AU/${PROJECT_NAME}.component" \
              "dist/${VERSION_STRING}/macOS/AU/"
        log_success "AU plugin copied"
    fi

    # Standalone App
    if [[ -d "${build_dir}/WubForge_artefacts/${build_type}/Standalone/${PROJECT_NAME}.app" ]]; then
        cp -r "${build_dir}/WubForge_artefacts/${build_type}/Standalone/${PROJECT_NAME}.app" \
              "dist/${VERSION_STRING}/macOS/Standalone/"
        log_success "Standalone app copied"
    fi
}

# Create macOS installer package
create_macos_package() {
    log_info "Creating macOS installer package..."

    local package_root="pkgbuild/${VERSION_STRING}"
    local scripts_dir="pkgbuild/scripts"

    # Create package structure
    mkdir -p "$package_root/Library/Audio/Plug-Ins/VST3"
    mkdir -p "$package_root/Library/Audio/Plug-Ins/Components"
    mkdir -p "$scripts_dir"

    # Copy plugins
    cp -r "dist/${VERSION_STRING}/macOS/VST3/${PROJECT_NAME}.vst3" \
          "$package_root/Library/Audio/Plug-Ins/VST3/" 2>/dev/null || true
    cp -r "dist/${VERSION_STRING}/macOS/AU/${PROJECT_NAME}.component" \
          "$package_root/Library/Audio/Plug-Ins/Components/" 2>/dev/null || true

    # Create postinstall script
    cat > "$scripts_dir/postinstall" << EOF
#!/bin/bash
# WubForge Post-Installation Script

echo "WubForge v${VERSION_STRING} installed successfully!"
echo "Please restart your DAW to load the new plugins."

# Clear AU cache (macOS specific)
if command -v auval > /dev/null 2>&1; then
    auval -a 2>/dev/null || true
fi

exit 0
EOF

    chmod +x "$scripts_dir/postinstall"

    # Create package
    pkgbuild --root "$package_root" \
             --identifier "$BUNDLE_ID" \
             --version "$VERSION_STRING" \
             --scripts "$scripts_dir" \
             --install-location "/" \
             "installers/${PROJECT_NAME}-${VERSION_STRING}.pkg"

    log_success "macOS installer package created"

    # Clean up
    rm -rf pkgbuild/
}

# Create comprehensive documentation
create_documentation() {
    log_info "Creating documentation..."

    # Create README for distribution
    cat > "dist/${VERSION_STRING}/README.html" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>WubForge v${VERSION_STRING}</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        h1, h2 { color: #2c3e50; }
        pre { background: #f8f8f8; padding: 10px; border-radius: 5px; }
        .version { color: #e74c3c; font-weight: bold; }
        .feature { margin: 10px 0; }
        .warning { background: #fff3cd; border: 1px solid #ffeaa7; padding: 15px; border-radius: 5px; margin: 15px 0; }
    </style>
</head>
<body>
    <h1>WubForge <span class="version">v${VERSION_STRING}</span></h1>

    <p><strong>${DESCRIPTION}</strong></p>

    <p><strong>Company:</strong> ${COMPANY_NAME}<br>
    <strong>Website:</strong> <a href="${VENDOR_WEBSITE}">${VENDOR_WEBSITE}</a></p>

    <h2>What's New in v${VERSION_STRING}</h2>

    <div class="feature">🎵 **Professional Spectral Processing** - Real-time FFT-based spectral morphing with fractal filtering</div>
    <div class="feature">🎛️ **Modular Processing Chain** - 4-slot system with key-tracked parameters</div>
    <div class="feature">📊 **Real-time Visualization** - Waterfall spectrogram with multiple color schemes</div>
    <div class="feature">🎸 **MDA SubSynth Integration** - Classic bass enhancement from legendary MDA plugins</div>
    <div class="feature">🎚️ **Professional EQ** - 3-band parametric equalizer using chowdsp_utils</div>
    <div class="feature">🎸 **Advanced Distortion** - Multi-model distortion with Hematone-style processing</div>

    <h2>System Requirements</h2>
    <ul>
        <li>macOS 10.13 or later</li>
        <li>Windows 10 or later (future release)</li>
        <li>Linux Ubuntu 18.04 or later (future release)</li>
        <li>Any VST3/AU compatible host application</li>
    </ul>

    <h2>Installation</h2>
    <p>Run the installer package and follow the on-screen instructions. The plugins will be installed to the standard locations:</p>
    <pre>
~/Library/Audio/Plug-Ins/VST3/ (VST3)
~/Library/Audio/Plug-Ins/Components/ (AU)
    </pre>

    <h2>Quick Start</h2>
    <ol>
        <li>Launch your DAW</li>
        <li>Create a new track and insert WubForge</li>
        <li>Send bass audio to the track</li>
        <li>Adjust the fractal filter for harmonic richness</li>
        <li>Use the spectrogram to visualize frequency content changes</li>
    </ol>

    <div class="warning">
        <strong>Note:</strong> After installation, restart your DAW if it was running during installation.
    </div>

    <h2>Technical Details</h2>
    <ul>
        <li><strong>Architecture:</strong> x86_64 + Apple Silicon Universal Binary</li>
        <li><strong>Processing:</strong> Real-time, <5ms typical latency</li>
        <li><strong>Formats:</strong> VST3, AU, Standalone</li>
        <li><strong>Libraries:</strong> JUCE, chowdsp_utils, MDA algorithms</li>
    </ul>

    <h2>Support</h2>
    <p>For support, feature requests, or bug reports, please visit our GitHub repository.</p>

    <p><em>WubForge - Elevating bass processing with spectral innovation</em></p>
</body>
</html>
EOF

    # Copy documentation files
    cp README.md "dist/${VERSION_STRING}/docs/" 2>/dev/null || true
    cp -r cline_docs/* "dist/${VERSION_STRING}/docs/" 2>/dev/null || true

    log_success "Documentation created"
}

# Create checksums for verification
create_checksums() {
    log_info "Creating checksums for verification..."

    pushd "dist/${VERSION_STRING}"

    # Create SHA256 checksums for all files
    find . -type f -not -name "*.sha256" | sort | while read -r file; do
        if command -v sha256sum >/dev/null 2>&1; then
            sha256sum "$file" >> "checksums.sha256"
        elif command -v shasum >/dev/null 2>&1; then
            shasum -a 256 "$file" >> "checksums.sha256"
        fi
    done

    popd

    log_success "Checksums created"
}

# Main build process
main() {
    check_requirements
    prepare_submodules

    # Build Release configuration
    clean_build
    configure_build "Release"
    build_project "Release"

    # Create distribution structure
    create_dist_structure
    copy_artifacts "Release"

    # Create macOS-specific installer
    if [[ "$OS_TYPE" == "Darwin" ]]; then
        create_macos_package
    fi

    # Create documentation and checksums
    create_documentation
    create_checksums

    log_success "🚀 Distribution package created successfully!"
    log_info "Output location: $(pwd)/dist/${VERSION_STRING}"

    echo ""
    echo "📦 Package Contents:"
    echo "├── macOS/VST3/${PROJECT_NAME}.vst3"
    echo "├── macOS/AU/${PROJECT_NAME}.component"
    echo "├── macOS/Standalone/${PROJECT_NAME}.app"
    echo "├── docs/"
    echo "├── presets/"
    echo "├── README.html"
    echo "└── checksums.sha256"

    if [[ "$OS_TYPE" == "Darwin" ]]; then
        echo "└── ../../installers/${PROJECT_NAME}-${VERSION_STRING}.pkg"
    fi

    echo ""
    log_success "Ready for distribution!"
}

# Run main function
main "$@"
