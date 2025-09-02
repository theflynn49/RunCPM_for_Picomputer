
/***********************************************************************************************************************
Picomputer Keyboard Matrix

KMatrix.c

written 2025 by theflynn49 (no copyright on this file)

************************************************************************************************************************/


#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
// #include "configuration.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/adc.h"
#include "hardware/exception.h"
#include "hardware/structs/timer.h"
#include "hardware/vreg.h"
#include "hardware/structs/pads_qspi.h"
#include "pico/unique_id.h"
#include "hardware/pwm.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "KMatrix.h"

#define BOUNCE_TIME 20000L

#define ROW_SZ  6
#define COL_SZ  6

static int kb_init = 0 ;
static uint32_t kb_state = 0 ;
static uint64_t timenow = 0L ;
static int last_c = -1 ;
static uint64_t timecur = 0 ; // time for cursor
static bool cursoron = false ;
static int cursorX, cursorY ;

// The basic keymap
static __uint8_t key_table [3]  [ROW_SZ * COL_SZ] = {
	{
	  ' ',   '.', 'm', 'n', 'b', DOWN,
	  ENTER, 'l', 'k', 'j', 'h', LEFT,
	  'p',   'o', 'i', 'u', 'y', UP,
	  BKSP,  'z', 'x', 'c', 'v', RIGHT,
	  'a',   's', 'd', 'f', 'g', ESC,
	  'q',   'w', 'e', 'r', 't', ALT 
	},

	{
	  ':', ',', '>', '<', '"', '{',
	  '~', '-', '*', '&', '+', '[',
	  '0', '9', '8', '7', '6', '}',
	  '=', '(', ')', '?', '/', ']',
	  '!', '@', '#', '$', '%', '\\',
	  '1', '2', '3', '4', '5', ALT
	},

	{
	  '_',   ';', 'M', 'N', 'B', DOWN,
	  ENTER, 'L', 'K', 'J', 'H', LEFT,
	  'P',   'O', 'I', 'U', 'Y', UP,
	  BKSP,  'Z', 'X', 'C', 'V', RIGHT,
	  'A',   'S', 'D', 'F', 'G', TAB,
	  'Q',   'W', 'E', 'R', 'T', ALT 
	}
};

// VT100 extended color codes see :https://en.wikipedia.org/wiki/ANSI_escape_code
// coding of this screen : <5-bits red><6-bits green><5-bits blue>
static __uint16_t vt100_color_table [256] = {
    // colors 0-16 correspond to the ANSI and aixterm naming
    0x0000,0xC800,0x0660,0xCE60,0x001D,0xC819,0x0679,0xE73C,0x7BEF,0xF800,0x07E0,0xFFE0,0x5AFF,0xF81F,0x07FF,0xFFFF,
    // colors 16-231 are a 6x6x6 color cube
    0x0000,0x000B,0x0010,0x0015,0x001A,0x001F,0x02E0,0x02EB,0x02F0,0x02F5,0x02FA,0x02FF,0x0420,0x042B,0x0430,0x0435,
    0x043A,0x043F,0x0560,0x056B,0x0570,0x0575,0x057A,0x057F,0x06A0,0x06AB,0x06B0,0x06B5,0x06BA,0x06BF,0x07E0,0x07EB,
    0x07F0,0x07F5,0x07FA,0x07FF,0x5800,0x580B,0x5810,0x5815,0x581A,0x581F,0x5AE0,0x5AEB,0x5AF0,0x5AF5,0x5AFA,0x5AFF,
    0x5C20,0x5C2B,0x5C30,0x5C35,0x5C3A,0x5C3F,0x5D60,0x5D6B,0x5D70,0x5D75,0x5D7A,0x5D7F,0x5EA0,0x5EAB,0x5EB0,0x5EB5,
    0x5EBA,0x5EBF,0x5FE0,0x5FEB,0x5FF0,0x5FF5,0x5FFA,0x5FFF,0x8000,0x800B,0x8010,0x8015,0x801A,0x801F,0x82E0,0x82EB,
    0x82F0,0x82F5,0x82FA,0x82FF,0x8420,0x842B,0x8430,0x8435,0x843A,0x843F,0x8560,0x856B,0x8570,0x8575,0x857A,0x857F,
    0x86A0,0x86AB,0x86B0,0x86B5,0x86BA,0x86BF,0x87E0,0x87EB,0x87F0,0x87F5,0x87FA,0x87FF,0xA800,0xA80B,0xA810,0xA815,
    0xA81A,0xA81F,0xAAE0,0xAAEB,0xAAF0,0xAAF5,0xAAFA,0xAAFF,0xAC20,0xAC2B,0xAC30,0xAC35,0xAC3A,0xAC3F,0xAD60,0xAD6B,
    0xAD70,0xAD75,0xAD7A,0xAD7F,0xAEA0,0xAEAB,0xAEB0,0xAEB5,0xAEBA,0xAEBF,0xAFE0,0xAFEB,0xAFF0,0xAFF5,0xAFFA,0xAFFF,
    0xD000,0xD00B,0xD010,0xD015,0xD01A,0xD01F,0xD2E0,0xD2EB,0xD2F0,0xD2F5,0xD2FA,0xD2FF,0xD420,0xD42B,0xD430,0xD435,
    0xD43A,0xD43F,0xD560,0xD56B,0xD570,0xD575,0xD57A,0xD57F,0xD6A0,0xD6AB,0xD6B0,0xD6B5,0xD6BA,0xD6BF,0xD7E0,0xD7EB,
    0xD7F0,0xD7F5,0xD7FA,0xD7FF,0xF800,0xF80B,0xF810,0xF815,0xF81A,0xF81F,0xFAE0,0xFAEB,0xFAF0,0xFAF5,0xFAFA,0xFAFF,
    0xFC20,0xFC2B,0xFC30,0xFC35,0xFC3A,0xFC3F,0xFD60,0xFD6B,0xFD70,0xFD75,0xFD7A,0xFD7F,0xFEA0,0xFEAB,0xFEB0,0xFEB5,
    0xFEBA,0xFEBF,0xFFE0,0xFFEB,0xFFF0,0xFFF5,0xFFFA,0xFFFF,
    // colors 232-255 are a grayscale ramp, intentionally leaving out black and white
    0x0841,0x1082,0x18E3,0x2124,0x3186,0x39C7,0x4228,0x4A69,0x5ACB,0x630C,0x6B6D,0x73AE,0x8410,0x8C51,0x94B2,0x9CF3,0xAD55,0xB596,0xBDF7,0xC638,0xD69A,0xDEDB,0xE73C,0xEF7D
} ;


// board.GP1, board.GP2, board.GP3, board.GP4, board.GP5, board.GP14)
// board.GP6, board.GP9, board.GP15, board.GP8, board.GP7, board.GP22

static uint cols[COL_SZ] = { 1, 2, 3, 4, 5, 14 } ;
static uint rows[ROW_SZ] = { 6, 9, 15, 8, 7, 22 } ;
char key_buffer[32] ;
int key_buffer_ndx = -1 ;
int idbg = 0 ;
char vt100_dbg[101] ;
__uint16_t vt100_fg_color = ST77XX_GREY ;
__uint16_t vt100_back_color = ST77XX_BLACK ;

#define mask_all 0x40c3fe 

void cursor_toggle(void){
  if ((timecur==0) || (timecur<time_us_64())) {
    if (cursoron) {
      tft.drawLine(cursorX, cursorY, cursorX+5, cursorY, ST77XX_BLACK) ;
    } else {
      cursorX=tft.getCursorX() ;
      cursorY=tft.getCursorY()+7 ;
      tft.drawLine(cursorX, cursorY, cursorX+5, cursorY, ST77XX_WHITE) ;
    }
    timecur = time_us_64() + 200000 ;
    cursoron = !cursoron ;
  }
}

extern int hitMatrix(void) {
  int c ;
  cursor_toggle() ;
  if (key_buffer_ndx != -1) return 1 ;
  c = getMatrix(0) ;
  if (c != -1) {
    key_buffer[++key_buffer_ndx] = c & 0xff ;
    return 1 ;
  } 
  return 0 ;
}


extern int getMatrix(int intr_process) {
   int c = getMatrix_n(intr_process) ;
   switch(c & 0xff) {
     case UP : if (key_buffer_ndx >= 28) return -1 ;
               key_buffer[++key_buffer_ndx]='A'; 
               key_buffer[++key_buffer_ndx]='[';
               return 0x1b ;
     case DOWN : if (key_buffer_ndx >= 28) return -1 ;
               key_buffer[++key_buffer_ndx]='B'; 
               key_buffer[++key_buffer_ndx]='[';
               return 0x1b ;
     case LEFT : if (key_buffer_ndx >= 28) return -1 ;
               key_buffer[++key_buffer_ndx]='D'; 
               key_buffer[++key_buffer_ndx]='[';
               return 0x1b ;
     case RIGHT : if (key_buffer_ndx >= 28) return -1 ;
               key_buffer[++key_buffer_ndx]='C'; 
               key_buffer[++key_buffer_ndx]='[';
               return 0x1b ;
     default : return c ;
   }
}

int getMatrix_n(int intr_process) {
    int c=-1;
    int c0=-1 ;
    int c1=-1 ;
    int _i, _j, _r, _sft, _cw ;
    int _w[ROW_SZ] ;
    // char _sdebug[25] ;
    // strcpy(_sdebug, "                     ") ;

    cursor_toggle() ;
    if (kb_init == 0) { // are we initialized ?
    	gpio_init_mask(mask_all) ;
    	gpio_set_dir_in_masked(mask_all) ;
    	for (_i=0; _i<COL_SZ; ++_i) {
	    gpio_set_pulls(cols[_i], true, false) ;
    	}
    	for (_i=0; _i<ROW_SZ; ++_i) {
	    gpio_set_pulls(rows[_i], false, false) ;
    	}
      kb_init = 1 ;
  /*
	// do not reserve GP1 and GP4, since audio does GP1 and LCDPANEL gets GP4 but doesn't use it.
	// numbers below are PIN numbers, not GPIO's.

 */
    }
    if (key_buffer_ndx != -1){
  //    Serial.write('!'+key_buffer_ndx) ;
      c = key_buffer[key_buffer_ndx--] ;
      return c ;
    }
    if (timenow!=0L && timenow>time_us_64()) return -1 ; // debouncing
    timenow=0L ; 
    for (_i=0; _i<ROW_SZ; ++_i) {
	    gpio_set_dir(rows[_i], GPIO_OUT);
	    gpio_put(rows[_i], 0) ;
	    sleep_us(10);
	    _r = gpio_get_all() & 0x403E ; // bits 1, 2, 3, 4, 5, 14
	    _r = ((_r&0x3e)>>1) | ((_r>>9)&0x20) ; 
	    gpio_put(rows[_i], 1) ;
	    gpio_set_dir(rows[_i], GPIO_IN);
	    _w[_i]=_r ; 
    }
    
    //
    _sft = _w[5] & 0x20 ; // shift key 
    _w[5] |= 0x20 ;
    for (_i=0; _i<ROW_SZ; ++_i) if (_w[_i] != 0x3F) {
	  _cw = _w[_i] ;
	  for (_j=0; _j<COL_SZ; ++_j)
	  {
		  if ((_cw & 1) == 0)
		  {
		    c = key_table [kb_state] [_i*COL_SZ+_j] ;
		    c0 = key_table [0] [_i*COL_SZ+_j] ;
		    c1 = key_table [1] [_i*COL_SZ+_j] ;
		    break ;
		  } 
		  _cw >>= 1 ;
	  }
	  if (c != -1) break ;
    } 
    // internal KB commands
    if (_sft==0) {
	  if (c0==UP) { kb_state=2 ; c=-1 ; }
	  else if (c0==DOWN) { kb_state=0 ; c=-1 ; }
	  else if (c0==RIGHT) { kb_state=1 ; c=-1 ; }
	  else { 
		  c=c1 ;
		 if (kb_state==1) 
		 {
			 if ((c0>='a')&&(c0<='z')) c=c0-0x60 ; else c=c0 ;
		 }
	  } 
    }
    if ((last_c != -1) && (c == -1)) {
    	timenow=time_us_64()+BOUNCE_TIME;  // debouncing on key release
    }
    if (c==-1) last_c=-1 ;
    if (last_c==c) c=-1 ; 
    if ((last_c!=c)&&(c!=-1))
    {
	    timenow=time_us_64()+BOUNCE_TIME; // debouncing on key press
    	last_c=c ;
	}
    return c;
}

static int vt100_state = 0 ;
static int vt100_mode = 0 ;
static int vt100_mode2 = 0 ;
static int vt100_mode3 = 0 ;
static int vt100_arg = 0 ;


void vt100_hexprt(char c) {
  if (c>=10) Serial.write('a'+c-10) ; else Serial.write('0'+c) ;
}

void vt100_dbgprint(void){
  /*
  for (int i=0; i<idbg; ++i) {
    char c = vt100_dbg[i] ;
    if ((unsigned char)c < 0x20)
    {
      Serial.write('\\') ;
      Serial.write('x') ;
      vt100_hexprt(c>>4) ;
      vt100_hexprt(c&0xF) ;
    } else Serial.write(c) ;
  } 
  Serial.write(0xA) ; 
  Serial.write(0xD);
  */
  idbg=0 ;
}

void vt100_state0(void){
  if (idbg!=0) vt100_dbgprint() ;
  vt100_state=0 ;
}

void vt100_clearscreen(int fblack) {
  tft.setCursor(318-6*7, 232) ;
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK) ;
  tft.fillRect(tft.getCursorX(), tft.getCursorY(), 6*7, 8, ST77XX_BLACK) ; 
  tft.print("<wait> ") ;
  tft.setTextColor(vt100_fg_color, vt100_back_color) ;
  if (fblack!=0)
    tft.fillScreen(ST77XX_BLACK); else
    tft.fillScreen(vt100_back_color);
  tft.setCursor(0,0) ;  
}

extern void vt100_conout(char ch)
{
  int c ;
  if (((vt100_state!=0)||(ch<32)||true) && (idbg<100)) vt100_dbg[idbg++]=ch ; 
  switch (vt100_state) {
    case 0 : {
      switch(ch) {
      case 7 : generate_beep_200hz() ; return ; //bell
      case 8 : {
        int Y=tft.getCursorY() ;
        int X=tft.getCursorX() ;
        if (X<6) {
          X=312 ; // 52*6
          if (Y<8) { X=0; Y=0 ; } else Y-=8 ;
          tft.setCursor(X, Y) ;
        } else {
          X-=6 ;
          tft.setCursor(X, Y) ;
        }
        tft.fillRect(tft.getCursorX(), tft.getCursorY(), 6, 8, vt100_back_color) ;
        return ;
      }
      case 12 : vt100_clearscreen(0) ; return ;
      case 27 : vt100_state = 1 ; return ;
      // case 32 : tft.fillRect(tft.getCursorX(), tft.getCursorY(), 6, 8, vt100_back_color) ; // then, fall-though
      default : {
          tft.print((char)(ch & 0x7f)) ; 
          if (tft.getCursorY()>=232) // 240-8
          {
            tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK) ;
            tft.setCursor(318-6*7, 232) ;
            tft.print("<Space>") ;
            for(c=0;c!=' ' && c!=':' && c!='_'; c=_getch()) ;
            vt100_clearscreen(1) ;
          }
          if (idbg!=0) vt100_dbgprint() ;
          return ;
        }
      }
    }
    case 1 : {
      switch (ch) {
        case '[': vt100_mode = 0 ; vt100_arg=1; vt100_mode2 = 0; vt100_mode3 = 0; vt100_state = 2 ; return ; 
        case '#': vt100_state = 3 ; return ; 
        default : vt100_state0() ; return ;
      }
    }
    case 2 : {  // \e[
      if ((ch>='0')&&(ch<='9')) {
        switch(vt100_arg) {
          case 1 : vt100_mode = vt100_mode*10+(ch-'0') ;  break ;
          case 2 : vt100_mode2 = vt100_mode2*10+(ch-'0') ; break ;
          case 3 : vt100_mode3 = vt100_mode3*10+(ch-'0') ; break ;
        }
        return ;
      }
      else if (ch==';') { vt100_arg++; return ; }
      else if (ch=='J') {
        vt100_state0() ;
        switch(vt100_mode) {
          case 2 : tft.fillScreen(vt100_back_color); tft.setCursor(0,0) ; return ;
        }        
      }
      else if (ch=='K') {
          tft.fillRect(tft.getCursorX(), tft.getCursorY(), 320 - tft.getCursorX()-1, 8, vt100_back_color) ; 
          vt100_state0() ;
      } else if (ch=='H') {
          tft.setCursor(vt100_mode2*6, vt100_mode*8 ) ;
          vt100_state0() ;
      } else if (ch=='m') {
        vt100_state0() ;
        switch(vt100_mode) {
        case 0: vt100_fg_color = ST77XX_GREY ; 
                vt100_back_color = ST77XX_BLACK ;
                //tft.setTextColor(ST77XX_GREY, ST77XX_BLACK); 
                break;  // light grey
        case 1: vt100_fg_color = ST77XX_WHITE ; 
                vt100_back_color = ST77XX_BLACK ;
                //tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK ); 
                break;
        case 38: if ((vt100_mode2==5) && (vt100_arg==3)) {
          vt100_fg_color = vt100_color_table[vt100_mode3 & 255] ; break ;
        } // else fall through
        case 48: if ((vt100_mode2==5) && (vt100_arg==3)) {
          vt100_back_color = vt100_color_table[vt100_mode3 & 255] ; break ;
        } // else fall through
        default : if (vt100_arg==1) { 
            if ((vt100_mode>=30)&&(vt100_mode<=37)) {
              vt100_fg_color = vt100_color_table[vt100_mode-30] ; break ;
            } else if ((vt100_mode>=90)&&(vt100_mode<=97)) {
              vt100_fg_color = vt100_color_table[vt100_mode-90+8] ; break ;
            } else if ((vt100_mode>=40)&&(vt100_mode<=47)) {
              vt100_back_color = vt100_color_table[vt100_mode-40] ; break ;
            } else if ((vt100_mode>=100)&&(vt100_mode<=107)) {
              vt100_back_color = vt100_color_table[vt100_mode-100+8] ; break ;
            }
          }
          return ;
        }
        tft.setTextColor(vt100_fg_color, vt100_back_color) ;
        return ;
      } else {
        vt100_state0() ;
        return ;
      }
    }
    case 3 : { // \e#N
      vt100_state0() ;
      return ;
    }
    default : vt100_state0() ; return ;
  }
}


// Function to set up PWM for generating a 440Hz tone on a GPIO
void generate_beep_200hz() {
    const uint BUZZER_PIN = 0; // GPIO connected to passive buzzer

    // Determine PWM slice for the buzzer pin
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Set buzzer pin to PWM function
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Configure PWM frequency to ~440Hz
    // Formula: PWM freq = clock_freq / (wrap * (1 + div_int + div_frac/128))
    // Using system clock (250 MHz), aim for ~440Hz
    uint32_t clock_freq = 250000000;
    uint16_t wrap = 62500; // Example value for low frequency
    float div = (float)clock_freq / (440 * wrap);

    // Set integer and fractional dividers
    uint8_t div_int = (uint8_t)div;
    uint8_t div_frac = (uint8_t)((div - div_int) * 128);

    pwm_set_clkdiv_int_frac(slice_num, div_int, div_frac);
    pwm_set_wrap(slice_num, wrap);

    // Set duty cycle to 0.8% for a very faint sound
    pwm_set_gpio_level(BUZZER_PIN, wrap / 128);

    // Enable PWM
    pwm_set_enabled(slice_num, true);

    // Play tone for 100ms
    sleep_ms(100);

    // Turn off PWM
    pwm_set_enabled(slice_num, false);
}

