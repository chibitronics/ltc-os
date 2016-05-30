#include "ch.h"
#include "hal.h"
#include "oled.h"
#include "spi.h"

#include "orchard.h"

#include "gfx.h"

static SPIDriver *driver;

// mutex to lock the graphics subsystem for safe multi-threaded drawing
mutex_t orchard_gfxMutex;

void orchardGfxInit(void) {
  osalMutexObjectInit(&orchard_gfxMutex);
  gfxInit();
}

void orchardGfxStart(void) {
  osalMutexLock(&orchard_gfxMutex);
}

void orchardGfxEnd(void) {
  osalMutexUnlock(&orchard_gfxMutex);
}

static void oled_command_mode(void) {
  palClearPad(GPIOB, 13);
}

static void oled_data_mode(void) {
  palSetPad(GPIOB, 13);
}

static void oled_select(void) {
  palClearPad(GPIOA, 5);
}

static void oled_unselect(void) {
  palSetPad(GPIOA, 5);
}

void oledStop(SPIDriver *spip) {
  (void)spip;
  
  oledAcquireBus();
  oledCmd(0xAE); // display off
  oledReleaseBus();

  oledAcquireBus();
  oledCmd(0x8D); // disable charge pump
  oledCmd(0x10);
  oledReleaseBus();

  // or just yank the reset line low?
  // gpioxSetPadMode(GPIOX, oledResPad, GPIOX_OUT_PUSHPULL | GPIOX_VAL_LOW);
  
  // wait 100ms per datasheet
  chThdSleepMilliseconds(100);
}

void oledStart(SPIDriver *spip) {

  driver = spip;

  palWritePad(GPIOB, 11, PAL_HIGH);  // oled_res
}

/* Exported functions, called by uGFX.*/
void oledCmd(uint8_t cmd) {

  oled_command_mode();
  spiSend(driver, 1, &cmd);
}

void oledData(uint8_t *data, uint16_t length) {
  unsigned int i;

  oled_data_mode();
  for( i = 0; i < length; i++ )
    spiSend(driver, 1, &data[i]);
}

void oledAcquireBus(void) {
  spiAcquireBus(driver);
  oled_select();
}

void oledReleaseBus(void) {
  oled_unselect();
  spiReleaseBus(driver);
}

void oledOrchardBanner(void) {
  coord_t width;
  coord_t height;
  font_t font;
  
  orchardGfxStart();
  width = gdispGetWidth();
  font = gdispOpenFont("UI2");
  height = gdispGetFontMetric(font, fontHeight);
  
  gdispClear(Black);
  gdispDrawStringBox(0, height * 2, width, gdispGetFontMetric(font, fontHeight),
                     "Chibi Dataviewer", font, White, justifyCenter);
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

