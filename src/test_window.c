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
  s_textlayer_next = text_layer_create(GRect(1, 110, 143, 27));
  text_layer_set_background_color(s_textlayer_next, GColorBlack);
  text_layer_set_text_color(s_textlayer_next, GColorWhite);
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

TextLayer* get_time_layer() {
  return s_time_layer;
}
TextLayer* get_textlayer_day() {
  return s_textlayer_day;
}
TextLayer* get_textlayer_date() {
  return s_textlayer_date;
}
TextLayer* get_textlayer_next() {
  return s_textlayer_next;
}
