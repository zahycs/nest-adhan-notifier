# ESP32 Prayer Times Reminder on Google nest

This is an Arduino sketch that uses an ESP8266 microcontroller to retrieve and play Islamic prayer times as reminders. The prayer times are retrieved from the Aladhan API and the adhan (call to prayer) is played using MP3 files.

## Configuration

To use this sketch, you will need to configure the following parameters:

### WiFi credentials

```cpp
const char *ssid = "Your SSID";
const char *password = "your password";
```

Replace Your SSID and your password with the SSID and password of your WiFi network.

### Aladhan API endpoint and parameters

```cpp
const char *api_endpoint = "https://api.aladhan.com/v1/timingsByCity";
const char *city = "Utrecht";
const char *country = "NL";
const int method = 5; // for egyptian general authority of survey
```

Replace Utrecht and NL with the name of your city and country. You can also change the method parameter to a different value to use a different calculation method.

### Adhan MP3 files
The MP3 files used for the adhan can be configured in the AdhanPlayer.cpp file. The file contains an array of 5 URLs, one for each prayer time:

```cpp
const char *adhan_urls[] = {
  "https://example.com/fajr.mp3",
  "https://example.com/dhuhr.mp3",
  "https://example.com/asr.mp3",
  "https://example.com/maghrib.mp3",
  "https://example.com/isha.mp3"
};
```

Replace the URLs with the URLs of your own MP3 files.

### Aladhan API configuration

The API endpoint and parameters are used in the prayerApiClient.cpp file. You can modify the following variables to change the API configuration:

```cpp
const char *api_endpoint = "https://api.aladhan.com/v1/timingsByCity";
const char *api_city = "Utrecht";
const char *api_country = "NL";
const int api_method = 5; // for egyptian general authority of survey
```

## Usage

Once you have configured the sketch with your WiFi credentials, API endpoint and parameters, and adhan MP3 files, you can upload it to your ESP8266 microcontroller using the Arduino IDE. The microcontroller will then connect to your WiFi network and retrieve the prayer times from the Aladhan API. The adhan will be played using the MP3 files at the appropriate times.

## Credits

This sketch uses the following libraries:

* ArduinoJson
* WiFi
* HTTPClient
* ESP8266-Google-Home-Notifier