#include <pebble.h>
#include "test_window.h"
#include "helpers.h"

const char WEEKDAY_NAMES[6][7][5] = { // 3 chars, 1 for utf-8, 1 for terminating 0
  {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
  {"So",  "Mo",  "Di",  "Mi",  "Do",  "Fr",  "Sa" },
  {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"},
  {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
  {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"},
  {"Sön", "Mån", "Tis", "Ons", "Tor", "Fre", "Lör"},
};

#define DATE_BUFFER_BYTES 32
static char date_buffer[DATE_BUFFER_BYTES];

void update_time(struct tm *tick_time) {
  // Get a tm structure
  if (tick_time == NULL) {
    time_t temp = time(NULL); 
    tick_time = localtime(&temp);      
  }
  
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  char* formattedTime = &buffer[0];

  // Write the current hours and minutes into the buffer
  if (clock_is_24h_style()) {
    //Use 2h hour format
    strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
    if (buffer[0] == '0')
      formattedTime = &buffer[1];
  }

  // Display this time on the TextLayer
  text_layer_set_text(get_time_layer(), formattedTime);
}

void update_date(struct tm *tick_time) {
  if (tick_time == NULL) {
    time_t temp = time(NULL); 
    tick_time = localtime(&temp);      
  }
  text_layer_set_text(get_textlayer_day(), WEEKDAY_NAMES[0][tick_time->tm_wday]);
  strftime(date_buffer, DATE_BUFFER_BYTES, "%e", tick_time);  
  text_layer_set_text(get_textlayer_date(), date_buffer);
}

void hide_next_game() {
  layer_set_hidden(text_layer_get_layer(get_textlayer_next()), true);
}

void show_next_game() {
  layer_set_hidden(text_layer_get_layer(get_textlayer_next()), false);
}

void toggle_next_game() {
  if (layer_get_hidden(text_layer_get_layer(get_textlayer_next())))
    show_next_game();
  else
    hide_next_game();
}

void set_next_game_text(char* text) {
  text_layer_set_text(get_textlayer_next(), text);
}
