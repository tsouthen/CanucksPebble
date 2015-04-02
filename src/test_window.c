#include <pebble.h>
#include "test_window.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_logo_white;
static GFont s_res_bitham_42_bold;
static GFont s_res_bitham_30_black;
static GFont s_res_roboto_condensed_21;
static BitmapLayer *s_logo_layer;
static TextLayer *s_time_layer;
static TextLayer *s_textlayer_day;
static TextLayer *s_textlayer_date;
static TextLayer *s_textlayer_next;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_fullscreen(s_window, true);
  
  s_res_logo_white = gbitmap_create_with_resource(RESOURCE_ID_LOGO_WHITE);
  s_res_bitham_42_bold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  s_res_bitham_30_black = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  // s_logo_layer
  s_logo_layer = bitmap_layer_create(GRect(0, -2, 144, 144));
  bitmap_layer_set_bitmap(s_logo_layer, s_res_logo_white);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_logo_layer);
  
  // s_time_layer
  s_time_layer = text_layer_create(GRect(0, 125, 144, 42));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "12:34");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, s_res_bitham_42_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_time_layer);
  
  // s_textlayer_day
  s_textlayer_day = text_layer_create(GRect(1, -2, 75, 30));
  text_layer_set_background_color(s_textlayer_day, GColorClear);
  text_layer_set_text_color(s_textlayer_day, GColorWhite);
  text_layer_set_text(s_textlayer_day, "?");
  text_layer_set_font(s_textlayer_day, s_res_bitham_30_black);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_day);
  
  // s_textlayer_date
  s_textlayer_date = text_layer_create(GRect(95, -2, 48, 30));
  text_layer_set_background_color(s_textlayer_date, GColorClear);
  text_layer_set_text_color(s_textlayer_date, GColorWhite);
  text_layer_set_text(s_textlayer_date, "00");
  text_layer_set_text_alignment(s_textlayer_date, GTextAlignmentRight);
  text_layer_set_font(s_textlayer_date, s_res_bitham_30_black);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_date);
  
  // s_textlayer_next
  s_textlayer_next = text_layer_create(GRect(0, 107, 144, 29));
  text_layer_set_background_color(s_textlayer_next, GColorBlack);
  text_layer_set_text_color(s_textlayer_next, GColorWhite);
  text_layer_set_text(s_textlayer_next, "Thu 5:30 @ CHI");
  text_layer_set_text_alignment(s_textlayer_next, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_next, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_next);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  bitmap_layer_destroy(s_logo_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_textlayer_day);
  text_layer_destroy(s_textlayer_date);
  text_layer_destroy(s_textlayer_next);
  gbitmap_destroy(s_res_logo_white);
}
// END AUTO-GENERATED UI CODE

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

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_test_window(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_test_window(void) {
  window_stack_remove(s_window, true);
}

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
  text_layer_set_text(s_time_layer, formattedTime);
}

void update_date(struct tm *tick_time) {
  if (tick_time == NULL) {
    time_t temp = time(NULL); 
    tick_time = localtime(&temp);      
  }
  text_layer_set_text(s_textlayer_day, WEEKDAY_NAMES[0][tick_time->tm_wday]);
  strftime(date_buffer, DATE_BUFFER_BYTES, "%e", tick_time);  
  text_layer_set_text(s_textlayer_date, date_buffer);
}

void hide_next_game() {
  layer_set_hidden(text_layer_get_layer(s_textlayer_next), true);
}

void show_next_game() {
  layer_set_hidden(text_layer_get_layer(s_textlayer_next), false);
}

void toggle_next_game() {
  if (layer_get_hidden(text_layer_get_layer(s_textlayer_next)))
    show_next_game();
  else
    hide_next_game();
}
