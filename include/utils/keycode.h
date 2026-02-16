#pragma once
#include <windows.h>

enum Keycode
{
    KEY_NONE = 0,

    // Letters
    KEY_A = 'A',
    KEY_B = 'B',
    KEY_C = 'C',
    KEY_D = 'D',
    KEY_E = 'E',
    KEY_F = 'F',
    KEY_G = 'G',
    KEY_H = 'H',
    KEY_I = 'I',
    KEY_J = 'J',
    KEY_K = 'K',
    KEY_L = 'L',
    KEY_M = 'M',
    KEY_N = 'N',
    KEY_O = 'O',
    KEY_P = 'P',
    KEY_Q = 'Q',
    KEY_R = 'R',
    KEY_S = 'S',
    KEY_T = 'T',
    KEY_U = 'U',
    KEY_V = 'V',
    KEY_W = 'W',
    KEY_X = 'X',
    KEY_Y = 'Y',
    KEY_Z = 'Z',

    // Numbers
    KEY_0 = '0',
    KEY_1 = '1',
    KEY_2 = '2',
    KEY_3 = '3',
    KEY_4 = '4',
    KEY_5 = '5',
    KEY_6 = '6',
    KEY_7 = '7',
    KEY_8 = '8',
    KEY_9 = '9',

    // Function keys
    KEY_F1  = VK_F1,
    KEY_F2  = VK_F2,
    KEY_F3  = VK_F3,
    KEY_F4  = VK_F4,
    KEY_F5  = VK_F5,
    KEY_F6  = VK_F6,
    KEY_F7  = VK_F7,
    KEY_F8  = VK_F8,
    KEY_F9  = VK_F9,
    KEY_F10 = VK_F10,
    KEY_F11 = VK_F11,
    KEY_F12 = VK_F12,

    // Arrows
    KEY_LEFT  = VK_LEFT,
    KEY_RIGHT = VK_RIGHT,
    KEY_UP    = VK_UP,
    KEY_DOWN  = VK_DOWN,

    // Modifiers
    KEY_SHIFT   = VK_SHIFT,
    KEY_CONTROL = VK_CONTROL,
    KEY_ALT     = VK_MENU,
    KEY_CAPSLOCK = VK_CAPITAL,
    KEY_TAB      = VK_TAB,
    KEY_ESCAPE   = VK_ESCAPE,
    KEY_SPACE    = VK_SPACE,
    KEY_ENTER    = VK_RETURN,
    KEY_BACKSPACE = VK_BACK,
    KEY_DELETE   = VK_DELETE,
    KEY_INSERT   = VK_INSERT,
    KEY_HOME     = VK_HOME,
    KEY_END      = VK_END,
    KEY_PAGEUP   = VK_PRIOR,
    KEY_PAGEDOWN = VK_NEXT,

    // Numpad
    KEY_NUMPAD0 = VK_NUMPAD0,
    KEY_NUMPAD1 = VK_NUMPAD1,
    KEY_NUMPAD2 = VK_NUMPAD2,
    KEY_NUMPAD3 = VK_NUMPAD3,
    KEY_NUMPAD4 = VK_NUMPAD4,
    KEY_NUMPAD5 = VK_NUMPAD5,
    KEY_NUMPAD6 = VK_NUMPAD6,
    KEY_NUMPAD7 = VK_NUMPAD7,
    KEY_NUMPAD8 = VK_NUMPAD8,
    KEY_NUMPAD9 = VK_NUMPAD9,
    KEY_NUMPAD_DIV   = VK_DIVIDE,
    KEY_NUMPAD_MUL   = VK_MULTIPLY,
    KEY_NUMPAD_SUB   = VK_SUBTRACT,
    KEY_NUMPAD_ADD   = VK_ADD,
    KEY_NUMPAD_DEC   = VK_DECIMAL,
    KEY_NUMPAD_ENTER = VK_RETURN,

    // Symbols
    KEY_MINUS   = VK_OEM_MINUS,
    KEY_PLUS    = VK_OEM_PLUS,
    KEY_COMMA   = VK_OEM_COMMA,
    KEY_PERIOD  = VK_OEM_PERIOD,
    KEY_SLASH   = VK_OEM_2,  // '/'
    KEY_BACKSLASH = VK_OEM_5,
    KEY_SEMICOLON = VK_OEM_1,
    KEY_QUOTE     = VK_OEM_7,
    KEY_TILDE     = VK_OEM_3, // '`'

    // Mouse
    MOUSE_LEFT   = 0,
    MOUSE_RIGHT  = 1,
    MOUSE_MIDDLE = 2
};
