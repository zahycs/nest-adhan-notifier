#include <WiFi.h>
#ifndef method_h
#define method_h

struct Method
{
    int id;
    String display_name;
};
struct MethodList {
  Method *methods;
  int num_methods;
};

struct Config
{
    char key[16];
    char ssid[32];
    char password[32];
    char city[32];
    char country[32];
    int method;
    // char api_endpoint[64];
    char speakerDisplayName[32];
    char adhan_urls[5][120];
};
#endif