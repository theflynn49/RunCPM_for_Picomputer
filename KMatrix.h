
int getMatrix(int intr_process) ;
int hitMatrix(void) ;
void vt100_conout(char ch) ;

#define ST77XX_GREY 0xc618
// #define ST77XX_RED  0xCE60

#define TAB       0x9
#define BKSP      0x8
#define ENTER     0xd
#define ESC       0x1b

// the values returned by the function keys
#define F1        0x91
#define F2        0x92
#define F3        0x93
#define F4        0x94
#define F5        0x95
#define F6        0x96
#define F7        0x97
#define F8        0x98
#define F9        0x99
#define F10       0x9a
#define F11       0x9b
#define F12       0x9c

// the values returned by special control keys
#define UP        0x80
#define DOWN      0x81
#define LEFT      0x82
#define RIGHT     0x83
#define INSERT    0x84
#define DEL       0x7f
#define HOME      0x86
#define END       0x87
#define PUP       0x88
#define PDOWN     0x89
#define NUM_ENT   ENTER
#define SLOCK     0x8c
#define ALT       0x8b


// definitions related to setting the tab spacing
#define CONFIG_TAB2   0b111
#define CONFIG_TAB4   0b001
#define CONFIG_TAB8   0b010
