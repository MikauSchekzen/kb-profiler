#include "binding.h"

const int maxMaps = 12;

class Keymap {
public:
  int bindCount;
  Binding bindings[maxKeyTypes];

  Keymap();
  void addBind(Binding bind);
  Binding getBind(int index);
};
