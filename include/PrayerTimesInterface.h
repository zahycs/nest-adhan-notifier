// PrayerTimesInterface.h
#ifndef PRAYER_TIMES_INTERFACE_H
#define PRAYER_TIMES_INTERFACE_H

#include <Arduino.h>
#include <time.h>
#include "models.h"

class PrayerTimesInterface {
public:
    virtual MethodList* getMethods() = 0;
    virtual struct tm* getPrayerTimes(String city, String country, int method) = 0;
};

#endif // PRAYER_TIMES_INTERFACE_H
