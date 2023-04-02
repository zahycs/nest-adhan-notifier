#include <esp8266-google-home-notifier.h>

class AdhanPlayer
{
private:
  GoogleHomeNotifier ghn;
  const char *prayerNames[5] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};

public:
  void playAdhan(int prayer)
  {
    const char *mp3Urls[] = {
        "https://download.tvquran.com/download/selections/380/6088017b417a3.mp3",
        "https://www.islamcan.com/audio/adhan/azan2.mp3",
        "https://www.islamcan.com/audio/adhan/azan20.mp3",
        "https://www.islamcan.com/audio/adhan/azan16.mp3",
        "https://www.islamcan.com/audio/adhan/azan14.mp3"};

    const char *mp3Url = mp3Urls[prayer];

    // Notify the name of the prayer
    String notification = "It's time for " + String(prayerNames[prayer]) + " prayer";
    sendNotification(notification.c_str());
    // give a chance to the announcement to play
    delay(3 * 1000);
    // Play the MP3
    if (ghn.play(mp3Url) != true)
    {
      Serial.println(ghn.getLastError());
      return;
    }
    Serial.println("Adhan played.");
  }

  void sendNotification(const char *notification)
  {
    if (ghn.notify(notification) != true)
    {
      Serial.println(ghn.getLastError());
      return;
    }
    Serial.println("Notification sent.");
  }

  void connect(const char speakerName[])
  {
    Serial.println("connecting to Google Home...");
    bool connected = false;
    while (!connected)
    {
      Serial.print("trying to connect to Google Home device....");
      connected = ghn.device(speakerName, "en");
      if (!connected)
      {
        Serial.println(ghn.getLastError());
        delay(1000);
      }
    }
     
    Serial.print("found Google Home(");
    Serial.print(ghn.getIPAddress());
    Serial.print(":");
    Serial.print(ghn.getPort());
    Serial.println(")");

    if (ghn.notify(("Adhan's online")) != true)
    {
      Serial.println(ghn.getLastError());
      return;
    }

    Serial.println("");
  }
};
