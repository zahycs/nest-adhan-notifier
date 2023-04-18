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
#endif