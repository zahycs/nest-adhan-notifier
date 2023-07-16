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
  // to mark the data written or not, won't be bind to html
    char key[16];
    char ssid[32];
    char password[32];
    char city[32];
    char country[32];
    // method is to be selected from a dropdown list, with id and method name 
    int method; 
    char speakerDisplayName[32];
    char adhan_urls[6][120];
};
#endif