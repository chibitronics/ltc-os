#define DEMOD_DEBUG 1

#define NB_SAMPLES 8   // number of samples per demod frame
#define NB_FRAMES  32   // number of frames to process at once (trade memory off against OS overhead)

#define BAUD_RATE 9600
#define SAMPLE_RATE 75000  // of the rx side
#define SAMPLE_RATE_TX 44100

#define COS_BITS   14
#define COS_BASE   (1 << COS_BITS)

#define F_LO   10400
#define F_HI   15000

#define FSK_FILTER_SIZE (SAMPLE_RATE / BAUD_RATE)  // <-- this can be 8 or so with optimal bauds
#define FSK_FILTER_BUF_SIZE (FSK_FILTER_SIZE * 2)

#define DMBUF_DEPTH (NB_SAMPLES * NB_FRAMES) 
extern int16_t dm_buf[];

/* bit I/O for data pumps */
typedef void (*put_bit_func)(int bit);

typedef struct {
    int16_t filter_buf[FSK_FILTER_BUF_SIZE];
    uint8_t buf_ptr;

    int32_t baud_incr;
    int32_t baud_pll, baud_pll_adj, baud_pll_threshold;
    int16_t lastsample;
    int16_t shift;
} FSK_demod_state;

typedef struct {
    /* parameters */
    uint32_t f_lo,f_hi;
    uint32_t sample_rate;
    uint32_t baud_rate;

    /* local variables */
    int16_t filter_size;
    int16_t filter_lo_i[FSK_FILTER_SIZE];
    int16_t filter_lo_q[FSK_FILTER_SIZE];
    
    int16_t filter_hi_i[FSK_FILTER_SIZE];
    int16_t filter_hi_q[FSK_FILTER_SIZE];
} FSK_demod_const;

extern const FSK_demod_const fsk_const;

static inline int dsp_dot_prod(int16_t *tab1, const int16_t *tab2, 
                               int16_t n, int sum)
{
    int i;

    for(i=0;i<n;i++) {
        sum += tab1[i] * tab2[i];
    }

    return sum;
}

void demodInit(void);
void FSKdemod(int16_t *samples, uint32_t nb, put_bit_func put_bit);


#define LOG_DEPTH  (256+4+2+1)  // 256 byte paylod, 4 byte hash, 2 byte sector offset, 1 byte version code
extern volatile uint16_t log_ptr;
extern uint8_t  log_buf[LOG_DEPTH];
