#ifndef _FLASH_H_
#define _FLASH_H_

#define P_FLASH_BASE              0x00000000
#define P_FLASH_SIZE              32768

#define READ_NORMAL_MARGIN        0x00
#define READ_USER_MARGIN          0x01
#define READ_FACTORY_MARGIN       0x02

#define FTFx_PSECTOR_SIZE         (1024)

#define F_USER_SECTOR_START ((uint32_t)__storage_start__ / FTFx_PSECTOR_SIZE) 

/************************************************************/
/* prototypes                                               */
/************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void flashStart(void);
uint32_t flashGetSecurity(void);
int8_t flashEraseSectors(uint32_t sectorOffset, uint16_t sectorCount);
int8_t flashProgram(uint8_t *src, uint8_t *dst, uint32_t count);

#ifdef __cplusplus
};
#endif

// define limit for user-accessible sector
// we assume we have flash from the user sector all the way to the end of Flash
// __storage_start__ comes from the linker

// chibios error codes -- they make more sense to me than Freescale's set of error codes
#define F_ERR_OK  0
#define F_ERR_NOTBLANK  -1   // attempting to program a programmed section
#define F_ERR_NOTALIGN  -2   // attempting to program a block of data that's not 4-byte aligned
#define F_ERR_LOWLEVEL  -3   // low level error (e.g. margin fail, etc.)
#define F_ERR_RANGE     -4   // attempting to manipulate a block outside the user range
#define F_ERR_U_MARGIN  -6   // failed verification at user margin levels -- worry a little
#define F_ERR_N_MARGIN  -7   // failed verification at normal margin levels -- worry a lot
#define F_ERR_F_MARGIN  -8   // failed verification at factory margin levels -- don't worry too much
#define F_ERR_GENERAL   -9   // general error code
#define F_ERR_JOURNAL_OVER   -10   // journalingy overflow -- we don't handle it

#endif /* _FLASH_H_ */
