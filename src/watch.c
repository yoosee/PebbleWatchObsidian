#include <pebble.h>
#include "watch.h"
#include "health.h"

static Window *s_main_window;
static Layer *s_bg_layer, *s_date_layer, *s_hands_layer;

static GFont s_date_font, s_weather_font, s_steps_font;
static TextLayer *s_date_label, *s_weather_label, *s_steps_label;

static GFont s_time_font;
static TextLayer *s_time_3_label, *s_time_6_label, *s_time_9_label, *s_time_12_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;

static GBitmap *s_bt_icon_bitmap;
static BitmapLayer *s_bt_icon_layer;

static char s_date_buffer[12];

/* *** Bluetooth Callback *** */

// Bluetooth connection indicator (shown when disconnected)
static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

/* *** proc layer updates *** */

static void proc_bg_update (Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
    const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
    gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
    gpath_draw_filled(ctx, s_tick_paths[i]);
  }
  
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FACENUM_27));
  
    s_time_12_label = text_layer_create(PBL_IF_ROUND_ELSE(GRect(77, 2, 40, 34), GRect(60, 0, 40, 34)));
  text_layer_set_background_color(s_time_12_label, GColorClear);
  text_layer_set_text_color(s_time_12_label, GColorWhite); 
  text_layer_set_text(s_time_12_label, "12");
  text_layer_set_font(s_time_12_label, s_time_font);
  layer_add_child(s_date_layer, text_layer_get_layer(s_time_12_label));
  
    s_time_3_label = text_layer_create(PBL_IF_ROUND_ELSE(GRect(155, 74, 30, 34),GRect(124, 71, 30, 34)));
  text_layer_set_background_color(s_time_3_label, GColorClear);
  text_layer_set_text_color(s_time_3_label, GColorWhite);
  text_layer_set_text(s_time_3_label, "3");
  text_layer_set_font(s_time_3_label, s_time_font);
  layer_add_child(s_date_layer, text_layer_get_layer(s_time_3_label));
  
    s_time_6_label = text_layer_create(PBL_IF_ROUND_ELSE(GRect(81, 146, 30, 34),GRect(66, 136, 30, 34)));
  text_layer_set_background_color(s_time_6_label, GColorClear);
  text_layer_set_text_color(s_time_6_label, GColorWhite);
  text_layer_set_text(s_time_6_label, "6");
  text_layer_set_font(s_time_6_label, s_time_font);
  layer_add_child(s_date_layer, text_layer_get_layer(s_time_6_label));
  
    s_time_9_label = text_layer_create(PBL_IF_ROUND_ELSE(GRect(7, 74, 30, 34),GRect(0, 71, 30, 34)));
  text_layer_set_background_color(s_time_9_label, GColorClear);
  text_layer_set_text_color(s_time_9_label, GColorWhite);
  text_layer_set_text(s_time_9_label, "9");
  text_layer_set_font(s_time_9_label, s_time_font);
  layer_add_child(s_date_layer, text_layer_get_layer(s_time_9_label));
  
}

static void proc_hands_update (Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  // GPoint center = grect_center_point(&bounds);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);  
  
  // minute hand
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);  
  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);
  
  // hour hand
  graphics_context_set_fill_color(ctx, GColorJazzberryJam);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);
  
  // dot in the middle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void proc_date_update (Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d", t);
  text_layer_set_text(s_date_label, s_date_buffer);
}

/* *** tick handlers *** */

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  if(tick_time->tm_sec % 30 == 0) { // update in every 30 sec
    layer_mark_dirty(window_get_root_layer(s_main_window));
  }
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Update Health Steps every minute
  static char steps_buffer[] = MAX_HEALTH_STR;
  update_health(steps_buffer);
  text_layer_set_text(s_steps_label, steps_buffer);
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) { 
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

/* *** callbacks *** */

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  
  Tuple *is_fahrenheit_tuple = dict_find(iterator, KEY_IS_FAHRENHEIT);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    if(is_fahrenheit_tuple && is_fahrenheit_tuple->value->int8 > 0) {
      persist_write_bool(KEY_IS_FAHRENHEIT, true);
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int) (temp_tuple->value->int32 * 9 / 5 + 32)); // Fahrenheit  
    } else {
      persist_write_bool(KEY_IS_FAHRENHEIT, false);
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32); // Celsius by default  
    }
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s\n%s", conditions_buffer, temperature_buffer);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Weather: %s", weather_layer_buffer);
  
  text_layer_set_text(s_weather_label, weather_layer_buffer);
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


/* *** main window load & unload *** */

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create Background and add to main layer
  s_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_bg_layer, proc_bg_update);
  layer_add_child(window_layer, s_bg_layer);
  
  // Create date layer and add to main layer
  s_date_layer = layer_create(bounds);
  layer_set_update_proc(s_date_layer, proc_date_update);
  layer_add_child(window_layer, s_date_layer);
  
  s_date_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(118, 74, 44, 25),
    GRect(88, 74, 44, 25)));
  text_layer_set_text(s_date_label, s_date_buffer);
  text_layer_set_background_color(s_date_label, GColorClear);
  text_layer_set_text_color(s_date_label, GColorWhite);
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_24));
  text_layer_set_font(s_date_label, s_date_font);
  layer_add_child(s_date_layer, text_layer_get_layer(s_date_label));
  
  // Create weather and templature layer
  s_weather_label = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(110,105), bounds.size.w, 50));

  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text_color(s_weather_label, GColorTiffanyBlue);
  text_layer_set_text_alignment(s_weather_label, GTextAlignmentCenter);
  text_layer_set_text(s_weather_label, "LOADING");
  
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INFOTEXT_14));
  text_layer_set_font(s_weather_label, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_label));
  
  // Create Health Steps layer
  s_steps_label = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(45, 35), bounds.size.w, 20));
  
  text_layer_set_background_color(s_steps_label, GColorClear);
  text_layer_set_text_color(s_steps_label, GColorIndigo);
  text_layer_set_text_alignment(s_steps_label, GTextAlignmentCenter);
  s_steps_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INFOTEXT_14));
  text_layer_set_font(s_steps_label, s_steps_font);
  text_layer_set_text(s_steps_label, "00000");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_steps_label));
      
  // Create Bluetooth layer
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_DISCONNECTED);
  s_bt_icon_layer = bitmap_layer_create(GRect(36, 78, 19, 24));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
  // Create Hands Layer and add it to the top of Root Window
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, proc_hands_update);
  layer_add_child(window_layer, s_hands_layer);
}

static void main_window_unload(Window *window) {  
  layer_destroy(s_bg_layer);
  layer_destroy(s_date_layer);
  layer_destroy(s_hands_layer);
  
  text_layer_destroy(s_date_label);
  text_layer_destroy(s_weather_label);
  text_layer_destroy(s_steps_label);
  
  fonts_unload_custom_font(s_weather_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_steps_font);
  
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
}

/* *** init / deinit and setup functions *** */

static void setup_face_hands() {
  s_date_buffer[0] = '\0';
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);
  
  for(int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
  }
}

static void setup_callbacks() {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);

  // Register callbacks
  setup_callbacks();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload  
  });
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Setup hands and face
  setup_face_hands();
  
  // Subscribe Tick Timer
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void deinit() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);
  for(int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}