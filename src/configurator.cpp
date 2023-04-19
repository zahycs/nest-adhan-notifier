#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ESPmDNS.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include "models.h"

#define EEPROM_SIZE 1024

struct Config
{
    String key;
    char ssid[32];
    char password[32];
    char city[32];
    char country[32];
    int method;
    // char api_endpoint[64];
    char speakerDisplayName[32];
    char adhan_urls[5][120];
};

class Configurator
{
private:
    Config config;
    WebServer server;
    const char ap_ssid[32] = "ESP32-Configurator";
    const char *host_name = "adhan";
    const String flash_key = "flash";
    tm *prayers;
    // array of methods struct
    MethodList *methods_list;
    bool playTestAdhan = false;

public:
void begin()
{
    EEPROM.begin(EEPROM_SIZE);

    // load config from eeprom
    loadConfig();

    // check if config is set, otherwise start softAP
    if (config.key != flash_key)
    {
        startSoftAP();
    }
    else
    {
        // start wifi
        WiFi.begin(config.ssid, config.password);

        Serial.println("Connecting to WiFi...");
        unsigned long startAttempt = millis(); // initialize timer
        while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < 60000) // add timer to while loop condition
        {
            delay(250);
            Serial.print(".");
        }
        if (WiFi.status() != WL_CONNECTED) // if connection attempt exceeded 1 minute
        {
            WiFi.disconnect(); // disconnect from WiFi
            startSoftAP(); // start SoftAP
        }
        else // if WiFi is connected
        {
            Serial.println("WiFi connected");

            initMDNS();
            initServer();
        }
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP()); // Print the local IP
    }
}

    void loop()
    {
        server.handleClient();
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
        return config.key == flash_key;
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
        Serial.println("SSID: ESP32-Configurator");
        WiFi.softAP(ap_ssid);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP);

        initMDNS();
        initServer();
    }

    void initServer()
    {
        // Start the server
        server.on("/home", HTTP_GET, [this]()
                  { handleRoot(); });

        server.on("/save", HTTP_POST, [this]()
                  { handleSave(); });

        server.on("/playTest", HTTP_POST, [this]()
                  { handlePlayTestAdhan(); });

        server.onNotFound([this]()
                          { handleNotFound(); });

        server.begin();

        Serial.println("Web server started");
        Serial.print("Access it at http://");
        Serial.print(host_name);
        Serial.println(".local/home");
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
                "https://www.islamcan.com/audio/adhan/azan14.mp3"};

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
        }
        else
        {
            printConfig(config);
        }
    }

    void saveConfig()
    {
        config.key = flash_key;
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.put(0, config);
        EEPROM.commit();
        EEPROM.end();
    }

    void printConfig(Config config)
    {
        Serial.println("SSID: " + String(config.ssid));
        Serial.println("Password: " + String(config.password));
        Serial.println("City: " + String(config.city));
        Serial.println("Country: " + String(config.country));
        Serial.println("Method: " + String(config.method));
        Serial.println("Speaker Display Name: " + String(config.speakerDisplayName));
        for (int i = 0; i < 5; i++)
        {
            Serial.println("Adhan URL " + String(i + 1) + ": " + String(config.adhan_urls[i]));
        }
    }

    void handleRoot()
    {
        String htmlString = "";
        try
        {
            htmlString = buildHomePage();
        }
        catch (const std::exception &e)
        {
            Serial.println(e.what());
            // create html with error message from the exception
            htmlString = buildErrorPage(e.what());
        }
        server.send(200, "text/html", htmlString);
    }
    String buildHomePage()
    {
        std::stringstream html;
        html << "<!DOCTYPE html>\n\
        <html>\n\
        <head>\n\
        <title>ESP Adhan Configuration</title>\n\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
        <style>\n\
            body {\n\
            font-family: Arial, Helvetica, sans-serif;\n\
            margin: 0;\n\
            padding: 0;\n\
            background-color: #f2f2f2;\n\
            }\n\
            .container {\n\
            width: 90%;\n\
            margin: auto;\n\
            background-color: #fff;\n\
            padding: 20px;\n\
            border-radius: 10px;\n\
            box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);\n\
            }\n\
            h2 {\n\
            text-align: center;\n\
            }\n\
            form {\n\
            display: grid;\n\
            grid-template-columns: 150px 1fr;\n\
            row-gap: 10px;\n\
            column-gap: 10px;\n\
            justify-items: start;\n\
            }\n\
            label {\n\
            font-weight: bold;\n\
            }\n\
            input[type=text],\n\
            input[type=password] {\n\
            width: 100%;\n\
            padding: 12px 20px;\n\
            margin: 8px 0;\n\
            display: inline-block;\n\
            border: 1px solid #ccc;\n\
            border-radius: 4px;\n\
            box-sizing: border-box;\n\
            }\n\
            select {\n\
            width: 100%;\n\
            padding: 12px 20px;\n\
            margin: 8px 0;\n\
            display: inline-block;\n\
            border: 1px solid #ccc;\n\
            border-radius: 4px;\n\
            box-sizing: border-box;\n\
            }\n\
            .save-restart {\n\
            float: right;\n\
            background-color: #4CAF50;\n\
            color: white;\n\
            padding: 12px 20px;\n\
            border: none;\n\
            border-radius: 4px;\n\
            cursor: pointer;\n\
            }\n\
            .play-test {\n\
            float: right;\n\
            background-color: blue;\n\
            color: white;\n\
            padding: 12px 20px;\n\
            border: none;\n\
            border-radius: 4px;\n\
            cursor: pointer;\n\
            }\n\
        </style>\n\
        </head>\n\
        <body>\n\
        <div class=\"container\">\n\
        <h2>ESP Adhan Configuration</h2>\n";

        // Get prayer times
        String prayer_times = buildPrayersHtml(prayers);
        html << prayer_times.c_str();
        html << "<form action=\"/save\" method=\"post\">\n\
            <label for=\"ssid\">SSID:</label>\n\
            <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"";

        html << config.ssid;
        html << "\">\n\
          <label for=\"password\">Password:</label>\n\
          <input type=\"password\" id=\"password\" name=\"password\" value=\"";
        html << config.password;
        html << "\">\n\
          <label for=\"city\">City:</label>\n\
          <input type=\"text\" id=\"city\" name=\"city\" value=\"";
        html << config.city;
        html << "\">\n\
          <label for=\"country\">Country:</label>\
          <input type = \"text\" id = \"country\" name = \"country\" value =\"";
        html << config.country;
        html << "\"> \n";
        // Method
        html << "<label for=\"method\">Method:</label>";
        html << "<select id=\"method\" name=\"method\">";

        // loop through methods

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
            html << "</select>\n";
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
            html << "</select>\n";
        }

        // API Endpoint
        // html << "<label for=\"api_endpoint\">API Endpoint:</label>";
        // html << "<input type=\"text\" id=\"api_endpoint\" name=\"api_endpoint\" value=\"";
        // html << config.api_endpoint;
        // html << "\">\n";

        // Speaker Display Name
        html << "<label for=\"speakerDisplayName\">Speaker Display Name:</label>\
          <input type = \"text\" id = \"speakerDisplayName\" name = \"speakerDisplayName\" value =\"";
        html << config.speakerDisplayName;
        html << "\"> \n";

        // Adhan URLs
        for (int i = 0; i < 5; i++)
        {
            String adhanLabel = "Adhan URL " + String(i + 1) + ":";
            String adhanInputId = "adhan_url_" + String(i);
            html << "<label for=\"";
            html << adhanInputId.c_str();
            html << adhanInputId.c_str();
            html << "\">";
            html << adhanLabel.c_str();
            html << "</label>";
            html << "<input type=\"text\" id=\"";
            html << adhanInputId.c_str();
            html << "\" name=\"";
            html << adhanInputId.c_str();
            html << "\" value=\"";
            html << config.adhan_urls[i];
            html << "\">\n";
        }

        // Save Button
        html << "<input type=\"submit\" class=\"save-restart\" value=\"Save & Restart\">";
        html << "</form>";
        html << "</div></body></html>";

        return html.str().c_str();
    }
    String buildErrorPage(const char *err)
    {
        return "<!DOCTYPE html>\n\
            <html>\n\
            <head>\n\
            <title>ESP Adhan Configuration</title>\n\
            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
            <style>\n\
                body {\n\
                font-family: Arial, Helvetica, sans-serif;\n\
                margin: 0;\n\
                padding: 0;\n\
                background-color: #f2f2f2;\n\
                }\n\
                .container {\n\
                width: 90%;\n\
                margin: auto;\n\
                background-color: #fff;\n\
                padding: 20px;\n\
                border-radius: 10px;\n\
                box-shadow: 0px 0px 10px 0px rgba(0,0,0,0.2);\n\
                }\n\
                .error {\n\
                color: red;\n\
                }\n\
            </style>\n\
            </head>\n\
            <body>\n\
            <div class=\"container\">\n\
            <h1>ESP Adhan Configuration</h1>\n\
            <p class=\"error\">Error: " +
               String(err) + "</p>\n\
            </div>\n\
            </body>\n\
            </html>";
    }
    String buildPrayersHtml(struct tm prayers[])
    {
        if (prayers == nullptr)
        {
            return "";
        }
        String html = "<div style=\"display: flex; flex-direction: row;\">";

        for (int i = 0; i < 5; i++)
        {
            String time_str = formatTime(prayers[i]);
            String prayer_name = getPrayerName(i);

            String prayer_html = "<div style=\"padding: 5px; margin-right: 10px; background-color: #f2f2f2;\">" + prayer_name + ": " + time_str + "</div>";

            html += prayer_html;
        }
        html += "<form action=\"/playTest\" method=\"post\">";
        html += "<input type=\"submit\" class=\"play-test\" value=\"Play Test\">";
        html += "</form>";
        html += "</div><br><br>";

        return html;
    }

    // formatTime function takes a tm struct representing a time and returns a formatted string
    char *formatTime(struct tm time)
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

    void handleSave()
    {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        String city = server.arg("city");
        String country = server.arg("country");
        int method = server.arg("method").toInt();
        String apiEndpoint = server.arg("api_endpoint");
        String speakerDisplayName = server.arg("speakerDisplayName");
        String adhanUrls[5] = {
            server.arg("adhan_url_0"),
            server.arg("adhan_url_1"),
            server.arg("adhan_url_2"),
            server.arg("adhan_url_3"),
            server.arg("adhan_url_4")};

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

        server.sendHeader("Location", "/home", true);
        server.send(302, "text/plain", "");
        // Restart ESP so that the new config is used
        delay(5 * 1000);
        esp_restart();
    }
    void handlePlayTestAdhan()
    {
        playTestAdhan = true;
        server.sendHeader("Location", "/home", true);
        server.send(302, "text/plain", "");
    }
    void handleNotFound()
    {
        server.send(404, "text/plain", "404 Not Found");
    }
};
