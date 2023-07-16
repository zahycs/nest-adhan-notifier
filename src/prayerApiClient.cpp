#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "models.h"

class PrayerApiClient
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
  PrayerApiClient()
  {
    client.setInsecure();  // Ignore SSL certificate errors
  }

  struct tm *getPrayerTimes(String city, String country, int method)
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

  MethodList *getMethods()
  {
    Serial.println("Getting calculation methods...");
    String url = "https://api.aladhan.com/v1/methods";  // HTTPS is used here
    String response = getRequest(url);
    if (response.isEmpty())
    {
      Serial.println("Failed to get the methods");
      return nullptr;
    }
    else
    {
      Serial.println(response);
    }

    // map the response to a struct
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response);

    JsonObject data = doc["data"];
    int num_responses = data.size(); // Get the number of responses in the data object
    Method *methods = new Method[num_responses];
    int i = 0;
    Serial.println("Number of methods: " + String(num_responses));

    for (ArduinoJson::V6212PB::JsonObject::iterator it = data.begin(); it != data.end(); ++it)
    {
      String key = it->key().c_str();
      Serial.println("Method: " + key);
      JsonObject methodJson = it->value();
      int id = methodJson["id"].as<int>();
      String name = methodJson["name"].as<String>();
      
      methods[i].id = id;
      methods[i].display_name = name;
      Serial.println("Method: " + String(id) + " " + name); 
      i++;
    }
    // Return the array of method struct
    MethodList *methodList = new MethodList();
    methodList->methods = methods;
    methodList->num_methods = num_responses;
    return methodList;
  }
};
