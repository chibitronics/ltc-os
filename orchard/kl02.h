#ifndef __KL02_H__
#define __KL02_H__

#include <stdint.h>

/* Base peripheral offsets */
#define FLASH_BASE              ((uint32_t)0x40020000)
#define TPM0_BASE               ((uint32_t)0x40038000)
#define TPM1_BASE               ((uint32_t)0x40039000)
#define ADC0_BASE               ((uint32_t)0x4003B000)
#define LPTMR0_BASE             ((uint32_t)0x40040000)
#define SIM_BASE                ((uint32_t)0x40047000)
#define PORTA_BASE              ((uint32_t)0x40049000)
#define PORTB_BASE              ((uint32_t)0x4004A000)
#define MCG_BASE                ((uint32_t)0x40064000)
#define OSC0_BASE               ((uint32_t)0x40065000)
#define I2C0_BASE               ((uint32_t)0x40066000)
#define I2C1_BASE               ((uint32_t)0x40067000)
#define UART0_BASE              ((uint32_t)0x4006A000)
#define DAC0_BASE               ((uint32_t)0x40073000)
#define SPI0_BASE               ((uint32_t)0x40076000)
#define PMC_BASE                ((uint32_t)0x4007D000)
#define SMC_BASE                ((uint32_t)0x4007E000)
#define RCM_BASE                ((uint32_t)0x4007F000)
#define GPIOA_BASE              ((uint32_t)0x400FF000)
#define GPIOB_BASE              ((uint32_t)0x400FF040)
#define FGPIOA_BASE             ((uint32_t)0xF8000000)
#define FGPIOB_BASE             ((uint32_t)0xF8000040)

/* System Integration Module */
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
#define GPIOA_PDOR 0x400FF000 /* Port Data Output Register */
#define GPIOA_PSOR 0x400FF004 /* Port Set Output Register */
#define GPIOA_PCOR 0x400FF008 /* Port Clear Output Register */
#define GPIOA_PTOR 0x400FF00C /* Port Toggle Output Register */
#define GPIOA_PDIR 0x400FF010 /* Port Data Input Register */
#define GPIOA_PDDR 0x400FF014 /* Port Data Direction Register */
#define GPIOB_PDOR 0x400FF040 /* Port Data Output Register */
#define GPIOB_PSOR 0x400FF044 /* Port Set Output Register */
#define GPIOB_PCOR 0x400FF048 /* Port Clear Output Register */
#define GPIOB_PTOR 0x400FF04C /* Port Toggle Output Register */
#define GPIOB_PDIR 0x400FF050 /* Port Data Input Register */
#define GPIOB_PDDR 0x400FF054 /* Port Data Direction Register */

/* FGPIO */
#define FGPIOA_PDOR 0xF8000000 /* Port Data Output Register */
#define FGPIOA_PSOR 0xF8000004 /* Port Set Output Register */
#define FGPIOA_PCOR 0xF8000008 /* Port Clear Output Register */
#define FGPIOA_PTOR 0xF800000C /* Port Toggle Output Register */
#define FGPIOA_PDIR 0xF8000010 /* Port Data Input Register */
#define FGPIOA_PDDR 0xF8000014 /* Port Data Direction Register */
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

#endif /* __KL02_H__ */
