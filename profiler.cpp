#include "interception.h"
#include "utils.h"

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <mmsystem.h>
#include <sstream>

using namespace rapidjson;
using namespace std;


const int maxMaps = 12;
const int maxKeyTypes = 256;

enum ScanCode
{
  SCANCODE_APOSTROPHE  = 0x29,
  SCANCODE_1           = 0x02,
  SCANCODE_2           = 0x03,
  SCANCODE_3           = 0x04,
  SCANCODE_4           = 0x05,
  SCANCODE_5           = 0x06,
  SCANCODE_6           = 0x07,
  SCANCODE_7           = 0x08,
  SCANCODE_8           = 0x09,
  SCANCODE_9           = 0x0a,
  SCANCODE_0           = 0x0b,
  SCANCODE_MINUS       = 0x0c,
  SCANCODE_PLUS        = 0x0d,

  SCANCODE_A   = 0x1e,
  SCANCODE_B   = 0x30,
  SCANCODE_C   = 0x2e,
  SCANCODE_D   = 0x20,
  SCANCODE_E   = 0x12,
  SCANCODE_F   = 0x21,
  SCANCODE_G   = 0x22,
  SCANCODE_H   = 0x23,
  SCANCODE_I   = 0x17,
  SCANCODE_J   = 0x24,
  SCANCODE_K   = 0x25,
  SCANCODE_L   = 0x26,
  SCANCODE_M   = 0x32,
  SCANCODE_N   = 0x31,
  SCANCODE_O   = 0x18,
  SCANCODE_P   = 0x19,
  SCANCODE_Q   = 0x10,
  SCANCODE_R   = 0x13,
  SCANCODE_S   = 0x1f,
  SCANCODE_T   = 0x14,
  SCANCODE_U   = 0x16,
  SCANCODE_V   = 0x2f,
  SCANCODE_W   = 0x11,
  SCANCODE_X   = 0x2d,
  SCANCODE_Y   = 0x15,
  SCANCODE_Z   = 0x2c,

  SCANCODE_F1    = 0x3b,
  SCANCODE_F2    = 0x3c,
  SCANCODE_F3    = 0x3d,
  SCANCODE_F4    = 0x3e,
  SCANCODE_F5    = 0x3f,
  SCANCODE_F6    = 0x40,
  SCANCODE_F7    = 0x41,
  SCANCODE_F8    = 0x42,
  SCANCODE_F9    = 0x43,
  SCANCODE_F10   = 0x44,
  SCANCODE_F11   = 0x57,
  SCANCODE_F12   = 0x58,

  SCANCODE_ESCAPE     = 0x01,
  SCANCODE_TAB        = 0x0f,
  SCANCODE_ENTER      = 0x1c,
  SCANCODE_BACKSPACE  = 0x0e,
  SCANCODE_SPACE      = 0x39,

  SCANCODE_CAPSLOCK     = 0x3a,
  SCANCODE_NUMLOCK      = 0x45,
  SCANCODE_SCROLLLOCK   = 0x46,

  SCANCODE_LCTRL   = 0x1d,
  SCANCODE_LSHIFT  = 0x2a,
  SCANCODE_LALT    = 0x38,

  SCANCODE_LBRACKET    = 0x1a,
  SCANCODE_RBRACKET    = 0x1b,
  SCANCODE_COLON       = 0x27,
  SCANCODE_QUOTE       = 0x28,
  SCANCODE_BACKSLASH   = 0x2b,
  SCANCODE_COMMA       = 0x33,
  SCANCODE_DOT         = 0x34,
  SCANCODE_SLASH       = 0x35,

  SCANCODE_KPMULT      = 0x37,
  SCANCODE_KPPLUS      = 0x4e,
  SCANCODE_KPMINUS     = 0x4a,
  SCANCODE_KPDEL       = 0x53,
  SCANCODE_KP1         = 0x4f,
  SCANCODE_KP2         = 0x50,
  SCANCODE_KP3         = 0x51,
  SCANCODE_KP4         = 0x4b,
  SCANCODE_KP5         = 0x4c,
  SCANCODE_KP6         = 0x4d,
  SCANCODE_KP7         = 0x47,
  SCANCODE_KP8         = 0x48,
  SCANCODE_KP9         = 0x49,
  SCANCODE_KP0         = 0x52,

  SCANCODE_UP          = 0x48,
  SCANCODE_DOWN        = 0x50,
  SCANCODE_LEFT        = 0x4b,
  SCANCODE_RIGHT       = 0x4d,
  SCANCODE_INSERT      = 0x52,
  SCANCODE_HOME        = 0x47,
  SCANCODE_PGUP        = 0x49,
  SCANCODE_DELETE      = 0x53,
  SCANCODE_END         = 0x4f,
  SCANCODE_PGDN        = 0x51
};

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

  Binding () {
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
  }

  void parseBind(const Value& bind) {
    if(bind["alt"].IsString() && bind["alt"] == "1") alt = true;
    if(bind["alt"].IsNumber() && bind["alt"] == 1) alt = true;
    if(bind["ctrl"].IsString() && bind["ctrl"] == "1") ctrl = true;
    if(bind["ctrl"].IsNumber() && bind["ctrl"] == 1) ctrl = true;
    if(bind["shift"].IsString() && bind["shift"] == "1") shift = true;
    if(bind["shift"].IsNumber() && bind["shift"] == 1) shift = true;

    ostringstream convert;

    if(bind["key"].IsString()) {
      code = parseKey(bind["key"].GetString());
      codeE0 = isE0(bind["key"].GetString());
    }
    if(bind["key"].IsNumber()) {
      convert << bind["key"].GetInt();
      code = parseKey(convert.str());
    }
    if(code == 0x00) keymap = parseKeymap(bind["key"].GetString());
    if(bind["origin"].IsString()) {
      origin = parseKey(bind["origin"].GetString());
      originE0 = isE0(bind["origin"].GetString());
    }
    if(bind["origin"].IsNumber()) {
      convert << bind["origin"].GetInt();
      code = parseKey(convert.str());
    }
  }

  int parseKeymap(string key) {
    if(key.length() > 6) {
      int chr;
      chr = atoi(key.substr(6, string::npos).c_str());
      return chr;
    }
    return 0;
  }

  short parseKey(string key) {
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

  bool isE0(string key) {
    int testNum = 13;
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
    test[12] = "vkbf";
    for(int a = 0;a < testNum;a++) {
      if(key == test[a]) return true;
    }
    return false;
  }
};

class Keymap {
  public:
  int bindCount;
  Binding bindings[maxKeyTypes];

  Keymap() {
    bindCount = 0;
  }

  void addBind(Binding bind) {
    bindings[bindCount] = bind;
    bindCount++;
  }

  Binding getBind(int index) {
    return bindings[index];
  }
};

class KeyObj {
  public:
  short code;
  int map;
  bool down;
  Binding bind;

  KeyObj() {
    code = 0x00;
    map = 0;
    down = false;
  }
};


int main(int argc, char *argv[])
{
    ifstream profile_file;
    string line;
    string profile_json;
    char* profile_filename = (char *)"profiles/RMVXA.json";
    if(argc > 1) {
      profile_filename = (char *)argv[1];
    }
    profile_file.open(profile_filename);
    if(profile_file.is_open()) {
      while(getline(profile_file, line)) {
        profile_json += line;
      }
      profile_file.close();
    }
    else cout << "Profile not found.";

    ifstream file;
    string whitelist[10];
    file.open("whitelist.txt");
    bool useWhitelist = false;
    if(file.is_open()) {
      useWhitelist = true;
      int a = 0;
      while(getline(file, line)) {
        whitelist[a] = line;
        a++;
      }
      profile_file.close();
    }
    else cout << "Unable to open whitelist.txt\nPressing on...\n";

    char json[profile_json.length()];
    strcpy(json, profile_json.c_str());
    // cout << json;

    // JSON
    Document d;
    d.Parse(json);

    Keymap keymaps[maxMaps];

    for(int a = 0;a < maxMaps;a++) {
      for(int b = 0;b < maxKeyTypes;b++) {
        Binding keyBind;
        keymaps[a].bindings[b] = keyBind;
      }
    }

    const Value& bindings = d["bindings"];
    int kmCount = 0;
    for(SizeType a = 0;a < bindings.Size();a++) {
      const Value& km = bindings[a];
      for(SizeType b = 0;b < km.Size();b++) {

        const Value& bind = km[b];
        Binding keyBind;
        keyBind.parseBind(bind);

        for(int c = 0;c < maxKeyTypes;c++) {
          if(keyBind.origin == static_cast<ScanCode>(c)) {
            keymaps[kmCount].bindings[c] = keyBind;
          }
        }
      }
      kmCount++;
    }


    // Interception
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    wchar_t hardware_id[500];

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP | INTERCEPTION_FILTER_KEY_E0);

    int curMap = 0;
    bool keyHeld[maxKeyTypes];
    int keyInMap[maxKeyTypes];
    for(int a = 0;a < maxKeyTypes;a++) {
      keyHeld[a] = false;
      keyInMap[a] = 0;
    }
    bool profileDisabled = false;
    while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        bool sendOld = true;

        size_t length = interception_get_hardware_id(context, device, hardware_id, sizeof(hardware_id));

        if(interception_is_keyboard(device))
        {
            bool doAction = false;
            if(!useWhitelist) doAction = true;
            else {
              for(int hwid = 0;hwid < 10 && !doAction;hwid++) {
                wstring hwid2(hardware_id);
                if(whitelist[hwid] == string(hwid2.begin(), hwid2.end())) {
                  doAction = true;
                }
              }
            }
            sendOld = false;

            InterceptionKeyStroke &keystroke = *(InterceptionKeyStroke *) &stroke;

            if(keystroke.code == SCANCODE_F1 && keystroke.state == INTERCEPTION_KEY_DOWN) {
              profileDisabled = !profileDisabled;
              if(profileDisabled) {
                PlaySound((LPCSTR) "deactivate_profile.wav", NULL, SND_FILENAME | SND_ASYNC);
              }
              else {
                PlaySound((LPCSTR) "activate_profile.wav", NULL, SND_FILENAME | SND_ASYNC);
              }
            }

            bool keyDown = true;
            if(keystroke.state == INTERCEPTION_KEY_UP + INTERCEPTION_KEY_E0 || keystroke.state == INTERCEPTION_KEY_UP) keyDown = false;

            if(!profileDisabled && doAction) {

              bool actionTaken = false;
              for(int a = 0;a < maxKeyTypes && !actionTaken;a++) {
                int m = keyInMap[a];
                Binding bind = keymaps[m].bindings[a];

                if(keystroke.code == bind.origin && bind.code != 0x00) {
                  if(keyDown) {
                    keyHeld[a] = true;
                    if(bind.originE0) keystroke.state -= INTERCEPTION_KEY_E0;
                    if(bind.shift) {
                      keystroke.code = SCANCODE_LSHIFT;
                      interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.ctrl) {
                      keystroke.code = SCANCODE_LCTRL;
                      interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.alt) {
                      keystroke.code = SCANCODE_LALT;
                      interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.originE0) keystroke.state += INTERCEPTION_KEY_E0;
                  }

                  keystroke.code = bind.code;
                  if(bind.originE0 && !bind.codeE0) keystroke.state -= INTERCEPTION_KEY_E0;
                  else if(!bind.originE0 && bind.codeE0) keystroke.state += INTERCEPTION_KEY_E0;
                  interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                  actionTaken = true;

                  if(!keyDown) {
                    keyHeld[a] = false;
                    if(bind.originE0) keystroke.state -= INTERCEPTION_KEY_E0;
                    Binding testBind = keymaps[curMap].bindings[a];
                    if(testBind.code != 0x00 || curMap == 0) {
                      keyInMap[a] = curMap;
                    }
                    if(bind.shift) {
                      keystroke.code = SCANCODE_LSHIFT;
                      interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.ctrl) {
                      keystroke.code = SCANCODE_LCTRL;
                      interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.alt) {
                      keystroke.code = SCANCODE_LALT;
                      interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.originE0) keystroke.state += INTERCEPTION_KEY_E0;
                  }

                } else if(keystroke.code == bind.origin && bind.keymap > 0) {
                  if(!keyDown) {
                    curMap = 0;
                    keyHeld[a] = false;
                    keyInMap[a] = curMap;
                  } else {
                    curMap = bind.keymap-1;
                    keyHeld[a] = true;
                  }

                  for(int b = 0;b < maxKeyTypes;b++) {
                    Binding testBind = keymaps[curMap].bindings[b];
                    if(!keyHeld[b] && (testBind.code != 0x00 || curMap == 0)) {
                      keyInMap[b] = curMap;
                    }
                  }
                  actionTaken = true;
                }
              }
            } else if(keystroke.code != SCANCODE_F1) {
              interception_send(context, device, (InterceptionStroke *)&keystroke, 1);
            }

        }

        // if(length > 0 && length < sizeof(hardware_id))
        //     wcout << hardware_id << endl;

        if(sendOld) {
          interception_send(context, device, &stroke, 1);
        }
    }

    interception_destroy_context(context);

    return 0;
}
