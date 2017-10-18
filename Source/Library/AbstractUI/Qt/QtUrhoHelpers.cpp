#include "QtUrhoHelpers.h"
#include <QMap>

namespace Urho3D
{

namespace
{

QMap<Qt::Key, int> BuildQtToUrho()
{
    QMap<Qt::Key, int> keymap;
    keymap[Qt::Key_unknown] = KEY_UNKNOWN;
    keymap[Qt::Key_Escape] = KEY_ESCAPE;
    keymap[Qt::Key_Tab] = KEY_TAB;
    keymap[Qt::Key_Backspace] = KEY_BACKSPACE;
    keymap[Qt::Key_Return] = KEY_RETURN;
    keymap[Qt::Key_Enter] = KEY_KP_ENTER;
    keymap[Qt::Key_Insert] = KEY_INSERT;
    keymap[Qt::Key_Delete] = KEY_DELETE;
    keymap[Qt::Key_Pause] = KEY_PAUSE;
    keymap[Qt::Key_Print] = KEY_PRINTSCREEN;
    keymap[Qt::Key_Home] = KEY_HOME;
    keymap[Qt::Key_End] = KEY_END;
    keymap[Qt::Key_Left] = KEY_LEFT;
    keymap[Qt::Key_Right] = KEY_RIGHT;
    keymap[Qt::Key_Up] = KEY_UP;
    keymap[Qt::Key_Down] = KEY_DOWN;
    keymap[Qt::Key_PageUp] = KEY_PAGEUP;
    keymap[Qt::Key_PageDown] = KEY_PAGEDOWN;
    keymap[Qt::Key_Shift] = KEY_LSHIFT;
    keymap[Qt::Key_Control] = KEY_LCTRL;
    keymap[Qt::Key_Alt] = KEY_LALT;
    keymap[Qt::Key_CapsLock] = KEY_CAPSLOCK;
    keymap[Qt::Key_NumLock] = KEY_NUMLOCKCLEAR;
    keymap[Qt::Key_ScrollLock] = KEY_SCROLLLOCK;
    keymap[Qt::Key_F1] = KEY_F1;
    keymap[Qt::Key_F2] = KEY_F2;
    keymap[Qt::Key_F3] = KEY_F3;
    keymap[Qt::Key_F4] = KEY_F4;
    keymap[Qt::Key_F5] = KEY_F5;
    keymap[Qt::Key_F6] = KEY_F6;
    keymap[Qt::Key_F7] = KEY_F7;
    keymap[Qt::Key_F8] = KEY_F8;
    keymap[Qt::Key_F9] = KEY_F9;
    keymap[Qt::Key_F10] = KEY_F10;
    keymap[Qt::Key_F11] = KEY_F11;
    keymap[Qt::Key_F12] = KEY_F12;
    keymap[Qt::Key_F13] = KEY_F13;
    keymap[Qt::Key_F14] = KEY_F14;
    keymap[Qt::Key_F15] = KEY_F15;

    for (int key = 'A'; key <= 'Z'; key++)
        keymap[Qt::Key(key)] = KEY_A + key - 'A';

    for (int key = '0'; key <= '9'; key++)
        keymap[Qt::Key(key)] = KEY_0 + key - '0';

    return keymap;
}

QMap<int, Qt::Key> BuildUrhoToQtMap()
{
    QMap<Qt::Key, int> source = BuildQtToUrho();
    QMap<int, Qt::Key> result;
    for (auto iter = source.begin(); iter != source.end(); ++iter)
        result[iter.value()] = iter.key();
    return result;
}

}

int Cast(Qt::Key key)
{
    static const QMap<Qt::Key, int> keymap = BuildQtToUrho();
    return keymap.value(key, KEY_UNKNOWN);
}

Qt::Key CastKey(int key)
{
    static const QMap<int, Qt::Key> keymap = BuildUrhoToQtMap();
    return keymap.value(key, Qt::Key_unknown);
}

}
