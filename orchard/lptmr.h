#ifndef __LPTMR_H__
#define __LPTMR_H__

void startLptmr(ioportid_t port, uint8_t pad, uint32_t rate_hz);
void stopLptmr(void);

#endif /* __LPTMR_H__ */
