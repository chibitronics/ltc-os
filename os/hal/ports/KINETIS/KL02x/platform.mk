# List of all platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x/pal_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x/serial_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x/spi_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/LLD/i2c_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/LLD/ext_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/LLD/adc_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x/pwm_lld.c \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x/st_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/common/ARMCMx \
              ${CHIBIOS}/os/hal/ports/KINETIS/KL02x \
              ${CHIBIOS}/os/hal/ports/KINETIS/LLD
