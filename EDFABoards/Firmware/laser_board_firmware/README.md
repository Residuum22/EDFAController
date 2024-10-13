# Laser Controller Board Firmware

## Prerequsites

- Visual Studio Code
- ESP-IDF extension


## Setting Up for Development

1. Open `Commad Palette` in VSCode (`F1` or `CNTR + Shift + P`) and search for `ESP-IDF: Configure ESP-IDF extension`.
2. Choose `Express` option and install `v.5.1.1 (released)` version.
3. Open `Command Palette` again and `ESP-IDF: Configure Paths`
   1. `IDF_PATH`: (Example on Windows): "C:\Users\mihal\esp\esp-idf"
   2. `IDF_TOOLS_PATH`: (Example on Windows): "C:\Espressif"
4. Open `Command Palette` and choose flash method with the `ESP-IDF: Select flash method` command.
   1. For UART flash: `UART`.
   2. For JTAG flash (and lated debug): `JTAG`.
5. To configure target open `Command Palette` and `ESP-IDF: Set Espressif device target`.
   1. Choose your workspace (should be `laser_board_firmware`).
   2. Choose `esp32` from the dropdown menu.
   3. Choose `ESP32 chip (via ESP-PROG)`
6. [Optional] If you are flashing with JTAG then you need to configure OpenOCD as well
    Add the following lines to the `settings.json` file  
    ```json
    "idf.openOcdDebugLevel": 0, // Less debug log
    "idf.openOcdLaunchArgs": [
        "-c adapter speed 5000", // Default 20 MHz is way too high -> Connection will be unstable
        "-c set ESP32_FLASH_VOLTAGE 1.8", // ESP32-WROVER kit has 1.8V flash
        "-c adapter srst pulse_width 500" // Sightly increase rst pulse width
    ],
    ```

## Configurable Parameter

### CONFIG_BROKER_URL

### CONFIG_EXAMPLE_WIFI_SSID

### CONFIG_EXAMPLE_WIFI_PASSWORD