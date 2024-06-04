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
    String username;
    String password;
    String apiAccessToken;
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
        if (apiAccessToken.isEmpty() && !obtainAccessToken()) {
            Serial.println("Failed to obtain access token");
            return "";
        }

        httpClient.begin(client, url);
        httpClient.addHeader("api-access-token", apiAccessToken);
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

    bool obtainAccessToken() {
        if (!client.connect("mawaqit.net", 443)) {
            Serial.println("Connection failed");
            return false;
        }

        String auth = base64::encode(username + ":" + password);
        String url = "https://mawaqit.net/api/2.0/me";

        httpClient.begin(client, url);
        httpClient.addHeader("Authorization", "Basic " + auth);
        httpClient.addHeader("Accept", "application/json");
        int statusCode = httpClient.GET();

        if (statusCode == HTTP_CODE_OK) {
            String response = httpClient.getString();
            httpClient.end();

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);
            apiAccessToken = doc["apiAccessToken"].as<String>();
            Serial.println("API access token obtained: " + apiAccessToken);
            return true;
        } else {
            Serial.println("Failed to obtain API access token, statusCode: " + String(statusCode));
            httpClient.end();
            return false;
        }
    }

public:
    MawaqitPrayerTimes(String mosqueId, String username, String password)
        : mosqueId(mosqueId), username(username), password(password) {
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
