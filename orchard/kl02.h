#ifndef __KL02_H__
#define __KL02_H__

/* Base peripheral offsets */
#if !defined(FLASH_BASE) /* Don't re-define these from another implementation */
#define FLASH_BASE              ((unsigned long)0x40020000)
#endif

#if !defined(TPM0_BASE) /* Don't re-define these from another implementation */
#define TPM0_BASE               ((unsigned long)0x40038000)
#endif

#if !defined(TPM1_BASE) /* Don't re-define these from another implementation */
#define TPM1_BASE               ((unsigned long)0x40039000)
#endif

#if !defined(ADC0_BASE) /* Don't re-define these from another implementation */
#define ADC0_BASE               ((unsigned long)0x4003B000)
#endif

#if !defined(OSC0_BASE) /* Don't re-define these from another implementation */
#define OSC0_BASE               ((unsigned long)0x40065000)
#endif

#if !defined(I2C0_BASE) /* Don't re-define these from another implementation */
#define I2C0_BASE               ((unsigned long)0x40066000)
#endif
#define I2C0_A1                 0x40066000
#define I2C0_F                  0x40066001
#define I2C0_C1                 0x40066002
#define I2C0_S                  0x40066003
#define I2C0_D                  0x40066004
#define I2C0_C2                 0x40066005
#define I2C0_FLT                0x40066006
#define I2C0_RA                 0x40066007

#if !defined(I2C1_BASE) /* Don't re-define these from another implementation */
#define I2C1_BASE               ((unsigned long)0x40067000)
#endif
#define I2C1_A1                 0x40067000
#define I2C1_F                  0x40067001
#define I2C1_C1                 0x40067002
#define I2C1_S                  0x40067003
#define I2C1_D                  0x40067004
#define I2C1_C2                 0x40067005
#define I2C1_FLT                0x40067006
#define I2C1_RA                 0x40067007

#if !defined(UART0_BASE) /* Don't re-define these from another implementation */
#define UART0_BASE              ((unsigned long)0x4006A000)
#endif

#if !defined(DAC0_BASE) /* Don't re-define these from another implementation */
#define DAC0_BASE               ((unsigned long)0x40073000)
#endif

#if !defined(SPI0_BASE) /* Don't re-define these from another implementation */
#define SPI0_BASE               ((unsigned long)0x40076000)
#endif
#define SPI0_C1                 0x40076000
#define SPI0_C2                 0x40076001
#define SPI0_BR                 0x40076002
#define SPI0_S                  0x40076003
#define SPI0_D                  0x40076005
#define SPI0_M                  0x40076007

#if !defined(PMC_BASE) /* Don't re-define these from another implementation */
#define PMC_BASE                ((unsigned long)0x4007D000)
#endif
#define PMC_LVDSC1              0x4007d000
/*
#define PMC_LVDSC1_LVDF         (1 << 7)
#define PMC_LVDSC1_LVDACK       (1 << 6)
#define PMC_LVDSC1_LVDIE        (1 << 5)
#define PMC_LVDSC1_LVDRE        (1 << 4)
#define PMC_LVDSC1_LVDV         (1 << 0)
*/
#define PMC_LVDSC2              0x4007d001
/*
#define PMC_LVDSC2_LVWF         (1 << 7)
#define PMC_LVDSC2_LVWACK       (1 << 6)
#define PMC_LVDSC2_LVWIE        (1 << 5)
#define PMC_LVDSC2_LVWV         (1 << 0)
*/
#define PMC_REGSC               0x4007d002
/*
#define PMC_REGSC_BGEN          (1 << 4)
#define PMC_REGSC_ACKISO        (1 << 3)
#define PMC_REGSC_REGONS        (1 << 2)
#define PMC_REGSC_BGBE          (1 << 0)
*/

#if !defined(SMC_BASE) /* Don't re-define these from another implementation */
#define SMC_BASE                ((unsigned long)0x4007E000)
#endif

#if !defined(RCM_BASE) /* Don't re-define these from another implementation */
#define RCM_BASE                ((unsigned long)0x4007F000)
#endif

/* Multipurpose Clock Generator */
#if !defined(MCG_BASE) /* Don't re-define these from another implementation */
#define MCG_BASE                ((unsigned long)0x40064000)
#endif
#define MCG_C1 0x40064000
#define MCG_C2 0x40064001
#define MCG_C3 0x40064002
#define MCG_C4 0x40064003
#define MCG_C6 0x40064005
#define MCG_S 0x40064006
#define MCG_SC 0x40064008
/*
#define MCG_SC_ATME             (1 << 7)
#define MCG_SC_ATMS             (1 << 6)
#define MCG_SC_ATMF             (1 << 5)
#define MCG_SC_FLTPRSRV         (1 << 4)
#define MCG_SC_FCRDIV           (1 << 1)
#define MCF_SC_LOCS0            (1 << 0)
*/
#define MCG_ATCVH 0x4006400A
#define MCG_ATCVL 0x4006400B

/* System Integration Module */
#if !defined(SIM_BASE)
#define SIM_BASE                ((unsigned long)0x40047000)
#endif /* !defined(SIM_BASE) */
#define SIM_SOPT1   0x40047000 /* System Options Register 1 */
#define SIM_SOPT2   0x40048004 /* System Options Register 2 */
#define SIM_SOPT4   0x4004800C /* System Options Register 4 */
#define SIM_SOPT5   0x40048010 /* System Options Register 5 */
#define SIM_SOPT7   0x40048018 /* System Options Register 7 */
#define SIM_SDID    0x40048024 /* System Device Identification Register */
#define SIM_SCGC4   0x40048034 /* System Clock Gating Control Register 4 */
#define SIM_SCGC5   0x40048038 /* System Clock Gating Control Register 5 */
#define SIM_SCGC6   0x4004803C /* System Clock Gating Control Register 6 */
#define SIM_CLKDIV1 0x40048044 /* System Clock Divider Register 1 */
#define SIM_FCFG1   0x4004804C /* Flash Configuration Register 1 */
#define SIM_FCFG2   0x40048050 /* Flash Configuration Register 2 */
#define SIM_UIDMH   0x40048058 /* Unique Identification Register Mid-High */
#define SIM_UIDML   0x4004805C /* Unique Identification Register Mid Low */
#define SIM_UIDL    0x40048060 /* Unique Identification Register Low */
#define SIM_COPC    0x40048100 /* COP Control Register */
#define SIM_SRVCOP  0x40048104 /* Service COP Register */

/* Pin mux offsets */
#if !defined(PORTA_BASE) /* Don't re-define these from another implementation */
#define PORTA_BASE              ((unsigned long)0x40049000)
#endif
#define PORTA_PCR(x) (PORTA_PCR0 + (x * 4))
#define PORTA_PCR0 0x40049000 /* Pin Control Register n */
#define PORTA_PCR1 0x40049004 /* Pin Control Register n */
#define PORTA_PCR2 0x40049008 /* Pin Control Register n */
#define PORTA_PCR3 0x4004900C /* Pin Control Register n */
#define PORTA_PCR4 0x40049010 /* Pin Control Register n */
#define PORTA_PCR5 0x40049014 /* Pin Control Register n */
#define PORTA_PCR6 0x40049018 /* Pin Control Register n */
#define PORTA_PCR7 0x4004901C /* Pin Control Register n */
#define PORTA_PCR8 0x40049020 /* Pin Control Register n */
#define PORTA_PCR9 0x40049024 /* Pin Control Register n */
#define PORTA_PCR10 0x40049028 /* Pin Control Register n */
#define PORTA_PCR11 0x4004902C /* Pin Control Register n */
#define PORTA_PCR12 0x40049030 /* Pin Control Register n */
#define PORTA_PCR13 0x40049034 /* Pin Control Register n */
#define PORTA_PCR14 0x40049038 /* Pin Control Register n */
#define PORTA_PCR15 0x4004903C /* Pin Control Register n */
#define PORTA_PCR16 0x40049040 /* Pin Control Register n */
#define PORTA_PCR17 0x40049044 /* Pin Control Register n */
#define PORTA_PCR18 0x40049048 /* Pin Control Register n */
#define PORTA_PCR19 0x4004904C /* Pin Control Register n */
#define PORTA_PCR20 0x40049050 /* Pin Control Register n */
#define PORTA_PCR21 0x40049054 /* Pin Control Register n */
#define PORTA_PCR22 0x40049058 /* Pin Control Register n */
#define PORTA_PCR23 0x4004905C /* Pin Control Register n */
#define PORTA_PCR24 0x40049060 /* Pin Control Register n */
#define PORTA_PCR25 0x40049064 /* Pin Control Register n */
#define PORTA_PCR26 0x40049068 /* Pin Control Register n */
#define PORTA_PCR27 0x4004906C /* Pin Control Register n */
#define PORTA_PCR28 0x40049070 /* Pin Control Register n */
#define PORTA_PCR29 0x40049074 /* Pin Control Register n */
#define PORTA_PCR30 0x40049078 /* Pin Control Register n */
#define PORTA_PCR31 0x4004907C /* Pin Control Register n */
#define PORTA_GPCLR 0x40049080 /* Global Pin Control Low Register */
#define PORTA_GPCHR 0x40049084 /* Global Pin Control High Register */
#define PORTA_ISFR 0x400490A0 /* Interrupt Status Flag Register */
#if !defined(PORTB_BASE) /* Don't re-define these from another implementation */
#define PORTB_BASE              ((unsigned long)0x4004A000)
#endif
#define PORTB_PCR(x) (PORTB_PCR0 + (x * 4))
#define PORTB_PCR0 0x4004A000 /* Pin Control Register n */
#define PORTB_PCR1 0x4004A004 /* Pin Control Register n */
#define PORTB_PCR2 0x4004A008 /* Pin Control Register n */
#define PORTB_PCR3 0x4004A00C /* Pin Control Register n */
#define PORTB_PCR4 0x4004A010 /* Pin Control Register n */
#define PORTB_PCR5 0x4004A014 /* Pin Control Register n */
#define PORTB_PCR6 0x4004A018 /* Pin Control Register n */
#define PORTB_PCR7 0x4004A01C /* Pin Control Register n */
#define PORTB_PCR8 0x4004A020 /* Pin Control Register n */
#define PORTB_PCR9 0x4004A024 /* Pin Control Register n */
#define PORTB_PCR10 0x4004A028 /* Pin Control Register n */
#define PORTB_PCR11 0x4004A02C /* Pin Control Register n */
#define PORTB_PCR12 0x4004A030 /* Pin Control Register n */
#define PORTB_PCR13 0x4004A034 /* Pin Control Register n */
#define PORTB_PCR14 0x4004A038 /* Pin Control Register n */
#define PORTB_PCR15 0x4004A03C /* Pin Control Register n */
#define PORTB_PCR16 0x4004A040 /* Pin Control Register n */
#define PORTB_PCR17 0x4004A044 /* Pin Control Register n */
#define PORTB_PCR18 0x4004A048 /* Pin Control Register n */
#define PORTB_PCR19 0x4004A04C /* Pin Control Register n */
#define PORTB_PCR20 0x4004A050 /* Pin Control Register n */
#define PORTB_PCR21 0x4004A054 /* Pin Control Register n */
#define PORTB_PCR22 0x4004A058 /* Pin Control Register n */
#define PORTB_PCR23 0x4004A05C /* Pin Control Register n */
#define PORTB_PCR24 0x4004A060 /* Pin Control Register n */
#define PORTB_PCR25 0x4004A064 /* Pin Control Register n */
#define PORTB_PCR26 0x4004A068 /* Pin Control Register n */
#define PORTB_PCR27 0x4004A06C /* Pin Control Register n */
#define PORTB_PCR28 0x4004A070 /* Pin Control Register n */
#define PORTB_PCR29 0x4004A074 /* Pin Control Register n */
#define PORTB_PCR30 0x4004A078 /* Pin Control Register n */
#define PORTB_PCR31 0x4004A07C /* Pin Control Register n */
#define PORTB_GPCLR 0x4004A080 /* Global Pin Control Low Register */
#define PORTB_GPCHR 0x4004A084 /* Global Pin Control High Register */
#define PORTB_ISFR 0x4004A0A0 /* Interrupt Status Flag Register */

/* GPIO */
#define GPIO_PDOR  0x00       /* Port Data Output Register offset */
#define GPIO_PSOR  0x04       /* Port Set Output Register offset */
#define GPIO_PCOR  0x08       /* Port Clear Output Register offset */
#define GPIO_PTOR  0x0C       /* Port Toggle Output Register offset */
#define GPIO_PDIR  0x10       /* Port Data Input Register offset */
#define GPIO_PDDR  0x14       /* Port Data Direction Register offset */

#if !defined(GPIOA_BASE) /* Don't re-define these from another implementation */
#define GPIOA_BASE              ((unsigned long)0x400FF000)
#endif
#define GPIOA_PDOR 0x400FF000 /* Port Data Output Register */
#define GPIOA_PSOR 0x400FF004 /* Port Set Output Register */
#define GPIOA_PCOR 0x400FF008 /* Port Clear Output Register */
#define GPIOA_PTOR 0x400FF00C /* Port Toggle Output Register */
#define GPIOA_PDIR 0x400FF010 /* Port Data Input Register */
#define GPIOA_PDDR 0x400FF014 /* Port Data Direction Register */

#if !defined(GPIOB_BASE) /* Don't re-define these from another implementation */
#define GPIOB_BASE              ((unsigned long)0x400FF040)
#endif
#define GPIOB_PDOR 0x400FF040 /* Port Data Output Register */
#define GPIOB_PSOR 0x400FF044 /* Port Set Output Register */
#define GPIOB_PCOR 0x400FF048 /* Port Clear Output Register */
#define GPIOB_PTOR 0x400FF04C /* Port Toggle Output Register */
#define GPIOB_PDIR 0x400FF050 /* Port Data Input Register */
#define GPIOB_PDDR 0x400FF054 /* Port Data Direction Register */

/* FGPIO */
#if !defined(FGPIOA_BASE) /* Don't re-define these from another implementation */
#define FGPIOA_BASE             ((unsigned long)0xF8000000)
#endif
#define FGPIOA_PDOR 0xF8000000 /* Port Data Output Register */
#define FGPIOA_PSOR 0xF8000004 /* Port Set Output Register */
#define FGPIOA_PCOR 0xF8000008 /* Port Clear Output Register */
#define FGPIOA_PTOR 0xF800000C /* Port Toggle Output Register */
#define FGPIOA_PDIR 0xF8000010 /* Port Data Input Register */
#define FGPIOA_PDDR 0xF8000014 /* Port Data Direction Register */

#if !defined(FGPIOB_BASE) /* Don't re-define these from another implementation */
#define FGPIOB_BASE             ((unsigned long)0xF8000040)
#endif
#define FGPIOB_PDOR 0xF8000040 /* Port Data Output Register */
#define FGPIOB_PSOR 0xF8000044 /* Port Set Output Register */
#define FGPIOB_PCOR 0xF8000048 /* Port Clear Output Register */
#define FGPIOB_PTOR 0xF800004C /* Port Toggle Output Register */
#define FGPIOB_PDIR 0xF8000050 /* Port Data Input Register */
#define FGPIOB_PDDR 0xF8000054 /* Port Data Direction Register */

/* TPM */
#define TPM0_SC 0x40038000 /* Status and Control */
#define TPM0_CNT 0x40038004 /* Counter */
#define TPM0_MOD 0x40038008 /* Modulo */
#define TPM0_C0SC 0x4003800C /* Channel Status and Control */
#define TPM0_C0V 0x40038010 /* Channel Value */
#define TPM0_C1SC 0x40038014 /* Channel Status and Control */
#define TPM0_C1V 0x40038018 /* Channel Value */
#define TPM0_STATUS 0x40038050 /* Capture and Compare Status */
#define TPM0_CONF 0x40038084 /* Configuration */
#define TPM1_SC 0x40039000 /* Status and Control */
#define TPM1_CNT 0x40039004 /* Counter */
#define TPM1_MOD 0x40039008 /* Modulo */
#define TPM1_C0SC 0x4003900C /* Channel Status and Control */
#define TPM1_C0V 0x40039010 /* Channel Value */
#define TPM1_C1SC 0x40039014 /* Channel Status and Control */
#define TPM1_C1V 0x40039018 /* Channel Value */
#define TPM1_STATUS 0x40039050 /* Capture and Compare Status */
#define TPM1_CONF 0x40039084 /* Configuration */

/* Flash Translation (Layer|Engine) */
#define FMC_PFB01CR 0x4001f004
#define FTFx_FSTAT  0x40020000
#define FTFx_FCNFG  0x40020001
#define FTFx_FSEC   0x40020002
#define FTFx_FOPT   0x40020003
#define FTFx_FCCOB3 0x40020004
#define FTFx_FCCOB2 0x40020005
#define FTFx_FCCOB1 0x40020006
#define FTFx_FCCOB0 0x40020007
#define FTFx_FCCOB7 0x40020008
#define FTFx_FCCOB6 0x40020009
#define FTFx_FCCOB5 0x4002000A
#define FTFx_FCCOB4 0x4002000B
#define FTFx_FCCOBB 0x4002000C
#define FTFx_FCCOBA 0x4002000D
#define FTFx_FCCOB9 0x4002000E
#define FTFx_FCCOB8 0x4002000F
#define FTFx_FPROT3 0x40020010
#define FTFx_FDPROT 0x40020017

/* System Management Controller */
#define SMC_PMCTRL  0x4007E001
#define SMC_PMSTAT  0x4007E003

/* Watchdog */
#define WDOG_STCTRH 0x40052000

/* Low power timer */
#if !defined(LPTMR0_BASE) /* Don't re-define these from another implementation */
#define LPTMR0_BASE             ((unsigned long)0x40040000)
#endif
#define LPTMR0_CSR  0x40040000
#define LPTMR_CSR_TCF (1 << 7)
#define LPTMR_CSR_TIE (1 << 6)
#define LPTMR_CSR_TPS (1 << 4)
#define LPTMR_CSR_TPP (1 << 3)
#define LPTMR_CSR_TFC (1 << 2)
#define LPTMR_CSR_TMS (1 << 1)
#define LPTMR_CSR_TEN (1 << 0)
#define LPTMR0_PSR  0x40040004
#define LPTMR_PSR_PRESCALE (1 << 3)
#define LPTMR_PSR_PBYP (1 << 2)
#define LPTMR_PSR_PCS (1 << 0)
#define LPTMR_PSR_PCS_CLK0 (0 << 0)
#define LPTMR_PSR_PCS_CLK1 (1 << 0)
#define LPTMR_PSR_PCS_CLK2 (2 << 0)
#define LPTMR_PSR_PCS_CLK3 (3 << 0)
#define LPTMR_PSR_PCS_MCGIRCLK (0 << 0)
#define LPTMR_PSR_PCS_LPO (1 << 0)
#define LPTMR_PSR_PCS_ERCLK32K (2 << 0)
#define LPTMR_PSR_PCS_OSCERCLK (3 << 0)
#define LPTMR0_CMR  0x40040008
#define LPTMR0_CNR  0x4004000C

#endif /* __KL02_H__ */
