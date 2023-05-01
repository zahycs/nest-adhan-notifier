# ESP32 Prayer Times Reminder on Google Nest

This project uses an ESP32 microcontroller to retrieve and play Islamic prayer times as reminders on your Google Nest device. The prayer times are retrieved from the Aladhan API and the adhan (call to prayer) is played using MP3 files.

## Usage
1. To use this project, follow these steps:

1. Install the Visual Studio Code editor on your computer.

1. Install the PlatformIO extension for Visual Studio Code.

1. Open the project folder in Visual Studio Code and connect your ESP32 board to your computer.

1. Build and upload the project to your ESP32 board by running the following command in the terminal of Visual Studio Code:

```
pio run -t upload
```
1. After uploading, power on the ESP32 board and wait a few seconds.

1. The ESP32 board will start an Access Point (AP) called `adhan_configurator`. Connect your computer or mobile device to this AP.

1. Open a web browser on your computer or mobile device and navigate to http://adhan.local.

1. On the web page, configure the WiFi settings for the ESP32 board and hit "Save and Restart".

1. The ESP32 board will now connect to your WiFi network and retrieve the prayer times from the Aladhan API. The adhan will be played using the MP3 files at the appropriate times.

## OTA Updates
The firmeware now supports over the air updates, more details about it will be added soon

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.





