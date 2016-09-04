#include <pebble.h>
#include "watch.h"

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}