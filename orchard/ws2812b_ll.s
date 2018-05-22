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

eclr    .req r4  // register for clearing a bit
eset    .req r3  // register for setting a bit
bitmask .req r2  // mask to jam into register

curpix  .req r5  // current pixel value
stop    .req r0  // stop value for fbptr
bit     .req r6  // current bit number (shift loop per color)
fbptr   .req r1  // frame buffer pointer

// Each pixel is 1.25 uS.  At our clock rate, that's
// 71-72 cycles for pixloop.

.func ledUpdate
ledUpdate:
	// r0  uint8_t *fb
	// r1  uint32_t	len
	push {r4-r7,lr}     // Save the other parameters we'll use
	ldr eclr, [sp, #20] // Get eclr from the stack, as argument #5

	// Figure out what address of fbptr is the last, so we know when to quit.
	mov bit, #3         // use bit temp for multiply
	mul stop, bit       // mult by 3 for RGB
	add stop, fbptr     // add in the fptr to finalize stop computation value

	// Load a brand-new pixel from the framebuffer pointer
load_next_pixel:
	str bitmask, [eset]     // start with bit set
	mov bit, #8             // Start processing bit #8 of current pixel
	ldrb curpix, [fbptr]    // load the word at the pointer location to r4
	lsl curpix, curpix, #24 // shift left by 24 so the carry pops out
	add fbptr, fbptr, #1    // Move to the next pixel
	b pixloop_fromtop // 10 total

	// While processing the same pixel, send the next bit
send_next_bit:
	str bitmask, [eset]   // start with bit set
	bl wait_5_cycles
	ldr eclr, [sp, #20]   // Burn two cycles in one opcode
	// fall-through

// Both new pixels and new bits (i.e. load_next_pixel and send_next_bit)
// should now be synchronized.
pixloop_fromtop:
	// 62 cycles remaining
	bl wait_5_cycles
	lsl curpix, #1
	bcs oneBranch

zeroBranch:
	str bitmask, [eclr]

oneBranch:

	bl wait_15_cycles
	bl wait_15_cycles
	bl wait_15_cycles
	str bitmask, [eclr]

	bl wait_15_cycles
	sub bit, bit, #1
	bne send_next_bit

	cmp fbptr, stop
	bne load_next_pixel

	b exit

exit:
	pop {r4-r7,pc}

wait_15_cycles:
	b wait_12_cycles
wait_12_cycles:
	b wait_9_cycles
wait_9_cycles:
	b wait_5_cycles
wait_5_cycles:
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
