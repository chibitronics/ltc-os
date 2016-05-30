#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "oled.h"
#include "gfx.h"

#include "dv-serialmode.h"

#define MAX_COLS 18
#define MAX_ROWS 4
#define TEXT_LEN (MAX_COLS * MAX_ROWS)
static char text_buffer[TEXT_LEN];
static uint8_t write_ptr = 0; 

uint8_t isprint_local(char c) {
  return ((c >= ' ' && c <= '~') ? 1 : 0);
}

int find_printable_window(void) {
  int chars_searched = 0;
  int num_lines = 0;
  int cur_ptr = write_ptr;
  int chars_since_newline = 0;

  // starting from the last written character, search backwards...
  while( chars_searched < TEXT_LEN ) {
    if( (text_buffer[cur_ptr] == '\n') || (text_buffer[cur_ptr] == '\r' ) ) {
      //      chprintf(stream, "n");
      num_lines++;
      if( num_lines == (MAX_ROWS+1) ) { // forward back over the final newline we found
	cur_ptr++; cur_ptr %= TEXT_LEN;
      	break;
      }
      chars_since_newline = 0;
      chars_searched++;

      cur_ptr--;
      if( cur_ptr < 0 )
	cur_ptr = TEXT_LEN - 1;
      continue;
    }

    if( chars_since_newline >= (MAX_COLS-1) ) {
      //      chprintf(stream, "c");
      num_lines++;
      chars_since_newline = 0;
      if( num_lines >= (MAX_ROWS+1) ) {
	cur_ptr++; cur_ptr %= TEXT_LEN;
	break;
      }
    }
    cur_ptr--;
    if( cur_ptr < 0 )
      cur_ptr = TEXT_LEN - 1;
    chars_searched++;
    chars_since_newline++;
  }

  // this is a bodge case, should never happen actually (remove once debugged fully)
  if( cur_ptr == write_ptr ) {
    //    chprintf(stream, "x");
    cur_ptr = write_ptr + 1;
    cur_ptr %= TEXT_LEN;
  }
  //  chprintf(stream, "\n\r");
  
  return cur_ptr;
}

void update_screen(void) {
  coord_t width;
  coord_t font_height;
  font_t font;
  int cur_char;
  int i;
  int chars_processed;
  int render_line = 0;
  char str_to_render[MAX_COLS + 1];

  orchardGfxStart();
  font = gdispOpenFont("fixed_7x14");
  width = gdispGetWidth();
  //  height = gdispGetHeight();

  font_height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  chars_processed = 0;
  cur_char = find_printable_window();
  while( (chars_processed < TEXT_LEN) && (render_line <= MAX_ROWS) ) {
    // copy text starting at last newline into local render buffer
    i = 0;
    while( i < MAX_COLS ) {
      if( cur_char == write_ptr )
	break;

      if( (text_buffer[cur_char] == '\n') || (text_buffer[cur_char] == '\r') ) {
	str_to_render[i] = '*';
	cur_char++; i++; chars_processed++;
	cur_char %= TEXT_LEN;
	break;
      }
      if( isprint_local(text_buffer[cur_char]) )
	str_to_render[i] = text_buffer[cur_char];
      else
	str_to_render[i] = '.';
      cur_char++; i++; chars_processed++;
      cur_char %= TEXT_LEN;
    }
    str_to_render[i] = '\0';
    //    chprintf(stream, "%s\n\r", str_to_render);
    gdispDrawStringBox(0, render_line * font_height, width, gdispGetFontMetric(font, fontHeight),
		       (const char *) str_to_render, font, White, justifyLeft);

    render_line++;
    if( cur_char == write_ptr )
      break;
  }
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

void dvInit(void) {
  int i;
  for( i = 0; i < TEXT_LEN; i++ ) {
    text_buffer[i] = '+'; // init with whitespace
  }
  text_buffer[TEXT_LEN-1] = '\n'; // simulate final newline
  write_ptr = 0;
}

void dvDoSerial(void) {
  char c;
  
  if(chSequentialStreamRead((BaseSequentialStream *) stream, (uint8_t *)&c, 1) == 0)
    return;

  //  chSequentialStreamPut((BaseSequentialStream *)stream, c); // local echo to tx

  text_buffer[write_ptr] = c;

  write_ptr++;
  write_ptr %= TEXT_LEN;
  text_buffer[write_ptr] = ' '; // rule: current spot we're pointing to for write cannot be a newline
  
  update_screen();

}
