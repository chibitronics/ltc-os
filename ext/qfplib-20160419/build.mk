ASMSRC += \
          $(QFPLIB)/qfpio.s \
          $(QFPLIB)/qfplib.s \
          $(QFPLIB)/qfpcompat.s \

QFPLIBINC += $(QFPLIB)
USE_COPT += -I$(QFPLIBINC)
