#include "pebble.h"

static Window *window;

static TextLayer *temperature_layer;
static TextLayer *city_layer;
static TextLayer *time_layer;
static TextLayer *ago_layer;

static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static BitmapLayer *bt_icon_layer;
static GBitmap *bt_icon_bitmap= NULL;

static AppSync sync;
static uint8_t sync_buffer[64];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x2       // TUPLE_CSTRING
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN, //0
  RESOURCE_ID_IMAGE_CLOUD, //1
  RESOURCE_ID_IMAGE_RAIN, //2
  RESOURCE_ID_IMAGE_SNOW //3
};

// Bluetooth On Vibe pattern: ON for 200ms, OFF for 100ms, ON for 200ms:
static const uint32_t const segments_bt_on[] = { 200, 100, 200 };
// Bluetooth Off Vibe pattern: ON for 100ms, OFF for 200ms, ON for 100ms:
static const uint32_t const segments_bt_off[] = { 100, 200, 100 };
VibePattern vibe_pat_bt_on = {
  .durations = segments_bt_on,
  .num_segments = ARRAY_LENGTH(segments_bt_on),
};

VibePattern vibe_pat_bt_off = {
  .durations = segments_bt_off,
  .num_segments = ARRAY_LENGTH(segments_bt_off),
};

static char time_text[] = "00:00 AM";
static char time_text_buffer[] = "00:00 AM";
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) ;
void update_time_text(struct tm *tick_time) ;
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) ;
static void select_click_handler(ClickRecognizerRef recognizer, void *context) ;
static void refresh_data() ;
static void update_refresh_data_time() ;
void bluetooth_connection_callback(bool connected);

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case WEATHER_ICON_KEY:
      if (icon_bitmap) {
        gbitmap_destroy(icon_bitmap);
      }
      icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
      break;

    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      break;
	

    case WEATHER_CITY_KEY:
        text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
	  
  }
   update_refresh_data_time();
	
}

static void update_refresh_data_time(void){

	static char time_text1[] = "@00:00 AM";
    static char time_text_buffer1[] = "@00:00 AM";
	struct tm *t;
    time_t temp;
    temp = time(NULL);
    t = localtime(&temp);
	
	char *time_format1;
	if (clock_is_24h_style()) {
        time_format1 = "@%R";
    } else {
        time_format1 = "@%I:%M %p";
    }
	
    strftime(time_text_buffer1, sizeof(time_text_buffer1), time_format1,  t);
	
   // if (time_text_buffer1[0] == '0') {
    //    memmove(time_text_buffer1, &time_text_buffer1[1], sizeof(time_text_buffer1) - 1);
   // }
	//  memcpy(time_text1, time_text_buffer1, strlen(time_text1)+1);
     // text_layer_set_text( ago_layer, time_text1);
	 
}

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // refresh
  refresh_data();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // refresh
 refresh_data();
}
static void refresh_data() {
  text_layer_set_text(temperature_layer, "...");
  text_layer_set_text(city_layer, "Wait...");
 // text_layer_set_text(ago_layer, "...");
  send_cmd();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  //icon_layer = bitmap_layer_create(GRect(32, 40, 80, 50));
	
 // icon_layer = bitmap_layer_create(GRect(5, 40, 50, 50));

  //bt_icon_layer = bitmap_layer_create(GRect(0, 55, 144, 14));
	
  bt_icon_layer = bitmap_layer_create(GRect(0, 0, 144, 14));
	
  bitmap_layer_set_alignment(bt_icon_layer, GAlignRight);

  layer_add_child(window_layer, bitmap_layer_get_layer(bt_icon_layer));
	
  icon_layer = bitmap_layer_create(GRect(5, 55, 50, 55));
  
  bitmap_layer_set_alignment(icon_layer, GAlignCenter);

  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));
	
  //time_layer = text_layer_create(GRect(0, 0, 144, 68));
  time_layer = text_layer_create(GRect(0,15, 144, 68));
	
	
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  //text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
 
	
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
	

  //temperature_layer = text_layer_create(GRect(0, 95, 144, 68));
 /// temperature_layer = text_layer_create(GRect(60, 40, 70, 68));
  temperature_layer = text_layer_create(GRect(60, 68, 70, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font( FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));
	
	
  ago_layer = text_layer_create(GRect(60, 50, 70, 50));
  text_layer_set_text_color(ago_layer, GColorWhite);
  text_layer_set_background_color(ago_layer, GColorClear);
  text_layer_set_font(ago_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(ago_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(ago_layer));
	

  city_layer = text_layer_create(GRect(0, 110, 144, 80));
  text_layer_set_text_color(city_layer, GColorWhite);
  text_layer_set_background_color(city_layer, GColorClear);
  text_layer_set_font(city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(city_layer));

  Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "..."),
    TupletCString(WEATHER_CITY_KEY, "Wait..."),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
     send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }

 if (bt_icon_bitmap) {
    gbitmap_destroy(bt_icon_bitmap);
  }
  text_layer_destroy(city_layer);
  text_layer_destroy(temperature_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(ago_layer);
  bitmap_layer_destroy(icon_layer);
  bitmap_layer_destroy(bt_icon_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
	
  window_stack_push(window, animated);
   // Display
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  update_time_text(tick_time);
	
 //Bluetooth Connection - Show current connection state
	if (bluetooth_connection_service_peek()) {
		  bt_icon_bitmap= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_ON);
		  bitmap_layer_set_bitmap(bt_icon_layer, bt_icon_bitmap );
		// vibes_enqueue_custom_pattern(vibe_pat_bt_on);
    } else {
          bt_icon_bitmap= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_OFF);
		  bitmap_layer_set_bitmap(bt_icon_layer, bt_icon_bitmap );
		  vibes_enqueue_custom_pattern(vibe_pat_bt_off);
    }

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);

}

void bluetooth_connection_callback(bool connected) { 
  
	if (connected) {
		   bt_icon_bitmap= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_ON);
		   bitmap_layer_set_bitmap(bt_icon_layer, bt_icon_bitmap );
		  // vibes_enqueue_custom_pattern(vibe_pat_bt_on);
    } else {
           bt_icon_bitmap= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_OFF);
		   bitmap_layer_set_bitmap(bt_icon_layer, bt_icon_bitmap );
		   vibes_enqueue_custom_pattern(vibe_pat_bt_off);
	}
	
 }

void update_time_text(struct tm *tick_time) {
	char *time_format;
    
	if (clock_is_24h_style()) {
        time_format = "%R";
    } else {
        time_format = "%I:%M %p";
    }
	
    strftime(time_text_buffer, sizeof(time_text_buffer), time_format, tick_time);
    if (time_text_buffer[0] == '0') {
        memmove(time_text_buffer, &time_text_buffer[1], sizeof(time_text_buffer) - 1);
    }
	  memcpy(time_text, time_text_buffer, strlen(time_text)+1);
      text_layer_set_text( time_layer, time_text);
   
}


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	 update_time_text(tick_time);
	 if ((units_changed & HOUR_UNIT) == HOUR_UNIT) {
    	//Refresh weather
		  refresh_data() ;
	  }
 }


static void deinit(void) {
  tick_timer_service_unsubscribe();

  bluetooth_connection_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
