#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>

Preferences prefs;

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

int x, y, z;
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

float ml_to_seconds(int ml) {
  return ml * 0.1f;
}

unsigned long last_dispense[2] = { 0, 0 };
unsigned long total_dispensed_ml[2] = { 0, 0 };
int frequency_hours[2] = { 24, 168 };

int slider_values[2] = { 0, 0 };
lv_obj_t *slider_labels[2];
lv_obj_t *stats_tab = nullptr;

lv_obj_t *tabview = nullptr;  // add this line at global scope

void open_valve(int pin, int ml) {
  digitalWrite(pin, HIGH);
  delay((int)(ml_to_seconds(ml) * 1000));
  digitalWrite(pin, LOW);
  Serial.printf("Valve on pin %d opened for %d ml\n", pin, ml);

  int i = (pin == 22) ? 0 : 1;
  last_dispense[i] = millis();
  total_dispensed_ml[i] += ml;

  Serial.printf("[LOG] open_valve(): last_dispense[%d] = %lu (millis)\n", i, last_dispense[i]);
  Serial.printf("[LOG] open_valve(): total_dispensed_ml[%d] = %lu\n", i, total_dispensed_ml[i]);

  prefs.begin("water", false);
  prefs.putULong(("total" + String(i)).c_str(), total_dispensed_ml[i]);
  prefs.end();
}

void log_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void slider_event_handler(lv_event_t *e) {
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int index = *(int *)lv_event_get_user_data(e);
  slider_values[index] = lv_slider_get_value(slider);
  lv_label_set_text_fmt(slider_labels[index], "%d ml", slider_values[index]);

  Serial.printf("Slider %d set to %d ml\n", index + 1, slider_values[index]);

  prefs.begin("water", false);
  prefs.putInt(("ml" + String(index)).c_str(), slider_values[index]);
  prefs.end();

  // Serial.printf("Saved ml[%d] = %d to NVS\n", index, slider_values[index]);
}

void dropdown_event_handler(lv_event_t *e) {
  lv_obj_t *dd = (lv_obj_t *)lv_event_get_target(e);
  int index = *(int *)lv_event_get_user_data(e);
  int sel = lv_dropdown_get_selected(dd);
  frequency_hours[index] = sel == 0 ? 24 : 168;

  prefs.begin("water", false);
  prefs.putInt(("freq" + String(index)).c_str(), frequency_hours[index]);
  prefs.end();

  // Serial.printf("Saved freq[%d] = %d to NVS\n", index, frequency_hours[index]);
}

void button_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  int *pin = (int *)lv_event_get_user_data(e);
  if (code == LV_EVENT_CLICKED) {
    int index = (*pin == 22) ? 0 : 1;


    Serial.printf("Opening valve %i", index + 1);

    open_valve(*pin, slider_values[index]);
  }
}

void create_valve_tab(lv_obj_t *parent, const char *label_text, int *slider_index, int pin) {
  int *pin_ptr = new int(pin);

  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text_fmt(label, "%s", label_text);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *slider = lv_slider_create(parent);
  lv_slider_set_range(slider, 0, 1000);
  lv_obj_set_width(slider, 200);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_add_event_cb(slider, slider_event_handler, LV_EVENT_VALUE_CHANGED, slider_index);

  slider_labels[*slider_index] = lv_label_create(parent);
  lv_label_set_text_fmt(slider_labels[*slider_index], "%d ml");
  lv_obj_align(slider_labels[*slider_index], LV_ALIGN_LEFT_MID, 20, 0);
  lv_obj_set_style_text_font(slider_labels[*slider_index], &lv_font_montserrat_18, 0);

  lv_slider_set_value(slider, slider_values[*slider_index], LV_ANIM_OFF);
  lv_label_set_text_fmt(slider_labels[*slider_index], "%d ml", slider_values[*slider_index]);

  lv_obj_t *label2 = lv_label_create(parent);
  lv_label_set_text(label2, "every");
  lv_obj_set_style_text_font(label2, &lv_font_montserrat_20, 0);
  lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *dd = lv_dropdown_create(parent);
  lv_obj_set_size(dd, 100, 40);
  lv_dropdown_set_options(dd, "Day\nWeek\n");
  lv_obj_align(dd, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_dropdown_set_selected(dd, frequency_hours[*slider_index] == 24 ? 0 : 1);
  lv_obj_add_event_cb(dd, dropdown_event_handler, LV_EVENT_VALUE_CHANGED, slider_index);

  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, 120, 40);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_ALL, pin_ptr);
  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "TEST AMOUNT");
  lv_obj_center(btn_label);
}

void update_stats_tab() {
  if (!stats_tab) return;
  lv_obj_clean(stats_tab);

  for (int i = 0; i < 2; i++) {
    char buf[64];
    if (last_dispense[i] > 0) {
      unsigned long now = millis();
      unsigned long elapsed = now - last_dispense[i];
      if (elapsed < 3600000UL) {
        int minutes = elapsed / 60000UL;
        minutes = max(minutes, 1);
        snprintf(buf, sizeof(buf), "Valve %d last: %d min ago", i + 1, minutes);
      } else {
        int hours = elapsed / 3600000UL;
        snprintf(buf, sizeof(buf), "Valve %d last: %d hr ago", i + 1, hours);
      }
    } else {
      snprintf(buf, sizeof(buf), "Valve %d last: never", i + 1);
    }
    lv_obj_t *label = lv_label_create(stats_tab);
    lv_label_set_text(label, buf);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 30 + i * 40);

    char totalBuf[64];
    snprintf(totalBuf, sizeof(totalBuf), "Total: %lu ml", total_dispensed_ml[i]);
    lv_obj_t *totalLabel = lv_label_create(stats_tab);
    lv_label_set_text(totalLabel, totalBuf);
    lv_obj_align(totalLabel, LV_ALIGN_TOP_LEFT, 10, 50 + i * 40);
  }
}

void lv_create_main_gui(void) {
  static int pin_22 = 22, pin_27 = 27;
  static int *slider_0 = new int(0);
  static int *slider_1 = new int(1);

  tabview = lv_tabview_create(lv_screen_active());

  lv_obj_clear_flag(tabview, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
  lv_tabview_set_tab_bar_size(tabview, 30);

  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Valve 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Valve 2");
  create_valve_tab(tab1, "Valve 1", slider_0, pin_22);
  create_valve_tab(tab2, "Valve 2", slider_1, pin_27);

  stats_tab = lv_tabview_add_tab(tabview, "Stats");
  update_stats_tab();

  lv_obj_t *override_btn = lv_btn_create(stats_tab);
  lv_obj_set_size(override_btn, 140, 40);
  lv_obj_align(override_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(
    override_btn, [](lv_event_t *e) {
      int pins[] = { 22, 27 };
      for (int i = 0; i < 2; i++) {
        open_valve(pins[i], slider_values[i]);
      }
      update_stats_tab();
    },
    LV_EVENT_CLICKED, NULL);
  lv_obj_t *override_label = lv_label_create(override_btn);
  lv_label_set_text(override_label, "WATER ALL NOW");
  lv_obj_center(override_label);


  lv_obj_add_event_cb(
    tabview, [](lv_event_t *e) {
      uint32_t tab_idx = lv_tabview_get_tab_act(tabview);
      // Assuming stats_tab is the 3rd tab (index 2)
      if (tab_idx == 2) {
        update_stats_tab();
      }
    },
    LV_EVENT_VALUE_CHANGED, NULL);
}

void setup() {
  Serial.begin(115200);
  int pins[] = { 22, 27 };
  for (int i = 0; i < 2; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }

  lv_init();
  lv_log_register_print_cb(log_print);
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);

  lv_display_t *disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  prefs.begin("water", true);
  for (int i = 0; i < 2; i++) {
    total_dispensed_ml[i] = prefs.getULong(("total" + String(i)).c_str(), 0);
    last_dispense[i] = 0;  // DO NOT persist last dispense anymore
    slider_values[i] = prefs.getInt(("ml" + String(i)).c_str(), 0);
    frequency_hours[i] = prefs.getInt(("freq" + String(i)).c_str(), 24);
  }
  prefs.end();

  lv_create_main_gui();
}

void loop() {
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);

  static unsigned long last_check = 0;
  if (millis() - last_check >= 60 * 60 * 1000) {
    last_check = millis();

    unsigned long now = millis();
    int pins[] = { 22, 27 };
    for (int i = 0; i < 2; i++) {
      if ((now - last_dispense[i]) >= frequency_hours[i] * 3600000UL) {
        open_valve(pins[i], slider_values[i]);
        last_dispense[i] = now;
        update_stats_tab();
      }
    }
  }
}
