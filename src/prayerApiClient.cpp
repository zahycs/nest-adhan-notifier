#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

class PrayerApiClient
{
private:
  HTTPClient httpClient;
  const int num_prayers = 5;

  struct tm *getLocalPrayerTimes(String json)
  {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, json);
    const char *prayer_names[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};

    const JsonObject &data = doc["data"];
    const JsonObject &timings = data["timings"];

    struct tm *prayer_times = new tm[5];

    for (int i = 0; i < num_prayers; i++)
    {
      const char *prayer_name = prayer_names[i];
      const char *prayer_time_str = timings[prayer_name];
      sscanf(prayer_time_str, "%d:%d", &prayer_times[i].tm_hour, &prayer_times[i].tm_min);
      prayer_times[i].tm_sec = 0;
      prayer_times[i].tm_isdst = -1;
    }

    return prayer_times;
  }

public:
  struct tm *getPrayerTimes(String city, String country, int method)
  {
    String url = "https://api.aladhan.com/v1/timingsByCity?city=" + city + "&country=" + country + "&method=" + String(method);
    Serial.println("endpoint: " + String(url));

    // Set the headers
    httpClient.addHeader("Host", "api.aladhan.com");
    httpClient.addHeader("Accept", "application/json");
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int retryCount = 0;

    // Try 3 times to get the response, as this api is sometimes glitchy
    while (retryCount < 3)
    {
      retryCount++;
      // Send the HTTP GET request
      httpClient.begin(url);
      int statusCode = httpClient.GET();

      // Check the response status code
      if (statusCode != HTTP_CODE_OK)
      {
        Serial.println("Request failed , statusCode:" + String(statusCode));
        return nullptr;
      }
      else
      {
        break;
      }
    }

    // Get the response body
    String response = httpClient.getString();

    // Close the connection
    httpClient.end();
    Serial.println(response);
    // Return the response
    return getLocalPrayerTimes(response);
  }
};
