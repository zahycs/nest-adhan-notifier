#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp8266-google-home-notifier.h>
#include "AdhanPlayer.cpp"
#include "PrayerApiClient.cpp"
#include <time.h>

// WiFi credentials
const char *ssid = "Your SSID";
const char *password = "your password";

//Aladhan API endpoint and parameters
const char *api_endpoint = "https://api.aladhan.com/v1/timingsByCity";
const char *city = "Utrecht";
const char *country = "NL";
const int method = 5; // for egyptian general authority of survey

// constants and variables
const int NUM_PRAYERS = 5;
const char displayName[] = "Family room speaker";
unsigned long lastApiCallMillis = 0;
struct tm *prayer_times = new tm[NUM_PRAYERS];

AdhanPlayer adhanPlayer;
PrayerApiClient prayerClient;

void setup()
{

  Serial.begin(115200);
  Serial.println("");
  Serial.print("connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Print the local IP

  // init time
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  adhanPlayer.connect(displayName);

  Serial.println("calling the API ...");
  setPrayertimes(city, country);
  //
}

void setPrayertimes(String city, String country)
{
  // Call the API and parse the JSON response
  struct tm *response = prayerClient.getPrayerTimes(city, country, method);
  if (response != nullptr)
  {
    prayer_times = response;
    for (int i = 0; i < NUM_PRAYERS; i++)
    {
      Serial.print("Prayer #" + String(i) + ": ");
      printLocalTime(&prayer_times[i]);
    }
  }
}

void printLocalTime(struct tm *local_time)
{
  Serial.print(local_time->tm_year + 1900); // print year
  Serial.print("-");
  Serial.print(local_time->tm_mon + 1); // print month
  Serial.print("-");
  Serial.print(local_time->tm_mday); // print day
  Serial.print(" ");
  Serial.print(local_time->tm_hour); // print hour
  Serial.print(":");
  Serial.print(local_time->tm_min); // print minute
  Serial.print(":");
  Serial.print(local_time->tm_sec); // print second
  Serial.println();
}

void loop()
{

  time_t now = time(nullptr);
  struct tm *local_time = localtime(&now);
  int now_hour = local_time->tm_hour;
  int now_min = local_time->tm_min;
  printLocalTime(local_time);

  // Call the API once a day
  if (now_hour == 1 && now_min == 1)
  {
    // Set prayer times by calling the API
    setPrayertimes(city, country);
    // Wait for a minute before running again
    delay(60 * 1000);
    Serial.println("Prayer times have been updated for today");
  }

  // Check if it's time for the next prayer and play the MP3
  for (int i = 0; i < NUM_PRAYERS; i++)
  {
    if (now_hour == prayer_times[i].tm_hour && now_min == prayer_times[i].tm_min)
    {
      adhanPlayer.playAdhan(i);
      // Wait for a minute before running again
      delay(60 * 1000);
    }
  }

  // Wait for 5 seconds before running again
  delay(5 * 1000);
}
