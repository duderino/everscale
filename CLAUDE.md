# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Everscale is a high-performance HTTP proxy server written in C++17. It's built on the ESB (Everscale Base) foundation library which provides networking, memory management, and system utilities. The project focuses on performance, scalability, and modularity for enterprise HTTP proxying needs.

## Build System and Commands

### Build Configuration
- **Build System**: CMake with custom build profiles
- **Primary Build Command**: `cmake . && make`
- **Test Command**: `make test` or `ctest`

### Build Profiles
Set build profile via: `cmake -DCMAKE_BUILD_TYPE=<TYPE> .`

Available build types:
- `DEBUG` (default): Debug with leak sanitizer
- `RELEASE`: Optimized production build  
- `ASAN`: Address sanitizer build
- `TSAN`: Thread sanitizer build
- `DEBUGNOPOOL`: Debug without memory pools (uses malloc/free directly)
- `RELEASENOPOOL`: Release without memory pools
- `COVERAGE`: Code coverage build with profiling

### Common Development Commands
```bash
# Clean build from scratch
make clean && cmake . && make

# Run all tests
make test

# Run specific test
./base/buddy-allocator-test

# Format code (requires clang-format)
make format

# Build specific components
make base                    # Build base library
make http-proxy             # Build HTTP proxy executable  
make http-origin            # Build HTTP origin server
make http-loadgen           # Build load generator

# Run integration tests
./data-plane/proxy/http-proxy-test
```

### Dependencies
- **External**: BoringSSL, GoogleTest, yajl JSON parser, clang-format, mocha (for Node.js tests)
- **Automatically fetched**: Dependencies are downloaded and built by CMake

## Architecture

### High-Level Structure
```
base/              - ESB foundation library (networking, memory, utilities)
data-plane/        - HTTP processing components
├── config/        - Configuration system  
├── http-common/   - Shared HTTP abstractions
├── http1/         - HTTP/1.1 implementation
├── proxy/         - HTTP proxy logic
├── origin/        - HTTP origin server
├── loadgen/       - Load generation tools
└── multiplexers/  - Event loop abstractions
unit-tf/           - Unit testing framework
tests/             - Integration tests
```

### Core Components

#### ESB Base Library (`base/`)
- **Memory Management**: Custom allocators (buddy, discard, shared)
- **Networking**: Socket abstractions, TLS support, multiplexers
- **Utilities**: Thread pools, timing wheels, containers, JSON parsing
- **Key Classes**: `ESBAllocator`, `ESBSocket`, `ESBMultiplexer`, `ESBBuffer`

#### Data Plane (`data-plane/`)
- **HTTP Stack**: Parser/formatter, client/server sockets, transactions
- **Proxy Engine**: Routing, load balancing, connection pooling
- **Configuration**: JSON/YAML-based rule engine
- **Key Classes**: `HttpProxy`, `HttpClientSocket`, `HttpServerSocket`, `HttpTransaction`

### Memory Management
- Uses custom allocators to avoid malloc/free overhead
- **Buddy Allocator**: For fixed-size allocations
- **Discard Allocator**: For request-lifetime allocations  
- **Shared Allocators**: For cross-thread data structures
- Build with `*NOPOOL` variants to disable custom allocators for debugging

### Threading Model
- Multi-threaded with epoll-based event loops
- Each thread runs its own multiplexer (event loop)
- Lock-free data structures where possible
- Thread pools for CPU-intensive tasks

## Testing

### Test Types
- **Unit Tests**: GoogleTest-based (`*-gtest` executables)
- **Component Tests**: Custom unit-tf framework (`*-test` executables)  
- **Integration Tests**: Multi-process HTTP scenarios
- **Interop Tests**: Node.js-based cross-language tests

### Running Tests
```bash
# All tests
make test

# Specific test categories
ctest -R "gtest"           # GoogleTest tests only
ctest -R "integration"     # Integration tests only

# Individual test execution
./base/buddy-allocator-gtest
./data-plane/proxy/http-proxy-test
```

### Test Configuration
- Timeout: Most tests have 30-second timeouts
- Coverage: Available via `COVERAGE` build type
- Sanitizers: Use `ASAN`/`TSAN` builds for memory/thread error detection

## Key Development Patterns

### Error Handling
- Uses `ESB::Error` enum for error codes
- Return `ESB_SUCCESS` for success, specific error codes for failures
- No exceptions (compiled with `-fno-exceptions`)

### Memory Management
- Use placement new with allocators: `new(allocator) ClassName()`
- Objects have cleanup handlers for proper destruction
- RAII patterns with smart pointers where appropriate

### Networking
- Asynchronous I/O with epoll
- State machines for protocol handling
- Buffer management for zero-copy where possible

### Configuration
- JSON-based configuration with validation
- Support for runtime configuration updates
- Rule-based routing and processing

## File Locations

### Build Artifacts
- Libraries: `base/libbase.a`, `data-plane/*/lib*.a`
- Executables: `data-plane/proxy/http-proxy`, `data-plane/origin/http-origin`
- Tests: Various `*-test` and `*-gtest` executables

### Configuration Examples
- Test configs: `data-plane/config/tests/config.json`
- TLS certificates: `base/tests/*.crt`, `base/tests/*.key`

### Performance Data
- Performance logs: `perf-logs/`
- TODO items: `TODO` (comprehensive development roadmap)

## Common Issues and Debugging

### TLS/Certificate Issues
- Recent BoringSSL updates may cause certificate validation failures
- Check `TODO` file for known TLS-related issues
- Test certificates available in `base/tests/`

### Memory Debugging
- Use `DEBUGNOPOOL` build to disable custom allocators
- Address sanitizer builds help catch memory issues
- Check for allocator cleanup handler registration

### Performance Analysis
- Use `RELEASE` builds for performance testing
- Coverage builds available for profiling
- Performance logs tracked in `perf-logs/`