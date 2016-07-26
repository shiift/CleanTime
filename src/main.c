#include <pebble.h>

#define KEY_ICON 0
#define KEY_TEMPMAX 1
#define KEY_TEMPMIN 2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static TextLayer *s_temp_high_layer;
static TextLayer *s_temp_low_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap;
static int s_weather_state = 0;

static TextLayer *s_battery_layer;
static BitmapLayer *s_battery_icon_layer;
static GBitmap *s_battery_icon;
static int s_battery_state = 0;

static GFont s_time_font;
static GFont s_date_font;

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_NA, // 0
  RESOURCE_ID_IMAGE_SUN, // 1
  RESOURCE_ID_IMAGE_CLOUD, // 2
  RESOURCE_ID_IMAGE_RAIN, // 3
  RESOURCE_ID_IMAGE_THUNDER, // 4
  RESOURCE_ID_IMAGE_SNOW, // 5
  RESOURCE_ID_IMAGE_PART // 6
};

static const uint32_t BATTERY_ICONS[] = {
  RESOURCE_ID_IMAGE_BATT0, // 0
  RESOURCE_ID_IMAGE_BATT1, // 1
  RESOURCE_ID_IMAGE_BATT2, // 2
  RESOURCE_ID_IMAGE_BATT3, // 3
  RESOURCE_ID_IMAGE_BATT_CHG // 4
};

char *itoa(int num) {
  static char buff[20] = {};
  int i = 0, tmp = num, len = 0;
  char *string = buff;
  while (tmp) {
    tmp /= 10;
    len++;
  }
  for (i = 0; i < len; i++) {
    buff[(len-1)-i] = '0' + (num % 10);
    num /= 10;
  }
  buff[i] = '\0';
  if (len == 0) {
    return "0";
  }
  return string;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Write the current hours and minutes into a buffer
  static char s_buffer[6];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
            "%k:%M" : "%l:%M", tick_time);
  
  // Write the current date into a buffer
  static char s_date_buffer[11];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %e", tick_time);
  
  // Display this date/time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer[0] == ' ' ?
                        s_buffer+1 : s_buffer);
  text_layer_set_text(s_date_layer, s_date_buffer);
  
  if (tick_time->tm_min == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void battery_handler(BatteryChargeState charge) {
  int new_battery_state;
  uint8_t charge_percent = charge.charge_percent;
  text_layer_set_text(s_battery_layer, itoa(charge_percent));
  if (charge.is_charging) {
    new_battery_state = 4;
  } else if (charge_percent < 25) {
    new_battery_state = 0;
  } else if (charge_percent < 50) {
    new_battery_state = 1;
  } else if (charge_percent < 75) {
    new_battery_state = 2;
  } else {
    new_battery_state = 3;
  }
  if (new_battery_state != s_battery_state) {
    gbitmap_destroy(s_battery_icon);
    s_battery_state = new_battery_state;
    s_battery_icon = gbitmap_create_with_resource(BATTERY_ICONS[s_battery_state]);
    bitmap_layer_set_compositing_mode(s_battery_icon_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_battery_icon_layer, s_battery_icon);
  }
}

static void main_window_load(Window *window) {
  // Get information about the widnow
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create GFonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_COOLVETICA_68));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_COOLVETICA_24));
  
  // Create the Time TextLayer with specific bounds
  s_time_layer = text_layer_create(
    GRect(0, 50, bounds.size.w, 100));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create the Date TextLayer with specific bounds
  s_date_layer = text_layer_create(
    GRect(0, 125, bounds.size.w, 100));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "WWW MMM DD");
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Create the weather icon layer
  s_icon_layer = bitmap_layer_create(GRect(0, 5, bounds.size.w, 60));
  s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[0]);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  
  // Create the temp high text layer
  s_temp_high_layer = text_layer_create(GRect(104, 0, 40, 60));
  text_layer_set_background_color(s_temp_high_layer, GColorClear);
  text_layer_set_text_color(s_temp_high_layer, GColorWhite);
  text_layer_set_text(s_temp_high_layer, "");
  text_layer_set_font(s_temp_high_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_temp_high_layer, GTextAlignmentCenter);
  
  // Create the temp low text layer
  s_temp_low_layer = text_layer_create(GRect(104, 20, 40, 60));
  text_layer_set_background_color(s_temp_low_layer, GColorClear);
  text_layer_set_text_color(s_temp_low_layer, GColorWhite);
  text_layer_set_text(s_temp_low_layer, "");
  text_layer_set_font(s_temp_low_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_temp_low_layer, GTextAlignmentCenter);
  
  // Create the battery icon layer
  s_battery_icon_layer = bitmap_layer_create(GRect(0, 5, 40, 14));
  s_battery_icon = gbitmap_create_with_resource(BATTERY_ICONS[s_battery_state]);
  bitmap_layer_set_compositing_mode(s_battery_icon_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_battery_icon_layer, s_battery_icon);
  
  // Create the battery text layer
  s_battery_layer = text_layer_create(GRect(0, 16, 38, 60));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text(s_battery_layer, "");
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  
  // Add as child layers to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_temp_high_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_temp_low_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_battery_icon_layer));
}

static void main_window_unload(Window *window) {
  // Destroy Layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_temp_high_layer);
  text_layer_destroy(s_temp_low_layer);
  bitmap_layer_destroy(s_icon_layer);
  gbitmap_destroy(s_icon_bitmap);
  text_layer_destroy(s_battery_layer);
  bitmap_layer_destroy(s_battery_icon_layer);
  gbitmap_destroy(s_battery_icon);
}

static void init_layers() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  tick_handler(tick_time, MINUTE_UNIT);
  
  battery_handler(battery_state_service_peek());
}

// AppMessage handlers
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store weather information from pebblejs
  static char temp_max_buffer[8];
  static char temp_min_buffer[8];
  static int weather_icon_id;
  
  Tuple *temp_max_tuple = dict_find(iterator, KEY_TEMPMAX);
  Tuple *temp_min_tuple = dict_find(iterator, KEY_TEMPMIN);
  Tuple *weather_icon_tuple = dict_find(iterator, KEY_ICON);
  
  
  if (temp_max_tuple && temp_min_tuple && weather_icon_tuple) {
    snprintf(temp_max_buffer, sizeof(temp_max_buffer), "%d°", (int)temp_max_tuple->value->int32);
    snprintf(temp_min_buffer, sizeof(temp_min_buffer), "%d°", (int)temp_min_tuple->value->int32);
    weather_icon_id = (int)weather_icon_tuple->value->int32;
    text_layer_set_text(s_temp_high_layer, temp_max_buffer);
    text_layer_set_text(s_temp_low_layer, temp_min_buffer);
    gbitmap_destroy(s_icon_bitmap);
    s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[weather_icon_id]);
    bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  }
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

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_set_background_color(s_main_window, GColorBlack);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register with BatteryStateService
  battery_state_service_subscribe(battery_handler);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // set initial state of layers
  init_layers();
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage (TODO: Find actual size)
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}