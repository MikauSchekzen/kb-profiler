#include "keymap.h"

Keymap::Keymap() {
  bindCount = 0;
}

void Keymap::addBind(Binding bind) {
  bindings[bindCount] = bind;
  bindCount++;
}

Binding Keymap::getBind(int index) {
  return bindings[index];
}
