#ifndef H_INPUT
#define H_INPUT

#include "types.h"

enum keystate {
    KEY_PRESSED = 1,
    KEY_HOLD = 2,
    KEY_RELEASED = 3
};

enum keycode {
    // 0 - 7 so far unused ( later for mouse )

    KEY_CODE_BACKSPACE = 8, // same as ascii
    KEY_CODE_TAB,           // same as ascii
    // 10 - 12 undefined
    KEY_CODE_ENTER = 13,    // same as ascii
    KEY_CODE_SHIFT_LEFT,
    KEY_CODE_SHIFT_RIGHT,
    KEY_CODE_CAPS_LOCK,
    KEY_CODE_CTRL,
    KEY_CODE_CTRL_RIGHT,
    KEY_CODE_ALT,
    KEY_CODE_ALT_GR,
    KEY_CODE_WIN_LEFT,
    KEY_CODE_WIN_RIGHT,
    KEY_CODE_APPS,
    KEY_CODE_ESC = 27,       // same as ascii

    // 28 - 31 undefined

    // same as ascii
    KEY_CODE_SPACE = 32,
    // 33 - 41 undefined
    KEY_CODE_ASTERISK = 42,
    KEY_CODE_PLUS,
    KEY_CODE_COMMA,
    KEY_CODE_MINUS,
    KEY_CODE_POINT,
    KEY_CODE_SLASH,

    // same as ascii
    KEY_CODE_0,
    KEY_CODE_1,
    KEY_CODE_2,
    KEY_CODE_3,
    KEY_CODE_4,
    KEY_CODE_5,
    KEY_CODE_6,
    KEY_CODE_7,
    KEY_CODE_8,
    KEY_CODE_9,
    
    // 58 - 64 undefined

    // same as ascii
    KEY_CODE_A = 65,
    KEY_CODE_B,
    KEY_CODE_C,
    KEY_CODE_D,
    KEY_CODE_E,
    KEY_CODE_F,
    KEY_CODE_G,
    KEY_CODE_H,
    KEY_CODE_I,
    KEY_CODE_J,
    KEY_CODE_K,
    KEY_CODE_L,
    KEY_CODE_M,
    KEY_CODE_N,
    KEY_CODE_O,
    KEY_CODE_P,
    KEY_CODE_Q,
    KEY_CODE_R,
    KEY_CODE_S,
    KEY_CODE_T,
    KEY_CODE_U,
    KEY_CODE_V,
    KEY_CODE_W,
    KEY_CODE_X,
    KEY_CODE_Y,
    KEY_CODE_Z,

    KEY_CODE_OEM_1,         // ger layout: ü
    KEY_CODE_OEM_2,         // ger layout: #
    KEY_CODE_OEM_3,         // ger layout: ö
    KEY_CODE_OEM_4,         // ger layout: ß
    KEY_CODE_OEM_5,         // ger layout: ^
    KEY_CODE_OEM_6,         // ger layout: ´
    KEY_CODE_OEM_7,         // ger layout: ä
    KEY_CODE_OEM_8,
    KEY_CODE_OEM_102,       // ger layout: <

    KEY_CODE_OEM_PLUS,
    KEY_CODE_OEM_COMMA,
    KEY_CODE_OEM_MINUS,
    KEY_CODE_OEM_POINT,

    KEY_CODE_F1,
    KEY_CODE_F2,
    KEY_CODE_F3,
    KEY_CODE_F4,
    KEY_CODE_F5,
    KEY_CODE_F6,
    KEY_CODE_F7,
    KEY_CODE_F8,
    KEY_CODE_F9,
    KEY_CODE_F10,
    KEY_CODE_F11,
    KEY_CODE_F12,
    KEY_CODE_F13,
    KEY_CODE_F14,
    KEY_CODE_F15,
    KEY_CODE_F16,
    KEY_CODE_F17,
    KEY_CODE_F18,
    KEY_CODE_F19,
    KEY_CODE_F20,
    KEY_CODE_F21,
    KEY_CODE_F22,
    KEY_CODE_F23,
    KEY_CODE_F24,

    KEY_CODE_NUM,
    KEY_CODE_SCROLL,
    KEY_CODE_PAUSE,

    KEY_CODE_INSERT,
    KEY_CODE_DELETE,
    KEY_CODE_POS_1,
    KEY_CODE_END,
    KEY_CODE_PAGE_UP,
    KEY_CODE_PAGE_DOWN,
    
    KEY_CODE_UP,
    KEY_CODE_DOWN,
    KEY_CODE_LEFT,
    KEY_CODE_RIGHT,

    KEY_CODE_NUMPAD_0,
    KEY_CODE_NUMPAD_1,
    KEY_CODE_NUMPAD_2,
    KEY_CODE_NUMPAD_3,
    KEY_CODE_NUMPAD_4,
    KEY_CODE_NUMPAD_5,
    KEY_CODE_NUMPAD_6,
    KEY_CODE_NUMPAD_7,
    KEY_CODE_NUMPAD_8,
    KEY_CODE_NUMPAD_9,

    KEY_CODE_NUMPAD_SLASH,
    KEY_CODE_NUMPAD_ASTERISK,
    KEY_CODE_NUMPAD_MINUS,
    KEY_CODE_NUMPAD_PLUS,
    KEY_CODE_NUMPAD_ENTER,
    KEY_CODE_NUMPAD_DECIMAL,
};

enum keyboard_layouts {
    QWERTZ_LAYOUT
};

void select_keyboard_layout( enum keyboard_layouts layout );

char scancode_to_ascii( uint8_t scancode );

#endif // H_INPUT
