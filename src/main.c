#include <pebble.h>
#include "test_window.h"
#include "helpers.h"  
#include "PDUtils.h"
  
#define KEY_JS_READY   0
#define KEY_LAST_TEXT  1
#define KEY_LAST_START 2
#define KEY_LAST_END   3
#define KEY_NEXT_TEXT  4
#define KEY_NEXT_START 5
#define KEY_NEXT_END   6
#define KEY_ERROR      99
  
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
  APP_LOG(APP_LOG_LEVEL_INFO, "Sending int message...");
  app_message_outbox_send();
  //set_next_game_text("sending");
}

static void sendRequestIfNecessary(time_t now) {
  if (s_nextStart < now) {
    send_int(0, 0);          
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t now = p_mktime(tick_time);
  update_time(tick_time);
  if (s_wday != tick_time->tm_wday || s_mday != tick_time->tm_mday) {
    s_wday = tick_time->tm_wday;
    s_mday = tick_time->tm_mday;
    update_date(tick_time);
    sendRequestIfNecessary(now);
  }
  
  //vibrate if next game is starting
  if (s_nextStart != 0 && s_nextStart == now) {
    vibes_double_pulse();
  }
}

static void setNextGameTextFromState() {
  set_next_game_text(s_state == NEXT_LAST_NextShown ? s_nextText : s_lastText);  
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  s_state = (s_state + 1) % 3;

  switch (s_state) {
    case NEXT_LAST_NotShown:
      hide_next_game();
      break;
    
    case NEXT_LAST_NextShown:
      if (s_nextEnd == -1) {
        sendRequestIfNecessary(time(NULL));
        strcpy(s_nextText, "loading...");
        strcpy(s_lastText, s_nextText);
      }
    case NEXT_LAST_LastShown:
      setNextGameTextFromState();
      show_next_game();
      break;
  }
}

static void clearVarsAndPersistedData() {
  //clear out vars and persisted data
  s_nextText[0] = 0;
  s_lastText[0] = 0;
  s_nextStart = 0;
  s_nextEnd = 0;
  persist_delete(KEY_LAST_TEXT);
  persist_delete(KEY_NEXT_TEXT);
  persist_delete(KEY_NEXT_START);
  persist_delete(KEY_NEXT_END);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Msg %d received", (int) t->key);

    // Process this pair's key
    switch (t->key) {
      case KEY_JS_READY:
        sendRequestIfNecessary(time(NULL));
        break;
      
      case KEY_LAST_TEXT:
        clearVarsAndPersistedData();
        strncpy(s_lastText, t->value->cstring, sizeof(s_lastText));
        persist_write_string(KEY_LAST_TEXT, s_lastText);
        if (s_state == NEXT_LAST_LastShown) {
          set_next_game_text(s_lastText);
        }
        break;
      
      case KEY_NEXT_TEXT:
        strncpy(s_nextText, t->value->cstring, sizeof(s_nextText));
        persist_write_string(KEY_NEXT_TEXT, s_nextText);
        if (s_state == NEXT_LAST_NextShown) {
          set_next_game_text(s_nextText);
        }
        break;
      
      case KEY_NEXT_START:
        s_nextStart = t->value->uint32;
        persist_write_int(KEY_NEXT_START, s_nextStart);
        break;

      case KEY_NEXT_END:
        s_nextEnd = t->value->uint32;
        persist_write_int(KEY_NEXT_END, s_nextEnd);
        break;
      
      case KEY_ERROR:
        clearVarsAndPersistedData();
        s_nextEnd = -1; //signifies we have an error condition, so refresh when we can
        snprintf(s_nextText, sizeof(s_nextText), "Error: %d", t->value->int16);
        strcpy(s_lastText, s_nextText);
        set_next_game_text(s_nextText);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  //set_next_game_text("msg dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! reason: %d", reason);
  //set_next_game_text("send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  //set_next_game_text("send success");
}

int main(void) {
  show_test_window();

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

  setNextGameTextFromState();

  update_time(NULL);
  update_date(NULL);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  app_event_loop();
  hide_test_window();
}
