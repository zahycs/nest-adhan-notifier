#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "models.h"
#include "PrayerTimesInterface.h"

class MawaqitPrayerTimes : public PrayerTimesInterface
{
private:
    WiFiClientSecure client;
    HTTPClient httpClient;
    String mosqueId;
    const int num_prayers = 5;

    struct tm* getLocalPrayerTimes(String json) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);

        JsonArray timesArray = doc["times"];
        struct tm* prayer_times = new struct tm[num_prayers];

        for (int i = 0; i < num_prayers; i++) {
            String timeStr = timesArray[i].as<String>();
            int hour = timeStr.substring(0, 2).toInt();
            int minute = timeStr.substring(3, 5).toInt();

            prayer_times[i].tm_hour = hour;
            prayer_times[i].tm_min = minute;
            prayer_times[i].tm_sec = 0;
            prayer_times[i].tm_isdst = -1;
        }

        return prayer_times;
    }

    String getRequest(String url) {

        httpClient.begin(client, url);
        httpClient.addHeader("Accept", "application/json");

        int statusCode = httpClient.GET();
        if (statusCode == HTTP_CODE_OK) {
            String response = httpClient.getString();
            httpClient.end();
            return response;
        } else {
            Serial.println("Request failed, statusCode: " + String(statusCode));
            httpClient.end();
            return "";
        }
    }


public:
    MawaqitPrayerTimes(String mosqueId)
        : mosqueId(mosqueId) {
        client.setInsecure(); // Ignore SSL certificate errors
    }

    struct tm* getPrayerTimes(String city, String country, int method) override {
        String url = "https://mawaqit.net/api/2.0/mosque/" + mosqueId + "/prayer-times";
        Serial.println("endpoint: " + url);

        String response = getRequest(url);
        if (response.isEmpty()) {
            return nullptr;
        }

        return getLocalPrayerTimes(response);
    }

    MethodList* getMethods() override {
        Method* methods = new Method[1];
        methods[0].id = 1;
        methods[0].display_name = "local mosque";

        MethodList* methodList = new MethodList();
        methodList->methods = methods;
        methodList->num_methods = 1;
        return methodList;
    }
};
