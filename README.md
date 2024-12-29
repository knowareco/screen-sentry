# Screen Sentry

The Screen Sentry is designed around the ESP32-S3 SoC that comes preinstalled on the [Arduino Nano ESP32](https://store-usa.arduino.cc/products/nano-esp32)
variant.

#### Developing
To get started with Screen Sentry, you will first need to install several dependencies:

- [ESP-IDF](https://idf.espressif.com/)
- [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/index.html)
- [Node.js/NPM](https://nodejs.org/en)

#### Installing ESP-IDF
Installing the ESP-IDF toolchain for the ESP32-S3 is relatively straightforward if you are using Eclipse or VS Code as
your IDE. You can also manually install the toolchain for other editors such as neovim. Either way, following the instructions
on the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html#installation).

#### Installing The PlatformIO Core (CLI)
If you are using Mac, you can quickly install the PlatformIO Core via Homebrew:


```
brew install platformio
```

Otherwise you can follow your platform specific instructions on the [PlatformIO Core documentation][https://docs.platformio.org/en/latest/core/installation/index.html].

#### Generate compile_commands.json
`make compiledb`

#### Flash the Microcontroller
Build with:
`make build`

Flash with:
`make flash`
