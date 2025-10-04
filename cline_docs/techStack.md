# WubForge Technology Stack

## Core Technologies
- **JUCE 7.x**: Cross-platform audio plugin framework
- **C++17**: Modern C++ standard for performance and features
- **CMake**: Build system for cross-platform compilation

## Audio Processing Libraries
- **ChowDSP Utils**: Professional DSP components (EQ, filters, dynamics)
- **JUCE DSP**: Built-in audio processing utilities (FFT, IIR, FIR)
- **Custom DSP**: Novel algorithms (Fibonacci Spiral Distort, Fractal Filters)

## Architecture Patterns
- **Modular Design**: 5-slot processing chain with factory pattern
- **MVP Architecture**: Model-View-Processor for GUI separation
- **Observer Pattern**: Parameter changes and real-time updates
- **RAII**: Resource management for audio buffers and states

## Development Tools
- **CLion/Cursor**: Primary IDE for development
- **Valgrind**: Memory leak detection and profiling
- **JUCE Projucer**: GUI layout and resource management
- **Git**: Version control with GitHub integration

### Code Style and Conventions
WubForge adheres to the coding standards and conventions established by the JUCE framework and the ChowDSP library, which are its primary dependencies. Developers should familiarize themselves with these guidelines to maintain consistency across the codebase. Specifically:
-   **JUCE Coding Standards**: Follow the guidelines outlined in the JUCE documentation for C++ style, naming conventions, and best practices.
-   **ChowDSP Style**: When contributing to or extending components related to `chowdsp_utils`, align with its established coding style.
-   **Clang-Format/Clang-Tidy**: While WubForge does not have its own `.clang-format` or `.clang-tidy` configuration files at the project root, it is recommended to use these tools with configurations that align with JUCE and ChowDSP standards for automated formatting and static analysis.

## Audio Specifications
- **Sample Rates**: 44.1kHz - 192kHz support
- **Buffer Sizes**: 32 - 4096 samples
- **Latency**: Zero-latency design where possible
- **CPU Target**: <15% per plugin instance

## Plugin Formats
- **VST3**: Primary target format
- **AU**: macOS Audio Units support
- **AAX**: Pro Tools compatibility (planned)

## Testing Framework
- **JUCE Unit Tests**: Built-in testing framework
- **Audio Validation**: Spectral analysis for algorithm verification
- **Performance Profiling**: CPU and memory usage monitoring

### Testing Guidelines

WubForge utilizes JUCE's built-in unit testing framework for validating individual components and DSP algorithms. The testing philosophy emphasizes:

1.  **Unit-level DSP Verification**: Ensuring the correctness of core DSP algorithms (e.g., filter responses, distortion curves) through precise input/output analysis.
2.  **Audio Thread Safety**: Verifying that all processing is lock-free and safe for real-time audio threads.
3.  **Performance Benchmarking**: Monitoring CPU and memory usage to ensure adherence to performance targets, especially for new modules.
4.  **Functional Testing**: Confirming that modules integrate correctly into the processing chain and respond as expected to parameter changes.

**How to Run Tests:**

Currently, specific test execution commands are integrated into the CMake build process. To run tests:

1.  **Build the project with testing enabled**: This typically involves a CMake configuration that includes `BUILD_TESTING` (though JUCE unit tests might be compiled directly).
    ```bash
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON .. # Or similar flag if available
    cmake --build . --config Debug
    ```
2.  **Execute the test runner**: JUCE unit tests are often compiled into a separate executable. You would typically find and run this executable from your build directory. The exact command may vary, but it often looks like:
    ```bash
    ./WubForge_Tests # (Example, actual name may vary)
    ```
    *Alternatively, if integrated with CTest:*
    ```bash
    ctest
    ```

**How to Write New Tests:**

1.  **Create a new test file**: In a designated `Tests` directory (e.g., `Source/Tests/`), create a new `.h` or `.cpp` file for your tests.
2.  **Include `juce_unit_test_utilities.h`**: This provides the necessary macros for defining tests.
3.  **Define a test class**:
    ```cpp
    class MyModuleTest : public juce::UnitTest
    {
    public:
        MyModuleTest() : juce::UnitTest ("My Module Test", juce::String()) {}

        void runTest() override
        {
            beginTest ("Test Case 1: Basic Functionality");
            // Your test assertions here
            expect (true, "This test should always pass!");

            beginTest ("Test Case 2: Edge Cases");
            // More assertions
        }
    };

    // Register the test
    static MyModuleTest myModuleTest;
    ```
4.  **Add assertions**: Use `expect()`, `expectEquals()`, `expectWithinAbsoluteError()`, etc., to verify expected behavior.
5.  **Integrate with CMake**: Ensure your new test file is added to the CMake build system so it gets compiled and run with the other tests.


## Novel DSP Algorithms
- **Fibonacci Spiral Distort**: Golden ratio harmonic processing
- **Fractal Filters**: Self-similar filtering algorithms
- **Sample Morphing**: Granular synthesis for bass transformation
- **Spectral Processing**: FFT-based effects and analysis

## Performance Optimizations
- **Lock-Free Parameters**: Thread-safe real-time parameter updates
- **Buffer Management**: Efficient audio buffer pooling
- **SIMD Operations**: Vectorized processing where beneficial
- **Memory Alignment**: Cache-friendly data structures
