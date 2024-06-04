#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "models.h"
#include "PrayerTimesInterface.h"

class AladhanPrayerTimes : public PrayerTimesInterface
{
private:
  WiFiClientSecure client;
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
  
  String getRequest(String url)
  {
    // Add the necessary headers
    httpClient.addHeader("Accept", "application/json");
    httpClient.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int retryCount = 0;

    while (retryCount < 3)
    {
      retryCount++;
      // Send the HTTP GET request
      httpClient.begin(client, url);  // Using the secured client
      int statusCode = httpClient.GET();

      // Check the response status code
      if (statusCode == HTTP_CODE_OK)
      {
        // Get the response body
        String response = httpClient.getString();

        // Close the connection
        httpClient.end();

        return response;
      }
      else
      {
        Serial.println("Request failed, statusCode: " + String(statusCode));
      }
    }

    // Close the connection
    httpClient.end();
    return "";
  }

public:
  AladhanPrayerTimes()
  {
    client.setInsecure();  // Ignore SSL certificate errors
  }

  struct tm *getPrayerTimes(String city, String country, int method) override
  {
    String url = "https://api.aladhan.com/v1/timingsByCity?city=" + city + "&country=" + country + "&method=" + String(method);
    Serial.println("endpoint: " + String(url));

    String response = getRequest(url);
    if (response.isEmpty())
    {
      return nullptr;
    }

    return getLocalPrayerTimes(response);
  }

    MethodList* getMethods() override {
        Serial.println("Getting calculation methods...");
        String url = "https://api.aladhan.com/v1/methods";
        String response = getRequest(url);
        if (response.isEmpty()) {
            Serial.println("Failed to get the methods");
            return nullptr;
        }

        // Deserialize the JSON response
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.c_str());
            return nullptr;
        }

        JsonObject data = doc["data"];
        int num_responses = data.size();
        Method* methods = new Method[num_responses];
        int i = 0;
        Serial.println("Number of methods: " + String(num_responses));

        // Use a range-based for loop
        for (JsonPair kv : data) {
            JsonObject methodJson = kv.value().as<JsonObject>();
            methods[i].id = methodJson["id"].as<int>();
            methods[i].display_name = methodJson["name"].as<String>();
            Serial.println("Method: " + String(methods[i].id) + " " + methods[i].display_name);
            i++;
        }

        // Return the array of method struct
        MethodList* methodList = new MethodList();
        methodList->methods = methods;
        methodList->num_methods = num_responses;
        return methodList;
    }
};