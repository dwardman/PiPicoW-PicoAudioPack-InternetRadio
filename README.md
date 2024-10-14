# WFMU RP2040 Pimoroni Internet Radio

This is an Arduino code Fork of wfmu-rp2040-pimoroni-radio with console logging improvements and dual core.  Code is currently streaming SomaFM and includes enhancements for better WiFi connectivity and metadata logging.

Future plans include push buttons for 5 radio station options and Wifi and station configuration through serial or Web interface.

## Bill of Materials (BOM)

- [Raspberry Pi Pico W](https://shop.pimoroni.com/products/raspberry-pi-pico-w) - The main board.
- [Pimoroni Pico Audio Pack](https://shop.pimoroni.com/products/pico-audio-pack) - I2S audio board.
- [Pico Audio Pack Case](https://www.thingiverse.com/thing:6404203) - Case Designed by YuuichiAkagawa

## Software / Library Requirements

- Arduino IDE
- [Arduino-Pico](https://arduino-pico.readthedocs.io/en/latest/) — Follow the [installation instructions](https://learn.adafruit.com/rp2040-arduino-with-the-earlephilhower-core/installing-the-earlephilhower-core) for setting up.
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio/) — Most of the audio and streaming code comes from this library, modified for the RP2040. Despite its name (i.e., ESP8266), it works with RP2040 and I2S boards.

## Key Improvements

This fork includes several enhancements:

1. **Dual-Core Operation**: The project utilizes both cores of the Raspberry Pi Pico W. Core 0 manages WiFi connectivity and logging, while Core 1 handles audio streaming, allowing for smoother performance.
2. **WiFi Connection Stability**: Improved handling of WiFi connection attempts and error reporting. The code now provides detailed feedback in the serial monitor, including reasons for failed connections.
3. **Metadata Logging**: The code captures and logs song metadata every few seconds while streaming audio. This includes the current song title, artist, and streaming status.
4. **Date and Time Logging**: The current date and time are printed alongside runtime, RSSI (Received Signal Strength Indicator), and metadata. The time is adjusted for Central Time, accounting for Daylight Saving Time.
5. **Buffer Management**: Adjustments made to improve handling of audio buffer underflow issues, ensuring smoother playback.
6. **Continuous WiFi Monitoring**: The code maintains a connection to WiFi even when the MP3 stream stops, preventing unnecessary reconnection attempts.

## Usage Instructions

1. **Clone or Download** this repository to your local machine.
2. **Open the Code**: Use the Arduino IDE to open the `.ino` file located in the repository.
3. **Configure WiFi Credentials**: In the code, replace the placeholders for `STASSID` and `STAPSK` with your actual WiFi SSID and password.
4. **Library Installation**: Ensure that you have the required libraries installed in your Arduino IDE:
   - Install the Arduino-Pico core via the Arduino Boards Manager.
   - Install the ESP8266Audio library from the Arduino Library Manager or download it from the [GitHub repository](https://github.com/earlephilhower/ESP8266Audio/).
5. **Compile and Upload**: Connect your Raspberry Pi Pico W to your computer and upload the code.
6. **Open Serial Monitor**: After uploading, open the Serial Monitor in the Arduino IDE to see the output logs, including connection attempts, song metadata, and status updates.

## Additional Notes

- **Custom Audio Sources**: If you wish to use this code with other audio streams, you can modify the URLs in the code.
- **Using ESP8266Audio for Other Projects**: Instead of compiling this repo's files directly, you can also copy (overwrite) the `AudioFileSourceHTTPStream.*` and `AudioFileSourceICYStream.*` files from this repo to the locally installed ESP8266Audio library (typically located in `~/Arduino/libraries/ESP8266Audio/src/`). This will allow you to compile existing ESP8266Audio examples with RP2040 and I2S boards.

## License

This project is licensed under the MIT License. Feel free to modify and distribute as you wish.
Pico Audio Pack Case by YuuichiAkagawa is licensed under the Creative Commons - Attribution - Non-Commercial - Share Alike license.
