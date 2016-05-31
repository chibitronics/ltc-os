#define SCOPE_SAMPLE_DEPTH  128

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);
uint8_t *analogReadMic(void);

void analogStart(void);
uint32_t analogRead(int adc_num);
uint16_t *scopeRead(int adc_num, int speed_mode);
