#include "interception.h"
#include "src/utils.h"

#include "src/scancodes.h"
#include "src/keymap.h"

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <fstream>
#include <string>
#if       _WIN32_WINNT < 0x0500
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT   0x0500
#endif
#include "mingw.thread.h"
#include <windows.h>
#include <mmsystem.h>
#include <sstream>

using namespace rapidjson;
using namespace std;


class KeyOptions {
public:
  bool keyHeld[maxKeyTypes];
  int keyInMap[maxKeyTypes];
  int keyRapidfire[maxKeyTypes];
  int keyOptions;
  int curMap;

  KeyOptions(int keyCount) {
    keyOptions = keyCount;
    curMap = 0;

    for(int a = 0;a < keyOptions;a++) {
      keyHeld[a] = false;
      keyInMap[a] = 0;
      keyRapidfire[a] = 0;
    }
  }
};

void rapidfireFunc(InterceptionContext *context, InterceptionDevice *device, KeyOptions *keyOpts, Keymap *keymaps) {
  while(true) {
    for(int a = 0;a < maxKeyTypes;a++) {
      if((*keyOpts).keyRapidfire[a] > 0) {
        int m = (*keyOpts).keyInMap[a];
        Binding bind = keymaps[m].bindings[a];

        (*keyOpts).keyRapidfire[a]--;
        if((*keyOpts).keyRapidfire[a] == 10) {
          InterceptionKeyStroke keystroke;
          keystroke.state = INTERCEPTION_KEY_UP;
          if(bind.codeE0) keystroke.state += INTERCEPTION_KEY_E0;
          keystroke.code = bind.code;
          interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
        }
        else if((*keyOpts).keyRapidfire[a] == 0) {
          InterceptionKeyStroke keystroke;
          keystroke.state = INTERCEPTION_KEY_DOWN;
          if(bind.codeE0) keystroke.state += INTERCEPTION_KEY_E0;
          keystroke.code = bind.code;
          interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
          (*keyOpts).keyRapidfire[a] = bind.rapidfire;
        }
      }
    }
    Sleep(1);
  }
}

void handleInterception(InterceptionContext *context, InterceptionDevice *device, InterceptionStroke& stroke, KeyOptions *keyOpts, Keymap *keymaps, string *whitelist) {
    wchar_t hardware_id[500];

    bool profileDisabled = false;

    *context = interception_create_context();

    interception_set_filter(*context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP | INTERCEPTION_FILTER_KEY_E0);

    while(interception_receive(*context, *device = interception_wait(*context), &stroke, 1) > 0)
    {
        bool sendOld = true;

        size_t length = interception_get_hardware_id(*context, *device, hardware_id, sizeof(hardware_id));

        if(interception_is_keyboard(*device))
        {
          InterceptionKeyStroke &keystroke = *(InterceptionKeyStroke *) &stroke;

            bool doAction = false;
            for(int hwid = 0;hwid < 10 && !doAction;hwid++) {
              wstring hwid2(hardware_id);
              if(whitelist[hwid] == string(hwid2.begin(), hwid2.end())) {
                doAction = true;
              }
            }
            sendOld = false;

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
                int m = (*keyOpts).keyInMap[a];
                Binding bind = keymaps[m].bindings[a];

                if(keystroke.code == bind.origin && bind.code != 0x00) {
                  if(keyDown) {
                    (*keyOpts).keyHeld[a] = true;
                    if(bind.originE0) keystroke.state -= INTERCEPTION_KEY_E0;
                    if(bind.shift) {
                      keystroke.code = SCANCODE_LSHIFT;
                      interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.ctrl) {
                      keystroke.code = SCANCODE_LCTRL;
                      interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.alt) {
                      keystroke.code = SCANCODE_LALT;
                      interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.originE0) keystroke.state += INTERCEPTION_KEY_E0;
                  }

                  keystroke.code = bind.code;
                  if(bind.originE0 && !bind.codeE0) keystroke.state -= INTERCEPTION_KEY_E0;
                  else if(!bind.originE0 && bind.codeE0) keystroke.state += INTERCEPTION_KEY_E0;
                  // if((*keyOpts).keyRapidfire[a] == 0 && keyDown) {
                    interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                  // }
                  actionTaken = true;
                  if(keyDown && bind.rapidfire > 10 && (*keyOpts).keyRapidfire[a] == 0) {
                    (*keyOpts).keyRapidfire[a] = bind.rapidfire;
                  }
                  if(!keyDown) {
                    (*keyOpts).keyRapidfire[a] = 0;
                  }

                  if(!keyDown) {
                    (*keyOpts).keyHeld[a] = false;
                    if(bind.originE0) keystroke.state -= INTERCEPTION_KEY_E0;
                    Binding testBind = keymaps[(*keyOpts).curMap].bindings[a];
                    if(testBind.code != 0x00 || (*keyOpts).curMap == 0) {
                      (*keyOpts).keyInMap[a] = (*keyOpts).curMap;
                    }
                    if(bind.shift) {
                      keystroke.code = SCANCODE_LSHIFT;
                      interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.ctrl) {
                      keystroke.code = SCANCODE_LCTRL;
                      interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.alt) {
                      keystroke.code = SCANCODE_LALT;
                      interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
                    }
                    if(bind.originE0) keystroke.state += INTERCEPTION_KEY_E0;
                  }

                }
                else if(keystroke.code == bind.origin && bind.keymap > 0) {
                  if(!keyDown) {
                    (*keyOpts).curMap = 0;
                    (*keyOpts).keyHeld[a] = false;
                    (*keyOpts).keyInMap[a] = (*keyOpts).curMap;
                  } else {
                    (*keyOpts).curMap = bind.keymap-1;
                    (*keyOpts).keyHeld[a] = true;
                  }

                  for(int b = 0;b < maxKeyTypes;b++) {
                    Binding testBind = keymaps[(*keyOpts).curMap].bindings[b];
                    if(!(*keyOpts).keyHeld[b] && (testBind.code != 0x00 || (*keyOpts).curMap == 0)) {
                      (*keyOpts).keyInMap[b] = (*keyOpts).curMap;
                    }
                  }
                  actionTaken = true;
                }
              }
            }
            else if(keystroke.code != SCANCODE_F1) {
              interception_send(*context, *device, (InterceptionStroke *)&keystroke, 1);
            }

        }

        if(sendOld) {
          interception_send(*context, *device, &stroke, 1);
        }
    }

    interception_destroy_context(*context);
}


int main(int argc, char *argv[])
{
    // Auto minimize
    ShowWindow(GetConsoleWindow(), SW_MINIMIZE);

    // Load profile
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

    // Load whitelist
    ifstream file;
    string whitelist[10];
    file.open("whitelist.txt");
    if(file.is_open()) {
      int a = 0;
      while(getline(file, line)) {
        whitelist[a] = line;
        a++;
      }
      profile_file.close();
    }
    else cout << "Unable to open whitelist.txt\nPressing on...\n";

	  const int profileJSONLen = profile_json.length();
    char json[profileJSONLen];
    strcpy(json, profile_json.c_str());

    // JSON
    Document d;
    d.Parse(json);

    // Parse keymaps
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
    KeyOptions keyOpts(maxMaps);

    raise_process_priority();

    thread rapidfireThread(rapidfireFunc, &context, &device, &keyOpts, keymaps);
    // rapidfireThread.join();

    handleInterception(&context, &device, stroke, &keyOpts, keymaps, whitelist);

    return 0;
}
