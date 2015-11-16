#ifndef INCLUDE_KEYBOARD
#define INCLUDE_KEYBOARD

extern void akdb_init();
extern void keyb_handler();
extern void setleds();
extern unsigned char getch();
extern unsigned char kbhit();
extern void gets(char *string);

#endif

