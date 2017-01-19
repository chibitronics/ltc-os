.text

#include "kl02.h"

  /***
    WS2812B Low-Level code
      WS2812B timing requirements:
      To transmit a "0": 400ns high, 850ns low  +/- 150ns  total = 1250ns
	19 cycles H / 41 cycles L
      To transmit a "1": 800ns high, 450ns low  +/- 150ns  total = 1250ns
	38 cycles H / 22 cycles L
      Clock balance to within +/-150ns = 7 cycles

      Reset code is 50,000ns of low

      Each instruction takes 20.8ns, 1250/20.8 = 60 instructions per cycle total
      
      Data transmit to chain is {G[7:0],R[7:0],B[7:0]}

      Data passed in shall be an array of bytes, GRB ordered, of length N specified as a parameter

      extern void ledUpdate(uint32_t num_leds, void *pixels, uint32_t mask,
                            uint32_t set_addr, uint32_t clr_addr);

   ***/

.global ledUpdate

.equ  mainled_bit0, 0x0
.equ  mainled_bit1, 0x1

eclr    .req r4  // register for clearing a bit
eset    .req r3  // register for setting a bit
bitmask .req r2  // mask to jam into register

curpix  .req r5  // current pixel value
stop    .req r0  // stop value for fbptr
bit     .req r6  // bit counter (shift loop per color)
fbptr   .req r1  // frame buffer pointer

	// r2 is GPIO dest location
	// r0 is "0" bit number code
	// r1 is "1" bit number code
	// r3 is loop counter for total number of pixels
	// r4 is the current pixel value
	// r5 is the test value for the top bit of current pixel
	// r6 is the loop counter for bit number in a pixel
	// r7 is current pointer to the pixel array

.func ledUpdate
ledUpdate:
	// r0  uint8_t *fb
	// r1  uint32_t	len
	push {r4-r7,lr}     // Save the other parameters we'll use
  ldr eclr, [sp, #20] // Get eclr from the stack, as argument #5
	mov bit, #3         // use bit temp for multiply
	mul stop, bit       // mult by 3 for RGB
	add stop, fbptr     // add in the fptr to finalize stop computation value

	///////////////////////
looptop:
	str bitmask, [eset]   // start with bit set
	mov bit, #8
	ldrb curpix, [fbptr]      // load the word at the pointer location to r4
	lsl curpix, curpix, #24  // shift left by 24 so the carry pops out
	add fbptr, fbptr, #1
	// above is 6 cycles, plus two cycles from hitting the end of a pixel
	b pixloop_fromtop // 10 total

// Each pixel is 1.25 uS.  At our clock rate, that's
// 71-72 cycles for pixloop.
pixloop:
	// Add in the two cycles from the jump to looptop,
	// to make both branches equal length.
	nop
	nop
	str bitmask, [eset]   // start with bit set
	bl wait_6_cycles
	nop

pixloop_fromtop:
	// 62 cycles remaining
	lsl curpix, #1
	bcs oneBranch

zeroBranch:
	bl wait_6_cycles
	nop
	str bitmask, [eclr]
	bl wait_15_cycles
	bl wait_15_cycles
	b pixEpilogue

oneBranch:
	// 59 cycles to burn
	// one path -- so far, 14 cycles high

	bl wait_15_cycles
	bl wait_18_cycles
	str bitmask, [eclr]

	b pixEpilogue

pixEpilogue:
	bl wait_6_cycles
	sub bit, bit, #1
	bne pixloop

	cmp fbptr, stop
	bne looptop

	b exit
	
exit:
	pop {r4-r7,pc}

wait_18_cycles:
	b wait_15_cycles
wait_15_cycles:
	b wait_12_cycles
wait_12_cycles:
	b wait_9_cycles
wait_9_cycles:
	b wait_6_cycles
wait_6_cycles:
	bx lr

.endfunc
.type ledUpdate, %function
.size ledUpdate, .-ledUpdate

.balign 4
PORTESET:	
.word FGPIOA_PSOR
PORTECLR:	
.word FGPIOA_PCOR
LEDBIT:	
.word (1<<3)

.end
	
