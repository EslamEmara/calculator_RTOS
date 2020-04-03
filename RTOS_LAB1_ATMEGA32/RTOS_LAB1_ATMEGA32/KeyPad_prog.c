#include "std_types.h"
#include "utils.h"

#include "DIO_int.h"

#include "KeyPad_priv.h"
#include "KeyPad_config.h"
#include "KeyPad_int.h"

u8 KeyPad_u8GetPressedKey(void)
{
    u8 col,row;

    for(col=0; col<N_col; col++) /* loop for columns */
    {
        /*
         * Each time only one of the column pins will be output and
         * the rest will be input pins including the row pins
         */

        DIO_voidSetPortDir(KEYPAD_PORT, (0b00010000<<col));

        /*
         * clear the output pin column and enable the internal
         * pull up resistors for the other pins
         */
        DIO_voidSetPort(KEYPAD_PORT, (~(0b00010000<<col)));

        for(row=0; row<N_row; row++) /* loop for rows */
        {
            if(DIO_u8GetPinInPort(KEYPAD_PORT, row) == 0) /* if the switch is pressed in this row */
            {
	
                #if (N_col == 3)
				
                    return KeyPad_u8AdjustKeyNumber_4x3((row*N_col)+col+1);
                #elif (N_col == 4)
                    return KeyPad_u8AdjustKeyNumber_4x4((row*N_col)+col+1);
                #endif
            }
        }
    }

    return NO_KEY_PRESSED;
}

u8 KeyPad_u8AdjustKeyNumber_4x3(u8 button_number)
{
    switch(button_number)
	{
	    case 1: return '1';
				break;
		case 2: return '2';
				break;
		case 3: return '3';
				break;
		case 4: return '4'; // ASCII Code of %
				break;
		case 5: return '5';
				break;
		case 6: return '6';
				break;
		case 7: return '7';
				break;
		case 8: return '8'; /* ASCII Code of '*' */
				break;
		case 9: return '9';
				break;
		case 10: return '*'; // ASCII Code of =
				 break;
		case 11: return '0';
				 break;
		case 12: return '#'; // ASCII Code of +
				 break;
		default: return NO_KEY_PRESSED;
	}

}

u8 KeyPad_u8AdjustKeyNumber_4x4(u8 button_number)
{
    switch(button_number)
	{
		case 1: return '7';
				break;
		case 2: return '8';
				break;
		case 3: return '9';
				break;
		case 4: return '/'; // ASCII Code of %
				break;
		case 5: return '4';
				break;
		case 6: return '5';
				break;
		case 7: return '6';
				break;
		case 8: return '*'; /* ASCII Code of '*' */
				break;
		case 9: return '1';
				break;
		case 10: return '2';
				break;
		case 11: return '3';
				break;
		case 12: return '-'; /* ASCII Code of '-' */
				break;
		case 13: return 13;  /* ASCII of Enter */
				break;
		case 14: return '0';
				break;
		case 15: return '='; /* ASCII Code of '=' */
				break;
		case 16: return '+'; /* ASCII Code of '+' */
				break;
		default: return NO_KEY_PRESSED;
	}

}



