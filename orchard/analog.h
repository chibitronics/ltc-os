#define MIC_SAMPLE_DEPTH  (DMBUF_DEPTH * 2)  // have to double-buffer around demodulation buffer depth

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);

void analogStart(void);
