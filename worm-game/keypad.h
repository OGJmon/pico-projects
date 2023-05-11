// A. Sheaff 2023/01/05
// Keypad interface
#ifndef KEYPAD_H
#define KEYPAD_H

#define KEY_GPIO_START 5
#define KEY_NUMBER_GPIOS 8

// Get GPIOs setup
void keypad_init(void);
// Scan for a key.  Return -1 on no key
// 0-15 for key press.  Non-blocking
signed char getkey(void);
// Converts the key press to the symbol on the keypad
signed char mapkey(signed char key);

// Enumeration for key symbols
enum {backspace=10, pound, keyA,keyB,keyC,keyD};

#endif // KEYPAD_H
