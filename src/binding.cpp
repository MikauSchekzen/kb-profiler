#include "binding.h"
#include "scancodes.h"

using namespace rapidjson;
using namespace std;

Binding::Binding () {
  alt = false;
  shift = false;
  ctrl = false;
  rapidfire = 0;
  code = 0x00;
  origin = 0x00;
  keymap = 0;
  index = 0;
  codeE0 = false;
  originE0 = false;
  toggle = false;
  hardware_id = "";
}

void Binding::parseBind(const Value& bind) {
  if(bind["alt"].IsString() && bind["alt"] == "1") alt = true;
  if(bind["alt"].IsNumber() && bind["alt"] == 1) alt = true;
  if(bind["ctrl"].IsString() && bind["ctrl"] == "1") ctrl = true;
  if(bind["ctrl"].IsNumber() && bind["ctrl"] == 1) ctrl = true;
  if(bind["shift"].IsString() && bind["shift"] == "1") shift = true;
  if(bind["shift"].IsNumber() && bind["shift"] == 1) shift = true;

  if(bind["rapidfire"].IsString() && bind["rapidfire"].GetString() != "0") rapidfire = atoi(bind["rapidfire"].GetString());
  else if(bind["rapidfire"].IsNumber() && bind["rapidfire"].GetInt() > 0) rapidfire = bind["rapidfire"].GetInt();

  if(bind.HasMember("toggle")) {
    if(bind["toggle"].IsString()) {
      string tog = bind["toggle"].GetString();
      if(tog == "1") toggle = true;
    }
    else if(bind["toggle"].IsNumber()) {
      int tog = bind["toggle"].GetInt();
      if(tog == 1) toggle = true;
    }
  }

  if(bind.HasMember("hardware_id") && bind["hardware_id"].IsString()) {
    hardware_id = bind["hardware_id"].GetString();
  }

  if(bind["key"].IsString()) {
    code = stringToKeycode(bind["key"].GetString());
    codeE0 = isE0(bind["key"].GetString());
  }
  if(bind["key"].IsNumber()) {
    code = numberToKeycode(bind["origin"].GetInt());
  }
  if(code == 0x00) keymap = parseKeymap(bind["key"].GetString());
  if(bind["origin"].IsString()) {
    origin = stringToKeycode(bind["origin"].GetString());
    originE0 = isE0(bind["origin"].GetString());
  }
  if(bind["origin"].IsNumber()) {
    code = numberToKeycode(bind["origin"].GetInt());
  }
}

short Binding::numberToKeycode(int number) {
  ostringstream convert;
  convert << number;
  return parseKey(convert.str());
}

short Binding::stringToKeycode(string str) {
  return parseKey(str);
}

int Binding::parseKeymap(string key) {
  if(key.length() > 6) {
    int chr;
    chr = atoi(key.substr(6, string::npos).c_str());
    return chr;
  }
  return 0;
}

short Binding::parseKey(string key) {
  if(key == "left") return SCANCODE_LEFT;
  if(key == "right") return SCANCODE_RIGHT;
  if(key == "up") return SCANCODE_UP;
  if(key == "down") return SCANCODE_DOWN;

  if(key == "insert") return SCANCODE_INSERT;
  if(key == "home") return SCANCODE_HOME;
  if(key == "pgup") return SCANCODE_PGUP;
  if(key == "delete") return SCANCODE_DELETE;
  if(key == "end") return SCANCODE_END;
  if(key == "pgdn") return SCANCODE_PGDN;

  if(key == "1") return SCANCODE_1;
  if(key == "2") return SCANCODE_2;
  if(key == "3") return SCANCODE_3;
  if(key == "4") return SCANCODE_4;
  if(key == "5") return SCANCODE_5;
  if(key == "6") return SCANCODE_6;
  if(key == "7") return SCANCODE_7;
  if(key == "8") return SCANCODE_8;
  if(key == "9") return SCANCODE_9;
  if(key == "0") return SCANCODE_0;
  if(key == "vkbd") return SCANCODE_MINUS;
  if(key == "vkbb") return SCANCODE_PLUS;

  if(key == "escape") return SCANCODE_ESCAPE;
  if(key == "enter") return SCANCODE_ENTER;
  if(key == "tab") return SCANCODE_TAB;
  if(key == "backspace") return SCANCODE_BACKSPACE;
  if(key == "space") return SCANCODE_SPACE;

  if(key == "lctrl") return SCANCODE_LCTRL;
  if(key == "lshift") return SCANCODE_LSHIFT;
  if(key == "lalt") return SCANCODE_LALT;
  if(key == "space") return SCANCODE_SPACE;

  if(key == "sc029") return SCANCODE_APOSTROPHE;
  if(key == "vkdb") return SCANCODE_LBRACKET;
  if(key == "vkdd") return SCANCODE_RBRACKET;
  if(key == "vkba") return SCANCODE_COLON;
  if(key == "vkde") return SCANCODE_QUOTE;
  if(key == "vkbc") return SCANCODE_COMMA;
  if(key == "vkbe") return SCANCODE_DOT;
  if(key == "vkbf") return SCANCODE_SLASH;
  if(key == "vkdc") return SCANCODE_BACKSLASH;

  if(key == "capslock") return SCANCODE_CAPSLOCK;
  if(key == "numlock") return SCANCODE_NUMLOCK;
  if(key == "scrolllock") return SCANCODE_SCROLLLOCK;

  if(key == "f1") return SCANCODE_F1;
  if(key == "f2") return SCANCODE_F2;
  if(key == "f3") return SCANCODE_F3;
  if(key == "f4") return SCANCODE_F4;
  if(key == "f5") return SCANCODE_F5;
  if(key == "f6") return SCANCODE_F6;
  if(key == "f7") return SCANCODE_F7;
  if(key == "f8") return SCANCODE_F8;
  if(key == "f9") return SCANCODE_F9;
  if(key == "f10") return SCANCODE_F10;
  if(key == "f11") return SCANCODE_F11;
  if(key == "f12") return SCANCODE_F12;

  if(key == "f13") return SCANCODE_F13;
  if(key == "f14") return SCANCODE_F14;
  if(key == "f15") return SCANCODE_F15;
  if(key == "f16") return SCANCODE_F16;
  if(key == "f17") return SCANCODE_F17;
  if(key == "f18") return SCANCODE_F18;
  if(key == "f19") return SCANCODE_F19;
  if(key == "f20") return SCANCODE_F20;
  if(key == "f21") return SCANCODE_F21;
  if(key == "f22") return SCANCODE_F22;
  if(key == "f23") return SCANCODE_F23;
  if(key == "f24") return SCANCODE_F24;

  if(key == "numpad1") return SCANCODE_KP1;
  if(key == "numpad2") return SCANCODE_KP2;
  if(key == "numpad3") return SCANCODE_KP3;
  if(key == "numpad4") return SCANCODE_KP4;
  if(key == "numpad5") return SCANCODE_KP5;
  if(key == "numpad6") return SCANCODE_KP6;
  if(key == "numpad7") return SCANCODE_KP7;
  if(key == "numpad8") return SCANCODE_KP8;
  if(key == "numpad9") return SCANCODE_KP9;
  if(key == "numpad0") return SCANCODE_KP0;
  if(key == "numpadadd") return SCANCODE_KPPLUS;
  if(key == "numpadsub") return SCANCODE_KPMINUS;
  if(key == "numpadmult") return SCANCODE_KPMULT;

  if(key == "q") return SCANCODE_Q;
  if(key == "w") return SCANCODE_W;
  if(key == "e") return SCANCODE_E;
  if(key == "r") return SCANCODE_R;
  if(key == "t") return SCANCODE_T;
  if(key == "y") return SCANCODE_Y;
  if(key == "u") return SCANCODE_U;
  if(key == "i") return SCANCODE_I;
  if(key == "o") return SCANCODE_O;
  if(key == "p") return SCANCODE_P;
  if(key == "a") return SCANCODE_A;
  if(key == "s") return SCANCODE_S;
  if(key == "d") return SCANCODE_D;
  if(key == "f") return SCANCODE_F;
  if(key == "g") return SCANCODE_G;
  if(key == "h") return SCANCODE_H;
  if(key == "j") return SCANCODE_J;
  if(key == "k") return SCANCODE_K;
  if(key == "l") return SCANCODE_L;
  if(key == "z") return SCANCODE_Z;
  if(key == "x") return SCANCODE_X;
  if(key == "c") return SCANCODE_C;
  if(key == "v") return SCANCODE_V;
  if(key == "b") return SCANCODE_B;
  if(key == "n") return SCANCODE_N;
  if(key == "m") return SCANCODE_M;
  return 0x00;
}

bool Binding::isE0(string key) {
	const int testNum = 12;
  string test[testNum];
  test[0] = "numpadenter";
  test[1] = "numpadsub";
  test[2] = "home";
  test[3] = "pgup";
  test[4] = "pgdn";
  test[5] = "end";
  test[6] = "insert";
  test[7] = "delete";
  test[8] = "left";
  test[9] = "right";
  test[10] = "up";
  test[11] = "down";
  for(int a = 0;a < testNum;a++) {
    if(key == test[a]) return true;
  }
  return false;
}

bool Binding::isValidHwid(string hwid) {
  return (hardware_id == "" || hardware_id == "any" || hardware_id == hwid);
}
