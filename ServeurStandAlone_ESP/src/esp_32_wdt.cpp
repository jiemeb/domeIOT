#include "esp32_wdt.h"
#include <esp32-hal-timer.h>

static hw_timer_t *timer = NULL;

/**
 * iInterrupt service routine called when the timer expires.
 */
void IRAM_ATTR resetModule() {
  ets_printf("watchdog reboot\n");
  esp_restart();
}

void wdt_enable(const unsigned long durationMs) {
  //timer 0, div 80
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &resetModule, true);
  //set time in us
  timerAlarmWrite(timer, durationMs * 1000, false);
  //enable interrupt
  timerAlarmEnable(timer);
}

void wdt_disable() {
  if (timer != NULL) {
    //disable interrupt
    timerDetachInterrupt(timer);
    timerEnd(timer);
    timer = NULL;
  }
}

void wdt_reset() {
  //reset timer (feed watchdog)
  if (timer != NULL) {
    timerWrite(timer, 0);
  }
}
