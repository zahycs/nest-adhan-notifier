#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "models.h"
#include "PrayerTimesInterface.h"

class MawaqitPrayerTimes : public PrayerTimesInterface
{
private:
    String mosqueId;
    String username;
    String password;
    String apiAccessToken;
    const int num_prayers = 5;

    struct tm* getLocalPrayerTimes(String json) {
        JsonDocument doc;
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

    // Each HTTP call gets its own fresh client — avoids SSL state reuse crashes
    bool obtainAccessToken() {
        Serial.println("Obtaining Mawaqit API access token...");

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        String auth = base64::encode(username + ":" + password);
        http.begin(client, "https://mawaqit.net/api/2.0/me");
        http.addHeader("Authorization", "Basic " + auth);
        http.addHeader("Accept", "application/json");
        int statusCode = http.GET();

        if (statusCode == HTTP_CODE_OK) {
            String response = http.getString();
            http.end();

            JsonDocument doc;
            deserializeJson(doc, response);
            apiAccessToken = doc["apiAccessToken"].as<String>();
            Serial.println("API access token obtained successfully");
            return true;
        }

        Serial.println("Failed to obtain API access token, statusCode: " + String(statusCode));
        http.end();
        return false;
    }

    String getRequest(String url) {
        if (apiAccessToken.isEmpty() && !obtainAccessToken()) {
            Serial.println("Failed to obtain access token");
            return "";
        }

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        http.begin(client, url);
        http.addHeader("api-access-token", apiAccessToken);
        http.addHeader("Accept", "application/json");

        int statusCode = http.GET();
        if (statusCode == HTTP_CODE_OK) {
            String response = http.getString();
            http.end();
            return response;
        }

        if (statusCode == 401) {
            // Token may have expired — refresh once and retry
            Serial.println("401 received, refreshing token...");
            http.end();
            apiAccessToken = "";
            if (!obtainAccessToken()) {
                return "";
            }

            WiFiClientSecure client2;
            client2.setInsecure();
            HTTPClient http2;
            http2.begin(client2, url);
            http2.addHeader("api-access-token", apiAccessToken);
            http2.addHeader("Accept", "application/json");
            statusCode = http2.GET();
            if (statusCode == HTTP_CODE_OK) {
                String response = http2.getString();
                http2.end();
                return response;
            }
            Serial.println("Retry failed, statusCode: " + String(statusCode));
            http2.end();
            return "";
        }

        Serial.println("Request failed, statusCode: " + String(statusCode));
        http.end();
        return "";
    }

public:
    MawaqitPrayerTimes(String mosqueId, String username, String password)
        : mosqueId(mosqueId), username(username), password(password) {
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
