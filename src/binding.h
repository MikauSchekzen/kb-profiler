#include <string>
#include "../rapidjson/document.h"
#include "../rapidjson/reader.h"
#include "../rapidjson/stringbuffer.h"
#include <sstream>

using namespace rapidjson;
using namespace std;

const int maxKeyTypes = 256;

class Binding {
public:
  bool alt;
  bool shift;
  bool ctrl;
  bool codeE0;
  bool originE0;
  short code;
  short origin;
  int rapidfire;
  int keymap;
  int index;

  Binding();
  void parseBind(const Value& bind);
  short numberToKeycode(int number);
  short stringToKeycode(string str);
  int parseKeymap(string key);
  short parseKey(string key);
  bool isE0(string key);
};
