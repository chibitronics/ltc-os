#ifndef __ORCHARD_ACCEL_H__
#define __ORCHARD_ACCEL_H__

struct accel_data {
  int x;
  int y;
  int z;
};

#define PULSE_AXIS_X 1
#define PULSE_AXIS_Y 2
#define PULSE_AXIS_Z 4

void accelStart(I2CDriver *driver);
void accelStop(void);
msg_t accelPoll(struct accel_data *data);
void accelEnableFreefall(int sensitivity, int debounce);
void accelDisableFreefall(void);

void accel_irq(eventid_t id);  // handler to register in main
void accel_test(eventid_t id); // test handler for main thread

extern event_source_t accel_pulse;
extern event_source_t accel_landscape_portrait;
extern event_source_t accel_freefall;
extern event_source_t accel_test_event;

extern uint8_t pulse_axis;

#endif /* __ORCHARD_ACCEL_H__ */
