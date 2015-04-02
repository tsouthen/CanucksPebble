#include <pebble.h>
#include "test_window.h"
  
int s_wday = -1;
int s_mday = -1;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  if (s_wday != tick_time->tm_wday || s_mday != tick_time->tm_mday) {
    s_wday = tick_time->tm_wday;
    s_mday = tick_time->tm_mday;
    update_date(tick_time);
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  toggle_next_game();
}

int main(void) {
  show_test_window();
  hide_next_game();
  
  update_time(NULL);
  update_date(NULL);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  
  app_event_loop();
  hide_test_window();
}
