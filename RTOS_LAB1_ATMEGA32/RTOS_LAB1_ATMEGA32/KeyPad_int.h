#ifndef KEYPAD_INT_H_
#define KEYPAD_INT_H_

#include"STD_TYPES.h"

u8 KeyPad_u8GetPressedKey(void);
u8 KeyPad_u8AdjustKeyNumber_4x3(u8 button_number);
u8 KeyPad_u8AdjustKeyNumber_4x4(u8 button_number);

#endif /* KEYPAD_INT_H_ */
