![](docs/images/GammaRay.png)
#### [简体中文](docs/Readme_CN.md)

### GammaRay
##### GammaRay is a set of tools for streaming your games and desktop to other devices, and replaying gamepad/keyboard/mouse events in the host PC.

### Main features
- Stream desktop & Replay events
- Start game automatically from clients
- Load & display configuration of Steam games
- Support Steam "Big Picture Mode"
- Audio Spectrum to clients
- Mock "Game Controller"
- Detail debug information

### How to build
#### 1. Clone the repo
```c++
    git clone --recursive 
```

#### 2. Install dependences by VCPKG in 3rdparty.
- 2.1 Change to VCPKG folder
```c++
    cd {your-project-folder}/deps/tc_3rdparty/vcpkg
```
- 2.2 Install vcpkg.exe
```c++
    .\bootstrap-vcpkg.bat 
```
- 2.3 Install dependences
```c++
    .\vcpkg.exe install openssl:x64-windows
    .\vcpkg.exe install gflags:x64-windows
    .\vcpkg.exe install sqlite3:x64-windows
    .\vcpkg.exe install libyuv:x64-windows
    .\vcpkg.exe install detours:x64-windows
    .\vcpkg.exe install gtest-:x64-windows
    .\vcpkg.exe install libvpx:x64-windows
    .\vcpkg.exe install opus:x64-windows
    .\vcpkg.exe install protobuf:x64-windows
    .\vcpkg.exe install ffmpeg:x64-windows
    .\vcpkg.exe install fftw3:x64-windows
    .\vcpkg.exe install poco:x64-windows-static
    .\vcpkg.exe install easyhook:x64-windows
    .\vcpkg.exe install glm:x64-windows
    .\vcpkg install boost:x64-windows
```

1. https://github.com/microsoft/vcpkg/issues/36858
2. change c++17 -> c++20