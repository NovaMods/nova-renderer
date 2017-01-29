#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H



#include "GLFW/glfw3.h"
#include <queue>
#include <mutex>
#include "../mc_interface/mc_objects.h"

namespace nova {

	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

	void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

	void MousePositionCallback(GLFWwindow *window, double xpos, double ypos);

	class InputHandler {
	public:
		InputHandler();
		~InputHandler();
		void queueMouseButtonEvent(struct MouseButtonEvent  e);
		struct MouseButtonEvent dequeueMouseButtonEvent();
		void queueMousePositionEvent(struct MousePositionEvent  e);
		struct MousePositionEvent dequeueMousePositionEvent();
	private:
		std::queue<struct MouseButtonEvent> mouseButtonEventQueue;
		std::queue<struct MousePositionEvent> mousePositionEventQueue;
		std::mutex lockButton;
		std::mutex lockPosition;
	};


}
#endif
