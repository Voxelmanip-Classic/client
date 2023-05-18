/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "keycode.h"
#include "exceptions.h"
#include "settings.h"
#include "log.h"
#include "debug.h"
#include "util/hex.h"
#include "util/string.h"
#include "util/basic_macros.h"

class UnknownKeycode : public BaseException
{
public:
	UnknownKeycode(const char *s) :
		BaseException(s) {};
};

struct table_key {
	const char *Name;
	irr::EKEY_CODE Key;
	wchar_t Char; // L'\0' means no character assigned
	const char *LangName; // NULL means it doesn't have a human description
};

#define DEFINEKEY1(x, lang) /* Irrlicht key without character */ \
	{ #x, irr::x, L'\0', lang },
#define DEFINEKEY2(x, ch, lang) /* Irrlicht key with character */ \
	{ #x, irr::x, ch, lang },
#define DEFINEKEY3(ch) /* single Irrlicht key (e.g. KEY_KEY_X) */ \
	{ "KEY_KEY_" TOSTRING(ch), irr::KEY_KEY_ ## ch, (wchar_t) *TOSTRING(ch), TOSTRING(ch) },
#define DEFINEKEY4(ch) /* single Irrlicht function key (e.g. KEY_F3) */ \
	{ "KEY_F" TOSTRING(ch), irr::KEY_F ## ch, L'\0', "F" TOSTRING(ch) },
#define DEFINEKEY5(ch) /* key without Irrlicht keycode */ \
	{ ch, irr::KEY_KEY_CODES_COUNT, (wchar_t) *ch, ch },

#define N_(text) text

static const struct table_key table[] = {
	// Keys that can be reliably mapped between Char and Key
	DEFINEKEY3(0)
	DEFINEKEY3(1)
	DEFINEKEY3(2)
	DEFINEKEY3(3)
	DEFINEKEY3(4)
	DEFINEKEY3(5)
	DEFINEKEY3(6)
	DEFINEKEY3(7)
	DEFINEKEY3(8)
	DEFINEKEY3(9)
	DEFINEKEY3(A)
	DEFINEKEY3(B)
	DEFINEKEY3(C)
	DEFINEKEY3(D)
	DEFINEKEY3(E)
	DEFINEKEY3(F)
	DEFINEKEY3(G)
	DEFINEKEY3(H)
	DEFINEKEY3(I)
	DEFINEKEY3(J)
	DEFINEKEY3(K)
	DEFINEKEY3(L)
	DEFINEKEY3(M)
	DEFINEKEY3(N)
	DEFINEKEY3(O)
	DEFINEKEY3(P)
	DEFINEKEY3(Q)
	DEFINEKEY3(R)
	DEFINEKEY3(S)
	DEFINEKEY3(T)
	DEFINEKEY3(U)
	DEFINEKEY3(V)
	DEFINEKEY3(W)
	DEFINEKEY3(X)
	DEFINEKEY3(Y)
	DEFINEKEY3(Z)
	DEFINEKEY2(KEY_PLUS, L'+', "+")
	DEFINEKEY2(KEY_COMMA, L',', ",")
	DEFINEKEY2(KEY_MINUS, L'-', "-")
	DEFINEKEY2(KEY_PERIOD, L'.', ".")

	// Keys without a Char
	DEFINEKEY1(KEY_LBUTTON, "Left Button")
	DEFINEKEY1(KEY_RBUTTON, "Right Button")
	DEFINEKEY1(KEY_CANCEL, "Cancel")
	DEFINEKEY1(KEY_MBUTTON, "Middle Button")
	DEFINEKEY1(KEY_XBUTTON1, "X Button 1")
	DEFINEKEY1(KEY_XBUTTON2, "X Button 2")
	DEFINEKEY1(KEY_BACK, "Backspace")
	DEFINEKEY1(KEY_TAB, "Tab")
	DEFINEKEY1(KEY_CLEAR, "Clear")
	DEFINEKEY1(KEY_RETURN, "Return")
	DEFINEKEY1(KEY_SHIFT, "Shift")
	DEFINEKEY1(KEY_CONTROL, "Control")
	//~ Key name, common on Windows keyboards
	DEFINEKEY1(KEY_MENU, "Menu")
	DEFINEKEY1(KEY_PAUSE, "Pause")
	DEFINEKEY1(KEY_CAPITAL, "Caps Lock")
	DEFINEKEY1(KEY_SPACE, "Space")
	DEFINEKEY1(KEY_PRIOR, "Page up")
	DEFINEKEY1(KEY_NEXT, "Page down")
	DEFINEKEY1(KEY_END, "End")
	DEFINEKEY1(KEY_HOME, "Home")
	DEFINEKEY1(KEY_LEFT, "Left")
	DEFINEKEY1(KEY_UP, "Up")
	DEFINEKEY1(KEY_RIGHT, "Right")
	DEFINEKEY1(KEY_DOWN, "Down")
	//~ Key name
	DEFINEKEY1(KEY_SELECT, "Select")
	//~ "Print screen" key
	DEFINEKEY1(KEY_PRINT, "Print")
	DEFINEKEY1(KEY_EXECUT, "Execute")
	DEFINEKEY1(KEY_SNAPSHOT, "Snapshot")
	DEFINEKEY1(KEY_INSERT, "Insert")
	DEFINEKEY1(KEY_DELETE, "Delete")
	DEFINEKEY1(KEY_HELP, "Help")
	DEFINEKEY1(KEY_LWIN, "Left Windows")
	DEFINEKEY1(KEY_RWIN, "Right Windows")
	DEFINEKEY1(KEY_NUMPAD0, "Numpad 0") // These are not assigned to a char
	DEFINEKEY1(KEY_NUMPAD1, "Numpad 1") // to prevent interference with KEY_KEY_[0-9].
	DEFINEKEY1(KEY_NUMPAD2, "Numpad 2")
	DEFINEKEY1(KEY_NUMPAD3, "Numpad 3")
	DEFINEKEY1(KEY_NUMPAD4, "Numpad 4")
	DEFINEKEY1(KEY_NUMPAD5, "Numpad 5")
	DEFINEKEY1(KEY_NUMPAD6, "Numpad 6")
	DEFINEKEY1(KEY_NUMPAD7, "Numpad 7")
	DEFINEKEY1(KEY_NUMPAD8, "Numpad 8")
	DEFINEKEY1(KEY_NUMPAD9, "Numpad 9")
	DEFINEKEY1(KEY_MULTIPLY, "Numpad *")
	DEFINEKEY1(KEY_ADD, "Numpad +")
	DEFINEKEY1(KEY_SEPARATOR, "Numpad .")
	DEFINEKEY1(KEY_SUBTRACT, "Numpad -")
	DEFINEKEY1(KEY_DECIMAL, NULL)
	DEFINEKEY1(KEY_DIVIDE, "Numpad /")
	DEFINEKEY4(1)
	DEFINEKEY4(2)
	DEFINEKEY4(3)
	DEFINEKEY4(4)
	DEFINEKEY4(5)
	DEFINEKEY4(6)
	DEFINEKEY4(7)
	DEFINEKEY4(8)
	DEFINEKEY4(9)
	DEFINEKEY4(10)
	DEFINEKEY4(11)
	DEFINEKEY4(12)
	DEFINEKEY4(13)
	DEFINEKEY4(14)
	DEFINEKEY4(15)
	DEFINEKEY4(16)
	DEFINEKEY4(17)
	DEFINEKEY4(18)
	DEFINEKEY4(19)
	DEFINEKEY4(20)
	DEFINEKEY4(21)
	DEFINEKEY4(22)
	DEFINEKEY4(23)
	DEFINEKEY4(24)
	DEFINEKEY1(KEY_NUMLOCK, "Num Lock")
	DEFINEKEY1(KEY_SCROLL, "Scroll Lock")
	DEFINEKEY1(KEY_LSHIFT, "Left Shift")
	DEFINEKEY1(KEY_RSHIFT, "Right Shift")
	DEFINEKEY1(KEY_LCONTROL, "Left Control")
	DEFINEKEY1(KEY_RCONTROL, "Right Control")
	DEFINEKEY1(KEY_LMENU, "Left Menu")
	DEFINEKEY1(KEY_RMENU, "Right Menu")

	// Rare/weird keys
	DEFINEKEY1(KEY_KANA, "Kana")
	DEFINEKEY1(KEY_HANGUEL, "Hangul")
	DEFINEKEY1(KEY_HANGUL, "Hangul")
	DEFINEKEY1(KEY_JUNJA, "Junja")
	DEFINEKEY1(KEY_FINAL, "Final")
	DEFINEKEY1(KEY_KANJI, "Kanji")
	DEFINEKEY1(KEY_HANJA, "Hanja")
	DEFINEKEY1(KEY_ESCAPE, "IME Escape")
	DEFINEKEY1(KEY_CONVERT, "IME Convert")
	DEFINEKEY1(KEY_NONCONVERT, "IME Nonconvert")
	DEFINEKEY1(KEY_ACCEPT, "IME Accept")
	DEFINEKEY1(KEY_MODECHANGE, "IME Mode Change")
	DEFINEKEY1(KEY_APPS, "Apps")
	DEFINEKEY1(KEY_SLEEP, "Sleep")
	DEFINEKEY1(KEY_OEM_1, "OEM 1") // KEY_OEM_[0-9] and KEY_OEM_102 are assigned to multiple
	DEFINEKEY1(KEY_OEM_2, "OEM 2") // different chars (on different platforms too) and thus w/o char
	DEFINEKEY1(KEY_OEM_3, "OEM 3")
	DEFINEKEY1(KEY_OEM_4, "OEM 4")
	DEFINEKEY1(KEY_OEM_5, "OEM 5")
	DEFINEKEY1(KEY_OEM_6, "OEM 6")
	DEFINEKEY1(KEY_OEM_7, "OEM 7")
	DEFINEKEY1(KEY_OEM_8, "OEM 8")
	DEFINEKEY1(KEY_OEM_AX, "OEM AX")
	DEFINEKEY1(KEY_OEM_102, "OEM 102")
	DEFINEKEY1(KEY_ATTN, "Attn")
	DEFINEKEY1(KEY_CRSEL, "CrSel")
	DEFINEKEY1(KEY_EXSEL, "ExSel")
	DEFINEKEY1(KEY_EREOF, "Erase EOF")
	DEFINEKEY1(KEY_PLAY, "Play")
	DEFINEKEY1(KEY_ZOOM, "Zoom")
	DEFINEKEY1(KEY_PA1, "PA1")
	DEFINEKEY1(KEY_OEM_CLEAR, "OEM Clear")

	// Keys without Irrlicht keycode
	DEFINEKEY5("!")
	DEFINEKEY5("\"")
	DEFINEKEY5("#")
	DEFINEKEY5("$")
	DEFINEKEY5("%")
	DEFINEKEY5("&")
	DEFINEKEY5("'")
	DEFINEKEY5("(")
	DEFINEKEY5(")")
	DEFINEKEY5("*")
	DEFINEKEY5("/")
	DEFINEKEY5(":")
	DEFINEKEY5(";")
	DEFINEKEY5("<")
	DEFINEKEY5("=")
	DEFINEKEY5(">")
	DEFINEKEY5("?")
	DEFINEKEY5("@")
	DEFINEKEY5("[")
	DEFINEKEY5("\\")
	DEFINEKEY5("]")
	DEFINEKEY5("^")
	DEFINEKEY5("_")
};

#undef N_


struct table_key lookup_keyname(const char *name)
{
	for (const auto &table_key : table) {
		if (strcmp(table_key.Name, name) == 0)
			return table_key;
	}

	throw UnknownKeycode(name);
}

struct table_key lookup_keykey(irr::EKEY_CODE key)
{
	for (const auto &table_key : table) {
		if (table_key.Key == key)
			return table_key;
	}

	std::ostringstream os;
	os << "<Keycode " << (int) key << ">";
	throw UnknownKeycode(os.str().c_str());
}

struct table_key lookup_keychar(wchar_t Char)
{
	for (const auto &table_key : table) {
		if (table_key.Char == Char)
			return table_key;
	}

	std::ostringstream os;
	os << "<Char " << hex_encode((char*) &Char, sizeof(wchar_t)) << ">";
	throw UnknownKeycode(os.str().c_str());
}

KeyPress::KeyPress(const char *name)
{
	if (strlen(name) == 0) {
		Key = irr::KEY_KEY_CODES_COUNT;
		Char = L'\0';
		m_name = "";
		return;
	}

	if (strlen(name) <= 4) {
		// Lookup by resulting character
		int chars_read = mbtowc(&Char, name, 1);
		FATAL_ERROR_IF(chars_read != 1, "Unexpected multibyte character");
		try {
			struct table_key k = lookup_keychar(Char);
			m_name = k.Name;
			Key = k.Key;
			return;
		} catch (UnknownKeycode &e) {};
	} else {
		// Lookup by name
		m_name = name;
		try {
			struct table_key k = lookup_keyname(name);
			Key = k.Key;
			Char = k.Char;
			return;
		} catch (UnknownKeycode &e) {};
	}

	// It's not a known key, complain and try to do something
	Key = irr::KEY_KEY_CODES_COUNT;
	int chars_read = mbtowc(&Char, name, 1);
	FATAL_ERROR_IF(chars_read != 1, "Unexpected multibyte character");
	m_name = "";
	warningstream << "KeyPress: Unknown key '" << name
		<< "', falling back to first char." << std::endl;
}

KeyPress::KeyPress(const irr::SEvent::SKeyInput &in, bool prefer_character)
{
	if (prefer_character)
		Key = irr::KEY_KEY_CODES_COUNT;
	else
		Key = in.Key;
	Char = in.Char;

	try {
		if (valid_kcode(Key))
			m_name = lookup_keykey(Key).Name;
		else
			m_name = lookup_keychar(Char).Name;
	} catch (UnknownKeycode &e) {
		m_name.clear();
	};
}

const char *KeyPress::sym() const
{
	return m_name.c_str();
}

const char *KeyPress::name() const
{
	if (m_name.empty())
		return "";
	const char *ret;
	if (valid_kcode(Key))
		ret = lookup_keykey(Key).LangName;
	else
		ret = lookup_keychar(Char).LangName;
	return ret ? ret : "<Unnamed key>";
}

const KeyPress EscapeKey("KEY_ESCAPE");
const KeyPress CancelKey("KEY_CANCEL");

/*
	Key config
*/

// A simple cache for quicker lookup
std::unordered_map<std::string, KeyPress> g_key_setting_cache;

KeyPress getKeySetting(const char *settingname)
{
	std::unordered_map<std::string, KeyPress>::iterator n;
	n = g_key_setting_cache.find(settingname);
	if (n != g_key_setting_cache.end())
		return n->second;

	KeyPress k(g_settings->get(settingname).c_str());
	g_key_setting_cache[settingname] = k;
	return k;
}

void clearKeyCache()
{
	g_key_setting_cache.clear();
}

irr::EKEY_CODE keyname_to_keycode(const char *name)
{
	return lookup_keyname(name).Key;
}
