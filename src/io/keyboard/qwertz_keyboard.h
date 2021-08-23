#ifndef H_QWERTZ_KEYBOARD
#define H_QWERTZ_KEYBOARD

#include "io/input.h"

static unsigned char qwertz_oem_table[13] = {
    154,    // Ü
    '#',
    153,    // Ö
    225,    // ß
    94,     // ^
    39,     // 39 is an apostrophe because greek letters has no '´' accent
    142,    // Ä
    0,      // undefined
    '<',

    '+',
    ',',
    '-',
    '.'
};

static unsigned char qwertz_numpad_table[6] = {
    '/',
    '*',
    '-',
    '+',
    '\n',
    ',',
};

static enum keycode qwertz_keycodes[256] = {
    0, // reserved
    KEY_CODE_ESC,
    KEY_CODE_1,
    KEY_CODE_2,
    KEY_CODE_3,
    KEY_CODE_4,
    KEY_CODE_5,
    KEY_CODE_6,
    KEY_CODE_7,
    KEY_CODE_8,
    KEY_CODE_9,
    KEY_CODE_0,
    KEY_CODE_OEM_4,     // ß
    KEY_CODE_OEM_6,     // ´
    KEY_CODE_BACKSPACE,
    KEY_CODE_TAB,
    KEY_CODE_Q,
    KEY_CODE_W,
    KEY_CODE_E,
    KEY_CODE_R,
    KEY_CODE_T,
    KEY_CODE_Z,
    KEY_CODE_U,
    KEY_CODE_I,
    KEY_CODE_O,
    KEY_CODE_P,
    KEY_CODE_OEM_1,     // Ü
    KEY_CODE_OEM_PLUS,
    KEY_CODE_ENTER,
    KEY_CODE_CTRL,
    KEY_CODE_A,
    KEY_CODE_S,
    KEY_CODE_D,
    KEY_CODE_F,
    KEY_CODE_G,
    KEY_CODE_H,
    KEY_CODE_J,
    KEY_CODE_K,
    KEY_CODE_L,
    KEY_CODE_OEM_3,     // Ö
    KEY_CODE_OEM_7,     // Ä
    KEY_CODE_OEM_5,     // ^
    KEY_CODE_SHIFT_LEFT,
    KEY_CODE_OEM_2,
    KEY_CODE_Y,
    KEY_CODE_X,
    KEY_CODE_C,
    KEY_CODE_V,
    KEY_CODE_B,
    KEY_CODE_N,
    KEY_CODE_M,
    KEY_CODE_OEM_COMMA,
    KEY_CODE_OEM_POINT,
    KEY_CODE_OEM_MINUS,
    KEY_CODE_SHIFT_RIGHT,
    KEY_CODE_NUMPAD_ASTERISK,
    KEY_CODE_ALT,
    KEY_CODE_SPACE,
    KEY_CODE_CAPS_LOCK,
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
    KEY_CODE_NUM,
    KEY_CODE_SCROLL,
    KEY_CODE_NUMPAD_7,
    KEY_CODE_NUMPAD_8,
    KEY_CODE_NUMPAD_9,
    KEY_CODE_NUMPAD_MINUS,
    KEY_CODE_NUMPAD_4,
    KEY_CODE_NUMPAD_5,
    KEY_CODE_NUMPAD_6,
    KEY_CODE_NUMPAD_PLUS,
    KEY_CODE_NUMPAD_1,
    KEY_CODE_NUMPAD_2,
    KEY_CODE_NUMPAD_3,
    KEY_CODE_NUMPAD_0,
    KEY_CODE_NUMPAD_DECIMAL,
    0,
    0,
    KEY_CODE_OEM_102,   // <
    [0x80 + 0x1c] = KEY_CODE_NUMPAD_ENTER,
    KEY_CODE_CTRL_RIGHT,
    [0x80 + 0x35] = KEY_CODE_NUMPAD_SLASH,
    [0x80 + 0x38] = KEY_CODE_ALT_GR,
    [0x80 + 0x47] = KEY_CODE_POS_1,
    KEY_CODE_UP,
    KEY_CODE_PAGE_UP,
    KEY_CODE_LEFT,
    [0x80 + 0x4d] = KEY_CODE_RIGHT,
    KEY_CODE_END,
    KEY_CODE_DOWN,
    KEY_CODE_PAGE_DOWN,
    KEY_CODE_INSERT,
    KEY_CODE_DELETE,
    [0x80 + 0x5b] = KEY_CODE_WIN_LEFT,
    KEY_CODE_WIN_RIGHT,
    KEY_CODE_APPS
};

#endif // H_QWERTZ_KEYBOARD
