#include <pebble.h>
#include "test_window.h"
#include "helpers.h"  

#define KEY_JS_READY   0
#define KEY_LAST_TEXT  1
#define KEY_LAST_START 2
#define KEY_LAST_END   3
#define KEY_NEXT_TEXT  4
#define KEY_NEXT_START 5
#define KEY_NEXT_END   6
#define KEY_NEXT_LAST  7
  
enum NextLastState {
  NEXT_LAST_NotShown = 0,
  NEXT_LAST_NextShown = 1,
  NEXT_LAST_LastShown = 2
};

int s_wday = -1;
int s_mday = -1;
char s_nextText[32];
char s_lastText[32];
time_t s_nextStart = 0;
time_t s_nextEnd = 0;
enum NextLastState s_state = 1;

static void send_int(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, key, &value, sizeof(int), true);
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_INFO, "Sending int message...");
  set_next_game_text("sending...");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  if (s_wday != tick_time->tm_wday || s_mday != tick_time->tm_mday) {
    s_wday = tick_time->tm_wday;
    s_mday = tick_time->tm_mday;
    update_date(tick_time);
    
    //get updated next/last game info
    send_int(KEY_NEXT_LAST, 0);
  }
  
  //vibrate if next game is starting
  if (s_nextStart != 0 && s_nextStart == mktime(tick_time)) {
    vibes_double_pulse();
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  toggle_next_game();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  set_next_game_text("msg recd");
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];
    snprintf(s_buffer, sizeof(s_buffer), "Msg %lu received", t->key);
    APP_LOG(APP_LOG_LEVEL_INFO, s_buffer);

    // Process this pair's key
    switch (t->key) {
      case KEY_JS_READY: {
        //if s_nextStart is out of date, request new data
        if (s_state != NEXT_LAST_NotShown) {
          time_t now = time(NULL);
          if (s_nextStart < now)
            send_int(KEY_NEXT_LAST, 0);          
        }
        break;
      }
      
      case KEY_NEXT_LAST:
        //clear out vars and persisted data
        s_nextText[0] = 0;
        s_lastText[0] = 0;
        s_nextStart = 0;
        s_nextEnd = 0;
        persist_delete(KEY_LAST_TEXT);
        persist_delete(KEY_NEXT_TEXT);
        persist_delete(KEY_NEXT_START);
        persist_delete(KEY_NEXT_END);
        break;
      
      case KEY_LAST_TEXT:
        snprintf(s_lastText, sizeof(s_lastText), "%s", t->value->cstring);
        persist_write_string(KEY_LAST_TEXT, s_lastText);
        break;
      
      case KEY_NEXT_TEXT:
        snprintf(s_nextText, sizeof(s_nextText), "%s", t->value->cstring);
        set_next_game_text(s_nextText);
        persist_write_string(KEY_NEXT_TEXT, s_nextText);
        break;
      
      case KEY_NEXT_START:
        s_nextStart = t->value->uint32;
        persist_write_int(KEY_NEXT_START, s_nextStart);
        break;

      case KEY_NEXT_END:
        s_nextEnd = t->value->uint32;
        persist_write_int(KEY_NEXT_END, s_nextEnd);
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
  //register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  //open app message
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  //get persisted data
  if (persist_read_string(KEY_NEXT_TEXT, s_nextText, sizeof(s_nextText)) > 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Read persisted next text");
  }
  s_nextStart = persist_read_int(KEY_NEXT_START);
  if (s_nextStart) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Read persisted next start");
  }
  s_nextEnd = persist_read_int(KEY_NEXT_END);
  if (s_nextEnd) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Read persisted next end");
  }  
  if (persist_read_string(KEY_LAST_TEXT, s_lastText, sizeof(s_lastText)) > 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Read persisted last text");
  }
  if (s_nextText[0])
    set_next_game_text(s_nextText);
  else if (s_lastText[0])
    set_next_game_text(s_lastText);

  show_test_window();

  update_time(NULL);
  update_date(NULL);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  
  app_event_loop();
  hide_test_window();
}
