# Fortress

This repository provides source code to build a TCP/IP client-server system to send and visualize, on a desktop PC,
sensors readings collected by an ESP32 board connected to Wi-Fi.

In this framework, the ESP32 board plays the role of the server (accepts connections) and the desktop application
is the client, (sending the ESP32 a connect request). 

The client has the ability to send custom commands, such as asking
the ESP32 to start reading the ADC at a certain frequency, and continuously send those readings to the client.

---
# 1. Introduction
The whole framework is built in C++. The desktop application is built on top of the Qt graphical library, 
in particular Qt6, with an extensive usage of QtQuick and QML language. Please refer to the [Qt website](https://www.qt.io)
to install Qt6 on your system. Currently, the application works on Mac and Linux. A Windows release will be available soon.

The code for the ESP32 board relies on the Arduino framework, and technically can be compiled with the official Arduino IDE.
However, this leads to some limitation on how header files are imported. Indeed, Arduino IDE requires that all source/header
files are stored within the same directory, but since some networking file are shared between the desktop app and the ESP32 board,
we suggest the usage of Visual Studio Code in tandem with [Platformio](https://platformio.org/platformio-ide).
This simplifies the building workflow, ensuring that all the source files are updated between the server and the client.
It also automatically download the dependencies needed.

[Here](https://docs.platformio.org/en/latest/tutorials/espressif32/arduino_debugging_unit_testing.html)
the official documentation to get started with Platformio and ESP32.

If you prefer to use the Arduino IDE, you must copy all the needed files into the same folder and rename the 
`esp32/src/main.cpp` file to `/your/path/<project_folder>/main.ino`. 
Also, the [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) library is needed.


---
# 2. Folder structure

The project is structured like so:
```
├── app                     # Desktop application entrypoint
├── esp32                   # ESP32 source files
├── include                 # Header files for both desktop app and ESP32
├── qml                     # Desktop QtQuick QML files (frontend)
├── server                  # Entrypoint for a command-line server (for debugging)
├── src                     # Desktop app and command-line server source files 
├── networking_examples     # Examples of a basic client and server system (for learning puporses)
└── test                    # Other tests
```

---
# 3. Building
## 3.1 Desktop App
The simplest way to build everything is using CMake.

First, configure CMake to point the Qt installation folder and set to Release mode, for example:

```bash
cd fortress
mkdir build
cd build
# On Mac
cmake -DCMAKE_PREFIX_PATH=/Users/my_user/Qt/6.1.1/clang_64/lib/cmake -DCMAKE_BUILD_TYPE=Release ../
# On Linux
cmake -DCMAKE_PREFIX_PATH=/home/my_user/Qt/6.1.1/gcc_64/lib/cmake -DCMAKE_BUILD_TYPE=Release ../
```

Note that Qt version could change.

To build:

```bash
# Build all targets, using four cores
cmake --build . -j4
# or build a specific target
cmake --build . -j4 --target Fortress
```

## 3.2 ESP32 (VSCode + Platformio)

The simplest way to build and upload the ESP32 firmware is directly VSCode and Platformio integration.
Simply open the `esp32` folder with VSCode with automatically recognize the ESP32 project. Then press the Build button
to build and the Upload button to flash the ESP.

**Important!** Before build change Wi-Fi parameters in `esp32/src/main.cpp`

You can also compile and flash the ESP32 using the command line.

Hint: first locate your Platformio installation (as Python virtual environment):
```bash
find / -name platformio 2>/dev/null
```

On the author's machine, Platformio is located in `~/.platformio/penv`.

Then, activate the virtual environment:

```bash
 source ~/.platformio/penv/bin/activate
```

Now you can build and upload the firmware
```bash
platformio run                      # Build only
platformio run --target upload      # Build and upload

# Optional
platformio run --target clean       # Remove compiled files
```


## 4. TODOs
- Windows support
- Read real ADC data
- Write ESP32 autonomous logic in case of connection drops