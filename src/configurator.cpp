#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ESPmDNS.h>
#define EEPROM_MARKER 0xAA
#define EEPROM_SIZE 1024

struct Config
{
    char ssid[32];
    char password[32];
    char city[32];
    char country[32];
    int method;
    char api_endpoint[64];
    char speakerDisplayName[32];
    char adhan_urls[5][64];
};

class Configurator
{
private:
    Config config;
    WebServer server;
    const char ap_ssid[32] = "ESP32-Configurator";
    const char *host_name = "adhan";
    

public:
    void begin()
    {
        EEPROM.begin(EEPROM_SIZE);

        // load config from eeprom
        loadConfig();

        // check if ssid and password are set, if not, start SoftAP
        if (!config.ssid[0] || !config.password[0])
        {
            startSoftAP();
        }
        else
        {
            // start wifi
            WiFi.begin(config.ssid, config.password);

            Serial.println("Connecting to WiFi...");
            while (WiFi.status() != WL_CONNECTED)
            {
                delay(250);
                Serial.print(".");
            }
            Serial.println("WiFi connected");

            initMDNS();
            initServer();
        }
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP()); // Print the local IP
    }
    void loop()
    {
        server.handleClient();
    }

    Config getConfig()
    {
        return config;
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
        server.on("/index.html", HTTP_GET, [this]()
                  { handleRoot(); });

        server.on("/save", HTTP_POST, [this]()
                  { handleSave(); });

        server.onNotFound([this]()
                          { handleNotFound(); });

        server.begin();

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Web server started");
            Serial.print("Access it at http://");
            Serial.print(host_name);
            Serial.println(".local/index.html");
        }
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
        printConfig(config);
        EEPROM.end();

        // initialize with default values if not set
         char *default_mp3Urls[] = {
        "https://download.tvquran.com/download/selections/380/6088017b417a3.mp3",
        "https://www.islamcan.com/audio/adhan/azan2.mp3",
        "https://www.islamcan.com/audio/adhan/azan20.mp3",
        "https://www.islamcan.com/audio/adhan/azan16.mp3",
        "https://www.islamcan.com/audio/adhan/azan14.mp3"};

        if (!config.city[0])
        {
            strcpy(config.city, "Utrecht");
        }
        if (!config.country[0])
        {
            strcpy(config.country, "Netherlands");
        }
        if (!config.method)
        {
            config.method = 5; // Egyptian General Authority of Survey
        }
        if (!config.api_endpoint[0])
        {
            strcpy(config.api_endpoint, "https://api.aladhan.com/v1/timingsByCity");
        }
        if (!config.speakerDisplayName[0])
        {
            strcpy(config.speakerDisplayName, "Family room speaker");
        }
        if (!config.adhan_urls[0][0])
        {
            strcpy(config.adhan_urls[0], default_mp3Urls[0]);
        }
        if (!config.adhan_urls[1][0])
        {
            strcpy(config.adhan_urls[1], default_mp3Urls[1]);
        }
        if (!config.adhan_urls[2][0])
        {
            strcpy(config.adhan_urls[2], default_mp3Urls[2]);
        }
        if (!config.adhan_urls[3][0])
        {
            strcpy(config.adhan_urls[3], default_mp3Urls[3]);
        }
        if (!config.adhan_urls[4][0])
        {
            strcpy(config.adhan_urls[4], default_mp3Urls[4]);
        }
    }

    void saveConfig()
    {
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
        Serial.println("API Endpoint: " + String(config.api_endpoint));
        Serial.println("Speaker Display Name: " + String(config.speakerDisplayName));
        for (int i = 0; i < 5; i++)
        {
            Serial.println("Adhan URL " + String(i + 1) + ": " + String(config.adhan_urls[i]));
        }
    }

    void handleRoot()
    {
        String html = "<!DOCTYPE html>\n\
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
            input[type=submit] {\n\
            background-color: #4CAF50;\n\
            color: white;\n\
            padding: 12px 20px;\n\
            border: none;\n\
            border-radius: 4px;\n\
            cursor: pointer;\n\
            }\n\
            input[type=submit]:hover {\n\
            background-color: #45a049;\n\
            }\n\
        </style>\n\
        </head>\n\
        <body>\n\
        <div class=\"container\">\n\
            <h2>ESP Adhan Configuration</h2>\n\
            <form action=\"/save\" method=\"post\">\n\
            <label for=\"ssid\">SSID:</label>\n\
            <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"";
        html += config.ssid;
        html += "\">\n\
          <label for=\"password\">Password:</label>\n\
          <input type=\"password\" id=\"password\" name=\"password\" value=\"";
        html += config.password;
        html += "\">\n\
          <label for=\"city\">City:</label>\n\
          <input type=\"text\" id=\"city\" name=\"city\" value=\"";
        html += config.city;
        html += "\">\n\
          <label for=\"country\">Country:</label>\
          <input type = \"text\" id = \"country\" name = \"country\" value =\"";
        html += config.country;
        html += "\"> \n";
        // Method
        html += "<label for=\"method\">Method:</label>";
        html += "<select id=\"method\" name=\"method\">";
        for (int i = 0; i < 6; i++)
        {
            html += "<option value=\"";
            html += String(i);
            html += "\"";
            if (i == config.method)
            {
                html += " selected";
            }
            html += ">";
            html += String(i + 1);
            html += "</option>";
        }
        html += "</select>\n";

        // API Endpoint
        html += "<label for=\"api_endpoint\">API Endpoint:</label>";
        html += "<input type=\"text\" id=\"api_endpoint\" name=\"api_endpoint\" value=\"";
        html += config.api_endpoint;
        html += "\">\n";

        // Speaker Display Name
        html += "<label for=\"speakerDisplayName\">Speaker Display Name:</label>";
        html += "<input type=\"text\" id=\"speakerDisplayName\" name=\"speakerDisplayName\" value=\"";
        html += config.speakerDisplayName;
        html += "\">\n";

        // Adhan URLs
        for (int i = 0; i < 5; i++)
        {
            String adhanLabel = "Adhan URL " + String(i + 1) + ":";
            String adhanInputId = "adhan_url_" + String(i);
            html += "<label for=\"" + adhanInputId + "\">" + adhanLabel + "</label>";
            html += "<input type=\"text\" id=\"" + adhanInputId + "\" name=\"" + adhanInputId + "\" value=\"";
            html += config.adhan_urls[i];
            html += "\">\n";
        }

        // Save Button
        html += "<input type=\"submit\" value=\"Save\">";
        html += "</form></body></html>";

        server.send(200, "text/html", html);
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
        apiEndpoint.toCharArray(config.api_endpoint, sizeof(config.api_endpoint));
        speakerDisplayName.toCharArray(config.speakerDisplayName, sizeof(config.speakerDisplayName));
        for (int i = 0; i < 5; i++)
        {
            adhanUrls[i].toCharArray(config.adhan_urls[i], sizeof(config.adhan_urls[i]));
        }

        Serial.println("Saving config...");
        saveConfig();

        server.sendHeader("Location", "/index.html", true);
        server.send(302, "text/plain", "");
    }

    void handleNotFound()
    {
        server.send(404, "text/plain", "404 Not Found");
    }
};
