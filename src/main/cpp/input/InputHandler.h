#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H



#include "GLFW/glfw3.h"
#include <queue>
#include <mutex>
#include <unordered_map>
#include "../mc_interface/mc_objects.h"

namespace nova {

	enum class lwjgl_keycodes
	{
		KEY_NONE = 0,
		KEY_ESCAPE = 1,
		KEY_1 = 2,
		KEY_2 = 3,
		KEY_3 = 4,
		KEY_4 = 5,
		KEY_5 = 6,
		KEY_6 = 7,
		KEY_7 = 8,
		KEY_8 = 9,
		KEY_9 = 10,
		KEY_0 = 11,
		KEY_MINUS = 12,
		KEY_EQUALS = 13,
		KEY_BACK = 14,
		KEY_TAB = 15,
		KEY_Q = 16,
		KEY_W = 17,
		KEY_E = 18,
		KEY_R = 19,
		KEY_T = 20,
		KEY_Y = 21,
		KEY_U = 22,
		KEY_I = 23,
		KEY_O = 24,
		KEY_P = 25,
		KEY_LBRACKET = 26,
		KEY_RBRACKET = 27,
		KEY_RETURN = 28,
		KEY_LCONTROL = 29,
		KEY_A = 30,
		KEY_S = 31,
		KEY_D = 32,
		KEY_F = 33,
		KEY_G = 34,
		KEY_H = 35,
		KEY_J = 36,
		KEY_K = 37,
		KEY_L = 38,
		KEY_SEMICOLON = 39,
		KEY_APOSTROPHE = 40,
		KEY_GRAVE = 41,
		KEY_LSHIFT = 42,
		KEY_BACKSLASH = 43,
		KEY_Z = 44,
		KEY_X = 45,
		KEY_C = 46,
		KEY_V = 47,
		KEY_B = 48,
		KEY_N = 49,
		KEY_M = 50,
		KEY_COMMA = 51,
		KEY_PERIOD = 52,
		KEY_SLASH = 53,
		KEY_RSHIFT = 54,
		KEY_MULTIPLY = 55,
		KEY_LMENU = 56,
		KEY_SPACE = 57,
		KEY_CAPITAL = 58,
		KEY_F1 = 59,
		KEY_F2 = 60,
		KEY_F3 = 61,
		KEY_F4 = 62,
		KEY_F5 = 63,
		KEY_F6 = 64,
		KEY_F7 = 65,
		KEY_F8 = 66,
		KEY_F9 = 67,
		KEY_F10 = 68,
		KEY_NUMLOCK = 69,
		KEY_SCROLL = 70,
		KEY_NUMPAD7 = 71,
		KEY_NUMPAD8 = 72,
		KEY_NUMPAD9 = 73,
		KEY_SUBTRACT = 74,
		KEY_NUMPAD4 = 75,
		KEY_NUMPAD5 = 76,
		KEY_NUMPAD6 = 77,
		KEY_ADD = 78,
		KEY_NUMPAD1 = 79,
		KEY_NUMPAD2 = 80,
		KEY_NUMPAD3 = 81,
		KEY_NUMPAD0 = 82,
		KEY_DECIMAL = 83,
		KEY_F11 = 87,
		KEY_F12 = 88,
		KEY_F13 = 100,
		KEY_F14 = 101,
		KEY_F15 = 102,
		KEY_F16 = 103,
		KEY_F17 = 104,
		KEY_F18 = 105,
		KEY_KANA = 112,
		KEY_F19 = 113,
		KEY_CONVERT = 121,
		KEY_NOCONVERT = 123,
		KEY_YEN = 125,
		KEY_NUMPADEQUALS = 141,
		KEY_CIRCUMFLEX = 144,
		KEY_AT = 145,
		KEY_COLON = 146,
		KEY_UNDERLINE = 147,
		KEY_KANJI = 148,
		KEY_STOP = 149,
		KEY_AX = 150,
		KEY_UNLABELED = 151,
		KEY_NUMPADENTER = 156,
		KEY_RCONTROL = 157,
		KEY_SECTION = 167,
		KEY_NUMPADCOMMA = 179,
		KEY_DIVIDE = 181,
		KEY_SYSRQ = 183,
		KEY_RMENU = 184,
		KEY_FUNCTION = 196,
		KEY_PAUSE = 197,
		KEY_HOME = 199,
		KEY_UP = 200,
		KEY_PRIOR = 201,
		KEY_LEFT = 203,
		KEY_RIGHT = 205,
		KEY_END = 207,
		KEY_DOWN = 208,
		KEY_NEXT = 209,
		KEY_INSERT = 210,
		KEY_DELETE = 211,
		KEY_CLEAR = 218,
		KEY_LMETA = 219,
		KEY_LWIN = 219,
		KEY_RMETA = 220,
		KEY_RWIN = 220,
		KEY_APPS = 221,
		KEY_POWER = 222,
		KEY_SLEEP = 223,
		KEYBOARD_SIZE = 256
	};

	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	
	void key_character_callback(GLFWwindow *window, unsigned int key);

	void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

	void mouse_position_callback(GLFWwindow *window, double xpos, double ypos);

    void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

	class input_handler {
	public:
		input_handler();
		~input_handler();
		void queue_mouse_button_event(struct mouse_button_event  e);
		struct mouse_button_event dequeue_mouse_button_event();
		void queue_mouse_position_event(struct mouse_position_event  e);
		struct mouse_position_event dequeue_mouse_position_event();
        void queue_mouse_scroll_event(struct mouse_scroll_event  e);
        struct mouse_scroll_event dequeue_mouse_scroll_event();

		void queue_key_press_event(struct key_press_event  e);
		struct key_press_event dequeue_key_press_event();
		void queue_key_char_event(struct key_char_event  e);
		struct key_char_event dequeue_key_char_event();

	private:
		std::unordered_map<int, lwjgl_keycodes> keymap;
		std::queue<struct mouse_button_event> mouse_button_event_queue;
		std::queue<struct mouse_position_event> mouse_position_event_queue;
        std::queue<struct mouse_scroll_event> mouse_scroll_event_queue;
		std::mutex lock_button;
		std::mutex lock_position;
        std::mutex lock_scroll;

		std::queue<struct key_press_event> key_press_event_queue;
		std::queue<struct key_char_event> key_char_event_queue;
		std::mutex lock_key_press;
		std::mutex lock_key_char;
		void create_keymap();
	};


}
#endif
