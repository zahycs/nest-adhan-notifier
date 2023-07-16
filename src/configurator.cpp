#include <EEPROM.h>
#include <ESPmDNS.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include "models.h"
#include "AsyncTCP.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#define EEPROM_SIZE 1024
#ifndef GIT_VERSION
#define GIT_VERSION "1.0.0"
#endif
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define GIT_VERSION_STR TOSTRING(GIT_VERSION)

class Configurator
{
private:
    Config config;
    const char ap_ssid[32] = "adhan_configurator";
    const char *host_name = "adhan";
    const char flash_key[16] = "flash";
    tm *prayers;
    // array of methods struct
    MethodList *methods_list;
    bool playTestAdhan = false;
    const char *git_version = GIT_VERSION_STR;
public:
    AsyncWebServer server = AsyncWebServer(80);
    void begin()
    {
        EEPROM.begin(EEPROM_SIZE);

        // load config from eeprom
        loadConfig();
        Serial.println("flash key : ");
        Serial.println(config.key);
        // check if config is set, otherwise start softAP
        if (isConfigSet())
        {
            // start wifi
            WiFi.begin(config.ssid, config.password);

            Serial.println("Connecting to WiFi...");
            unsigned long startAttempt = millis();                                     // initialize timer
            while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < 60000) // add timer to while loop condition
            {
                delay(250);
                Serial.print(".");
            }
            if (WiFi.status() != WL_CONNECTED) // if connection attempt exceeded 1 minute
            {
                WiFi.disconnect();                    // disconnect from WiFi
                startSoftAP();                        
                initMDNS();
                initServer(); 
                server.begin();                       // start the web server                       
                unsigned long apStartTime = millis(); // store the AP start time

                while ((millis() - apStartTime) < 180000) // wait for 3 minutes in AP mode
                {
                    delay(1000);
                }

                ESP.restart(); // restart the ESP32
            }
            else // if WiFi is connected
            {
                Serial.println("WiFi connected");
            }
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP()); // Print the local IP
        }
        else
        {
            startSoftAP();
        }
        initMDNS();
        initServer();
    }

    Config getConfig()
    {
        return config;
    }
    void setPrayerTimes(tm *prayers)
    {
        this->prayers = prayers;
    }
    void setMethodsList(MethodList *methods_list)
    {
        this->methods_list = methods_list;
    }
    bool isConfigSet()
    {
        return strcmp(config.key, flash_key) == 0;
    }
    bool isPlayTestAdhan()
    {
        return this->playTestAdhan;
    }
    void setPlayTestAdhan(bool playTestAdhan)
    {
        this->playTestAdhan = playTestAdhan;
    }

private:
    void startSoftAP()
    {
        Serial.println("Starting SoftAP");
        Serial.println("Connect to ESP32 to configure WiFi");
        Serial.print("SSID: ");
        Serial.println(ap_ssid);
        WiFi.softAP(ap_ssid);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP);
    }

    void initServer()
    {
        if (!SPIFFS.begin(true))
        {
            Serial.println("An error has occurred while mounting SPIFFS");
        }

        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, "/index.html", "text/html", false, [this](const String &var)
                                  { return processor(var, this); }); });

        server.serveStatic("/", SPIFFS, "/");

        server.on("/save", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleSave(request); });

        server.on("/playTest", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handlePlayTestAdhan(request); });

        server.onNotFound([this](AsyncWebServerRequest *request)
                          { handleNotFound(request); });

        Serial.println("Web server started");
        Serial.print("Access it at http://");
        Serial.print(host_name);
        Serial.println(".local");
    }

    void initMDNS()
    {
        while (!MDNS.begin(host_name))
        {
            Serial.println("Starting mDNS...");
            delay(1000);
        }

        Serial.println("MDNS started");
    }

    void loadConfig()
    {
        EEPROM.get(0, config);
        Serial.println("config loaded..");

        EEPROM.end();

        if (!isConfigSet())
        {
            // initialize with default values if not set
            char *default_mp3Urls[] = {
                "https://download.tvquran.com/download/selections/380/6088017b417a3.mp3",
                "https://www.islamcan.com/audio/adhan/azan2.mp3",
                "https://www.islamcan.com/audio/adhan/azan20.mp3",
                "https://www.islamcan.com/audio/adhan/azan16.mp3",
                "https://www.islamcan.com/audio/adhan/azan14.mp3",
                "http://n09.radiojar.com/8s5u5tpdtwzuv?rj-ttl=5&rj-tok=AAABiVyBukIAsqq5rspqjgmmOA"
                //"https://stream.radioendirect.net/8945"
                };

            strcpy(config.ssid, "wifi ssid here");

            strcpy(config.password, "wifi password here");

            strcpy(config.city, "Utrecht");

            strcpy(config.country, "Netherlands");

            config.method = 3; // Egyptian General Authority of Survey

            strcpy(config.speakerDisplayName, "Family room speaker");

            strcpy(config.adhan_urls[0], default_mp3Urls[0]);

            strcpy(config.adhan_urls[1], default_mp3Urls[1]);

            strcpy(config.adhan_urls[2], default_mp3Urls[2]);

            strcpy(config.adhan_urls[3], default_mp3Urls[3]);

            strcpy(config.adhan_urls[4], default_mp3Urls[4]);

            strcpy(config.adhan_urls[5], default_mp3Urls[5]);
        }
        else
        {
            printConfig(config);
        }
    }

    void saveConfig()
    {
        strcpy(config.key, flash_key);
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.put(0, config);
        EEPROM.commit();
        EEPROM.end();
        Serial.println("Config has been committed to EEPROM");
    }

    void printConfig(Config config)
    {
        Serial.println("SSID: " + String(config.ssid));
        Serial.println("Password: " + String(config.password));
        Serial.println("City: " + String(config.city));
        Serial.println("Country: " + String(config.country));
        Serial.println("Method: " + String(config.method));
        Serial.println("Speaker Display Name: " + String(config.speakerDisplayName));
        for (int i = 0; i < 6; i++)
        {
            Serial.println("Adhan URL " + String(i + 1) + ": " + String(config.adhan_urls[i]));
        }
    }
    static String processor(const String &var, const Configurator *configurator)
    {
        Config config = configurator->config;
        if (var == "TMPL_VERSION")
            return configurator->git_version;
        if (var == "TMPL_SSID")
            return config.ssid;
        if (var == "TMPL_PASSWORD")
            return config.password;
        if (var == "TMPL_CITY")
            return config.city;
        if (var == "TMPL_COUNTRY")
            return config.country;
        if (var == "TMPL_METHOD_OPTIONS")
            return buildMethodOptions(config, configurator->methods_list);

        if (var == "TMPL_SPEAKER_DISPLAY_NAME")
            return config.speakerDisplayName;

        if (var == "TMPL_ADHAN_URL_1")
            return config.adhan_urls[0];

        if (var == "TMPL_ADHAN_URL_2")
            return config.adhan_urls[1];

        if (var == "TMPL_ADHAN_URL_3")
            return config.adhan_urls[2];

        if (var == "TMPL_ADHAN_URL_4")
            return config.adhan_urls[3];

        if (var == "TMPL_ADHAN_URL_5")
            return config.adhan_urls[4];

        if (configurator->prayers != nullptr)
        {
            if (var == "TMPL_FAJR")
                return formatTime(configurator->prayers[0]);
            if (var == "TMPL_DHUHR")
                return formatTime(configurator->prayers[1]);
            if (var == "TMPL_ASR")
                return formatTime(configurator->prayers[2]);
            if (var == "TMPL_MAGHRIB")
                return formatTime(configurator->prayers[3]);
            if (var == "TMPL_ISHA")
                return formatTime(configurator->prayers[4]);
        }

        return String();
    }

    static String buildMethodOptions(const Config &config, const MethodList *methods_list)
    {
        std::stringstream html;
        if (methods_list != nullptr)
        {
            int numMethods = methods_list->num_methods;
            if (numMethods > 0)
            {
                // loop over the methods
                for (int i = 0; i < numMethods; i++)
                {
                    html << "<option value=\"";
                    html << String(methods_list->methods[i].id).c_str();
                    html << "\"";
                    if (methods_list->methods[i].id == config.method)
                    {
                        html << " selected";
                    }
                    html << ">";
                    html << methods_list->methods[i].display_name.c_str();
                    html << "</option>";
                }
            }
            else
            {
                Serial.println("No methods found");
                html << "<option value=\"";
                html << String(config.method).c_str();
                html << "\"";
                html << " selected";
                html << ">";
                html << "default method";
                html << "</option>";
            }
        }
        else
        {
            Serial.println("methods is null");
            html << "<option value=\"";
            html << String(config.method).c_str();
            html << "\"";
            html << " selected";
            html << ">";
            html << "default method";
            html << "</option>";
        }
        return html.str().c_str();
    }

    // formatTime function takes a tm struct representing a time and returns a formatted string
    static char *formatTime(struct tm time)
    {
        static char formattedTime[6];
        sprintf(formattedTime, "%02d:%02d", time.tm_hour, time.tm_min);
        return formattedTime;
    }

    // getPrayerName function takes an integer representing a prayer index (0-4) and returns the prayer name
    char *getPrayerName(int index)
    {
        char *prayerNames[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
        return prayerNames[index];
    }

    void handleSave(AsyncWebServerRequest *request)
    {
        Serial.println("Saving configs....");
        String ssid = request->arg("ssid");
        String password = request->arg("password");
        String city = request->arg("city");
        String country = request->arg("country");
        int method = request->arg("method").toInt();
        String apiEndpoint = request->arg("api_endpoint");
        String speakerDisplayName = request->arg("speakerDisplayName");
        String adhanUrls[5] = {
            request->arg("adhan_url_0"),
            request->arg("adhan_url_1"),
            request->arg("adhan_url_2"),
            request->arg("adhan_url_3"),
            request->arg("adhan_url_4")};

        ssid.toCharArray(config.ssid, sizeof(config.ssid));
        password.toCharArray(config.password, sizeof(config.password));
        city.toCharArray(config.city, sizeof(config.city));
        country.toCharArray(config.country, sizeof(config.country));
        config.method = method;
        speakerDisplayName.toCharArray(config.speakerDisplayName, sizeof(config.speakerDisplayName));
        for (int i = 0; i < 5; i++)
        {
            adhanUrls[i].toCharArray(config.adhan_urls[i], sizeof(config.adhan_urls[i]));
        }

        Serial.println("Saving config...");
        saveConfig();

        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Location", "/");
        request->send(response);
        // Restart ESP so that the new config is used
        delay(5 * 1000);
        esp_restart();
    }

    void handlePlayTestAdhan(AsyncWebServerRequest *request)
    {
        playTestAdhan = true;
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Location", "/");
        request->send(response);
    }

    void handleNotFound(AsyncWebServerRequest *request)
    {
        request->send(404, "text/plain", "404 Not Found");
    }
};
