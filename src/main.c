#include <pebble.h>
#include "test_window.h"
#include "helpers.h"  
  
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

static void send_int(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, key, &value, sizeof(int), true);
  app_message_outbox_send();
  set_next_game_text("sending...");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  set_next_game_text("msg recd");
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];

    // Process this pair's key
    switch (t->key) {
      case 0:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Msg 0 received");
        // Copy value and display
        snprintf(s_buffer, sizeof(s_buffer), "%s", t->value->cstring);
        set_next_game_text(s_buffer);
        break;
      
      case 1:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Msg 1 received");
        send_int(0, 0);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  set_next_game_text("msg dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  static char s_errMsg[64];
  snprintf(s_errMsg, sizeof(s_errMsg), "Outbox send failed! reason: %d", reason);
  APP_LOG(APP_LOG_LEVEL_ERROR, s_errMsg);
  set_next_game_text("send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  set_next_game_text("send success");
}

int main(void) {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  show_test_window();

  //hide_next_game();
  //request updated next game
  
  update_time(NULL);
  update_date(NULL);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  
  app_event_loop();
  hide_test_window();
}
