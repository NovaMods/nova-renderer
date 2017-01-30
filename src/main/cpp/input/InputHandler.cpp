#include "glad/glad.h"
#include "InputHandler.h"
#include "../render/nova_renderer.h"

namespace nova {


	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
		nova_renderer::instance->get_input_handler().queue_key_press_event({key,scancode,action,mods,1});
	}

	void key_character_callback(GLFWwindow *window, unsigned int key) {
		nova_renderer::instance->get_input_handler().queue_key_char_event({  key,1 });
	}

	void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
		nova_renderer::instance->get_input_handler().queue_mouse_button_event({ button,action,mods ,1 });
	}

	void mouse_position_callback(GLFWwindow *window, double xpos, double ypos) {
		nova_renderer::instance->get_input_handler().queue_mouse_position_event({(int)xpos,(int) nova_renderer::instance->get_game_window().get_size().y-(int) ypos,1 });
	}


	input_handler::input_handler() {};

	input_handler::~input_handler() {};

	void input_handler::queue_mouse_button_event(struct mouse_button_event e) {
		std::lock_guard<std::mutex> lock_guard(lock_button);
		mouse_button_event_queue.push(e);
	};

	struct mouse_button_event input_handler::dequeue_mouse_button_event() {
		std::lock_guard<std::mutex> lock_guard(lock_button);
		if (mouse_button_event_queue.size() != 0) {
			struct mouse_button_event e = mouse_button_event_queue.front();
			mouse_button_event_queue.pop();
			return e;
		}
		return { 0,0,0,0 };
	};


	void input_handler::queue_mouse_position_event(struct mouse_position_event e) {
		std::lock_guard<std::mutex> lock_guard(lock_position);
		mouse_position_event_queue.push(e);
	};

	struct mouse_position_event input_handler::dequeue_mouse_position_event() {
		std::lock_guard<std::mutex> lock_guard(lock_position);
		if (mouse_position_event_queue.size() != 0) {
			struct mouse_position_event e = mouse_position_event_queue.front();
			mouse_position_event_queue.pop();
			return e;
		}
		return { 0,0,0 };
	}

	void input_handler::queue_key_press_event(key_press_event e)
	{
		std::lock_guard<std::mutex> lock_guard(lock_key_press);
		key_press_event_queue.push(e);
	}

	key_press_event input_handler::dequeue_key_press_event()
	{
		std::lock_guard<std::mutex> lock_guard(lock_key_press);
		if (key_press_event_queue.size() != 0) {
			struct key_press_event e = key_press_event_queue.front();
			key_press_event_queue.pop();
			return e;
		}
		return {0,0,0,0,0};
	}

	void input_handler::queue_key_char_event(key_char_event e)
	{
		std::lock_guard<std::mutex> lock_guard(lock_key_char);
		key_char_event_queue.push(e);
	}

	key_char_event input_handler::dequeue_key_char_event()
	{
		std::lock_guard<std::mutex> lock_guard(lock_key_char);
		if (key_char_event_queue.size() != 0) {
			struct key_char_event e = key_char_event_queue.front();
			key_char_event_queue.pop();
			return e;
		}
		return {0,0};
	}
	

	void input_handler::create_keymap() {

		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_SPACE] = lwjgl_keycodes::KEY_SPACE;
		keymap[GLFW_KEY_APOSTROPHE] = lwjgl_keycodes::KEY_APOSTROPHE;
		keymap[GLFW_KEY_COMMA] = lwjgl_keycodes::KEY_COMMA;
		keymap[GLFW_KEY_MINUS] = lwjgl_keycodes::KEY_MINUS;
		keymap[GLFW_KEY_PERIOD] = lwjgl_keycodes::KEY_PERIOD;
		keymap[GLFW_KEY_SLASH] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_0] = lwjgl_keycodes::KEY_0;
		keymap[GLFW_KEY_1] = lwjgl_keycodes::KEY_1;
		keymap[GLFW_KEY_2] = lwjgl_keycodes::KEY_2;
		keymap[GLFW_KEY_3] = lwjgl_keycodes::KEY_3;
		keymap[GLFW_KEY_4] = lwjgl_keycodes::KEY_4;
		keymap[GLFW_KEY_5] = lwjgl_keycodes::KEY_5;
		keymap[GLFW_KEY_6] = lwjgl_keycodes::KEY_6;
		keymap[GLFW_KEY_7] = lwjgl_keycodes::KEY_7;
		keymap[GLFW_KEY_8] = lwjgl_keycodes::KEY_8;
		keymap[GLFW_KEY_9] = lwjgl_keycodes::KEY_9;
		keymap[GLFW_KEY_SEMICOLON] = lwjgl_keycodes::KEY_SEMICOLON;
		keymap[GLFW_KEY_EQUAL] = lwjgl_keycodes::KEY_EQUALS;
		keymap[GLFW_KEY_A] = lwjgl_keycodes::KEY_A;
		keymap[GLFW_KEY_B] = lwjgl_keycodes::KEY_B;
		keymap[GLFW_KEY_C] = lwjgl_keycodes::KEY_C;
		keymap[GLFW_KEY_D] = lwjgl_keycodes::KEY_D;
		keymap[GLFW_KEY_E] = lwjgl_keycodes::KEY_E;
		keymap[GLFW_KEY_F] = lwjgl_keycodes::KEY_F;
		keymap[GLFW_KEY_G] = lwjgl_keycodes::KEY_G;
		keymap[GLFW_KEY_H] = lwjgl_keycodes::KEY_H;
		keymap[GLFW_KEY_I] = lwjgl_keycodes::KEY_I;
		keymap[GLFW_KEY_J] = lwjgl_keycodes::KEY_J;
		keymap[GLFW_KEY_K] = lwjgl_keycodes::KEY_K;
		keymap[GLFW_KEY_L] = lwjgl_keycodes::KEY_L;
		keymap[GLFW_KEY_M] = lwjgl_keycodes::KEY_M;
		keymap[GLFW_KEY_N] = lwjgl_keycodes::KEY_N;
		keymap[GLFW_KEY_O] = lwjgl_keycodes::KEY_O;
		keymap[GLFW_KEY_P] = lwjgl_keycodes::KEY_P;
		keymap[GLFW_KEY_Q] = lwjgl_keycodes::KEY_Q;
		keymap[GLFW_KEY_R] = lwjgl_keycodes::KEY_R;
		keymap[GLFW_KEY_S] = lwjgl_keycodes::KEY_S;
		keymap[GLFW_KEY_T] = lwjgl_keycodes::KEY_T;
		keymap[GLFW_KEY_U] = lwjgl_keycodes::KEY_U;
		keymap[GLFW_KEY_V] = lwjgl_keycodes::KEY_V;
		keymap[GLFW_KEY_W] = lwjgl_keycodes::KEY_W;
		keymap[GLFW_KEY_X] = lwjgl_keycodes::KEY_X;
		keymap[GLFW_KEY_Y] = lwjgl_keycodes::KEY_Y;
		keymap[GLFW_KEY_Z] = lwjgl_keycodes::KEY_Z;
		keymap[GLFW_KEY_LEFT_BRACKET] = lwjgl_keycodes::KEY_LBRACKET;
		keymap[GLFW_KEY_BACKSLASH] = lwjgl_keycodes::KEY_BACKSLASH;
		keymap[GLFW_KEY_RIGHT_BRACKET] = lwjgl_keycodes::KEY_RBRACKET;
		keymap[GLFW_KEY_GRAVE_ACCENT] = lwjgl_keycodes::KEY_GRAVE;
		keymap[GLFW_KEY_WORLD_1] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_WORLD_2] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_ESCAPE] = lwjgl_keycodes::KEY_ESCAPE;
		keymap[GLFW_KEY_ENTER] = lwjgl_keycodes::KEY_RETURN;
		keymap[GLFW_KEY_TAB] = lwjgl_keycodes::KEY_TAB;
		keymap[GLFW_KEY_BACKSPACE] = lwjgl_keycodes::KEY_BACK;
		keymap[GLFW_KEY_INSERT] = lwjgl_keycodes::KEY_INSERT;
		keymap[GLFW_KEY_DELETE] = lwjgl_keycodes::KEY_DELETE;
		keymap[GLFW_KEY_RIGHT] = lwjgl_keycodes::KEY_RIGHT;
		keymap[GLFW_KEY_LEFT] = lwjgl_keycodes::KEY_LEFT;
		keymap[GLFW_KEY_DOWN] = lwjgl_keycodes::KEY_DOWN;
		keymap[GLFW_KEY_UP] = lwjgl_keycodes::KEY_UP;
		keymap[GLFW_KEY_PAGE_UP] = lwjgl_keycodes::KEY_PRIOR;
		keymap[GLFW_KEY_PAGE_DOWN] = lwjgl_keycodes::KEY_NEXT;
		keymap[GLFW_KEY_HOME] = lwjgl_keycodes::KEY_HOME;
		keymap[GLFW_KEY_END] = lwjgl_keycodes::KEY_END;
		keymap[GLFW_KEY_CAPS_LOCK] = lwjgl_keycodes::KEY_CAPITAL;
		keymap[GLFW_KEY_SCROLL_LOCK] = lwjgl_keycodes::KEY_SCROLL;
		keymap[GLFW_KEY_NUM_LOCK] = lwjgl_keycodes::KEY_NUMLOCK;
		keymap[GLFW_KEY_PRINT_SCREEN] = lwjgl_keycodes::KEY_SYSRQ;
		keymap[GLFW_KEY_PAUSE] = lwjgl_keycodes::KEY_PAUSE;
		keymap[GLFW_KEY_F1] = lwjgl_keycodes::KEY_F1;
		keymap[GLFW_KEY_F2] = lwjgl_keycodes::KEY_F2;
		keymap[GLFW_KEY_F3] = lwjgl_keycodes::KEY_F3;
		keymap[GLFW_KEY_F4] = lwjgl_keycodes::KEY_F4;
		keymap[GLFW_KEY_F5] = lwjgl_keycodes::KEY_F5;
		keymap[GLFW_KEY_F6] = lwjgl_keycodes::KEY_F6;
		keymap[GLFW_KEY_F7] = lwjgl_keycodes::KEY_F7;
		keymap[GLFW_KEY_F8] = lwjgl_keycodes::KEY_F8;
		keymap[GLFW_KEY_F9] = lwjgl_keycodes::KEY_F9;
		keymap[GLFW_KEY_F10] = lwjgl_keycodes::KEY_F10;
		keymap[GLFW_KEY_F11] = lwjgl_keycodes::KEY_F11;
		keymap[GLFW_KEY_F12] = lwjgl_keycodes::KEY_F12;
		keymap[GLFW_KEY_F13] = lwjgl_keycodes::KEY_F13;
		keymap[GLFW_KEY_F14] = lwjgl_keycodes::KEY_F14;
		keymap[GLFW_KEY_F15] = lwjgl_keycodes::KEY_F15;
		keymap[GLFW_KEY_F16] = lwjgl_keycodes::KEY_F16;
		keymap[GLFW_KEY_F17] = lwjgl_keycodes::KEY_F17;
		keymap[GLFW_KEY_F18] = lwjgl_keycodes::KEY_F18;
		keymap[GLFW_KEY_F19] = lwjgl_keycodes::KEY_F19;
		keymap[GLFW_KEY_F20] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_F21] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_F22] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_F23] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_F24] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_F25] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_KP_0] = lwjgl_keycodes::KEY_NUMPAD0;
		keymap[GLFW_KEY_KP_1] = lwjgl_keycodes::KEY_NUMPAD1;
		keymap[GLFW_KEY_KP_2] = lwjgl_keycodes::KEY_NUMPAD2;
		keymap[GLFW_KEY_KP_3] = lwjgl_keycodes::KEY_NUMPAD3;
		keymap[GLFW_KEY_KP_4] = lwjgl_keycodes::KEY_NUMPAD4;
		keymap[GLFW_KEY_KP_5] = lwjgl_keycodes::KEY_NUMPAD5;
		keymap[GLFW_KEY_KP_6] = lwjgl_keycodes::KEY_NUMPAD6;
		keymap[GLFW_KEY_KP_7] = lwjgl_keycodes::KEY_NUMPAD7;
		keymap[GLFW_KEY_KP_8] = lwjgl_keycodes::KEY_NUMPAD8;
		keymap[GLFW_KEY_KP_9] = lwjgl_keycodes::KEY_NUMPAD9;
		keymap[GLFW_KEY_KP_DECIMAL] = lwjgl_keycodes::KEY_NUMPADCOMMA;
		keymap[GLFW_KEY_KP_DIVIDE] = lwjgl_keycodes::KEY_DIVIDE;
		keymap[GLFW_KEY_KP_MULTIPLY] = lwjgl_keycodes::KEY_MULTIPLY;
		keymap[GLFW_KEY_KP_SUBTRACT] = lwjgl_keycodes::KEY_SUBTRACT;
		keymap[GLFW_KEY_KP_ADD] = lwjgl_keycodes::KEY_ADD;
		keymap[GLFW_KEY_KP_ENTER] = lwjgl_keycodes::KEY_NUMPADENTER;
		keymap[GLFW_KEY_KP_EQUAL] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_LEFT_SHIFT] = lwjgl_keycodes::KEY_LSHIFT;
		keymap[GLFW_KEY_LEFT_CONTROL] = lwjgl_keycodes::KEY_LCONTROL;
		keymap[GLFW_KEY_LEFT_ALT] = lwjgl_keycodes::KEY_LMENU;
		keymap[GLFW_KEY_LEFT_SUPER] = lwjgl_keycodes::KEY_LMETA;
		keymap[GLFW_KEY_RIGHT_SHIFT] = lwjgl_keycodes::KEY_RSHIFT;
		keymap[GLFW_KEY_RIGHT_CONTROL] = lwjgl_keycodes::KEY_RCONTROL;
		keymap[GLFW_KEY_RIGHT_ALT] = lwjgl_keycodes::KEY_RMENU;
		keymap[GLFW_KEY_RIGHT_SUPER] = lwjgl_keycodes::KEY_RMETA;
		keymap[GLFW_KEY_MENU] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;
		keymap[GLFW_KEY_UNKNOWN] = lwjgl_keycodes::KEY_NONE;


	}

}