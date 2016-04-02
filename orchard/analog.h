#define MIC_SAMPLE_DEPTH  40
#define DMBUF_DEPTH 540

extern uint16_t dm_buf[];
extern uint32_t dm_buf_ptr;

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);

void analogStart(void);
