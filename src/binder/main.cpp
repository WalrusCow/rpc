#include "binder/Binder.hpp"

int main(void) {
  Binder binder;
  binder.connect();
  binder.run();
}
