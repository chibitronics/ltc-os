#include "hal.h"
#include "i2c.h"
#include "orchard.h"

static const I2CConfig i2c_config = {
  100000
};

static uint8_t i2c_started;

msg_t i2cTransceive(i2caddr_t addr,
                    const uint8_t *txbuf, size_t txbytes,
                    uint8_t *rxbuf, size_t rxbytes,
                    systime_t timeout) {
  msg_t ret;

  /* Only start I2C if we're using it.  If we're in this function,
   * then obviously we're using it.
   */
  if (!i2c_started) {
    i2cStart(i2cDriver, &i2c_config);
    i2c_started = 1;
  }

  i2cAcquireBus(i2cDriver);
  ret = i2cMasterTransmitTimeout(i2cDriver, addr, txbuf, txbytes, rxbuf, rxbytes, timeout);
  i2cReleaseBus(i2cDriver);
  return ret;
}

int i2cSetupSlave(void) {

  return -1;
}