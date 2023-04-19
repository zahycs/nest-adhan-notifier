#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp8266-google-home-notifier.h>
#include "AdhanPlayer.cpp"
#include "PrayerApiClient.cpp"
#include "configurator.cpp"
#include <time.h>
#include <EEPROM.h>

// constants and variables
const int NUM_PRAYERS = 5;
struct tm *prayer_times = new tm[NUM_PRAYERS];

AdhanPlayer adhanPlayer;
PrayerApiClient prayerClient;
Configurator configurator;
Config config;
void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting ...");

  configurator.begin();
  static bool configSet = false;
  static Config config;
  // wait for the user to submit the config object
  while (!configSet)
  {
    config = configurator.getConfig();
    if (configurator.isConfigSet())
    {
      Serial.println("Config set!");
      configSet = true;
    }
    configurator.loop();
  }

  // init time
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  adhanPlayer.connect(config.speakerDisplayName);

  Serial.println("calling the API ...");
  setPrayerTimes(config.city, config.country, config.method);
}

void setPrayerTimes(String city, String country, int method)
{
  // Call the API and parse the JSON response
  MethodList *methods_list = prayerClient.getMethods();
  struct tm *response = prayerClient.getPrayerTimes(city, country, method);
  if (response != nullptr)
  {
    prayer_times = response;
    for (int i = 0; i < NUM_PRAYERS; i++)
    {
      Serial.print("Prayer #" + String(i) + ": ");
      printLocalTime(&prayer_times[i]);
    }
    configurator.setPrayerTimes(prayer_times);
    configurator.setMethodsList(methods_list);
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
    try
    {
      // Set prayer times by calling the API
      setPrayerTimes(config.city, config.country, config.method);
      Serial.println("Prayer times have been updated for today");
      adhanPlayer.sendNotification("Prayer times have been successfully updated for today");
      // Wait for a minute before running again
      delay(60 * 1000);
    }
    catch (const std::exception &e)
    {
      Serial.println(" Error setting prayer times, " + String(e.what()));
    }
  }
   config = configurator.getConfig();
  // Check if it's time for the next prayer and play the MP3
  for (int i = 0; i < NUM_PRAYERS; i++)
  {
   
    if (now_hour == prayer_times[i].tm_hour && now_min == prayer_times[i].tm_min)
    {
      playAdhan(i, config.adhan_urls[i]);
    }
  }
  // Check if the user wants to play a test adhan
  if (configurator.isPlayTestAdhan())
  {
    playAdhan(0, config.adhan_urls[0]);
    configurator.setPlayTestAdhan(false);
  }
  configurator.loop();
  // Wait for 2 seconds before running again
  delay(2 * 1000);
}

void playAdhan(int prayer_index, char *adhan_url)
{
  try
  {
    adhanPlayer.playAdhan(prayer_index, adhan_url);
    // Wait for a minute before running again
    delay(60 * 1000);
  }
  catch (const std::exception &e)
  {
    Serial.println(" error playing adhan, " + String(e.what()));
  }
}
