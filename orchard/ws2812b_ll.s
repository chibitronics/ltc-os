.text

#include "kl02.h"

  /***
    WS2812B Low-Level code

    WS2812B timing requirements:
    To transmit a "0": 220ns-380ns high, 580ns-1.6us low
    To transmit a "1": 580ns-1.6us high, 220ns-420ns low

    Reset code is 280,000ns of low

    The Chibi Chip has a 32768 Hz crystal, and a multiplier of 1464, giving
    a clock rate of 47972352 Hz (47.972352 MHz).

    1000000000.0 ns / 47972352 MHz = 20.84534024931694 ns per cycle.  Most
    instructions are single-cycle, with the exception of loads/stores from RAM
    and branches taken.  This forms the basis of cycle-counting.

    Data transmit to chain is {G[7:0],R[7:0],B[7:0]}

    The pixel buffer is an array of 8-bit bytes, ordered GRB.  Alignment
    does not matter.

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
	push {r4-r7,lr}     // Save the other parameters we'll use
	ldr eclr, [sp, #20] // Get eclr from the stack, as argument #5

	// Figure out what address of fbptr is the last, so we know when to quit.
	mov bit, #3         // use bit temp for multiply
	mul stop, bit       // mult by 3 for RGB
	add stop, fbptr     // add in the fptr to finalize stop computation value

	// Load a brand-new pixel from the framebuffer pointer (10 cycles to pixloop_fromtop)
load_next_pixel:
	str bitmask, [eset]     // start with bit set
	mov bit, #8             // Start processing bit #8 of current pixel
	ldrb curpix, [fbptr]    // load the word at the pointer location to r5
	lsl curpix, #24 		// shift left by 24 so the carry pops out
	add fbptr, #1    		// Move to the next pixel
	b pixloop_fromtop       // 9 total

	// While processing the same pixel, send the next bit (10 cycles to pixloop_fromtop)
send_next_bit:
	str bitmask, [eset]   	// start with bit set
	bl wait_5_cycles		// Match the 5 cycles from load_next_pixel
	ldr eclr, [sp, #20]   	// Match 'b pixloop_fromtop' in load_next_pixel (delay for 2 cycles)
	// fall-through

// Both new pixels and new bits (i.e. load_next_pixel and send_next_bit)
// should now be synchronized.
pixloop_fromtop:
	ldr eclr, [sp, #20]   	// Delay 2 cycles
	lsl curpix, #1
	bcs oneBranch

zeroBranch:
	str bitmask, [eclr]

oneBranch:

	bl wait_14_cycles
	bl wait_14_cycles
	str bitmask, [eclr]

	bl wait_11_cycles
	sub bit, #1
	bne send_next_bit

	cmp fbptr, stop
	bne load_next_pixel

	b exit

exit:
	pop {r4-r7,pc}

wait_14_cycles:
	b wait_11_cycles
wait_11_cycles:
	b wait_8_cycles
wait_8_cycles:
	b wait_5_cycles
wait_5_cycles:
	bx lr

.endfunc
.type ledUpdate, %function
.size ledUpdate, .-ledUpdate

.end
