#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H



#include "GLFW/glfw3.h"
#include <queue>
#include <mutex>
#include "../mc_interface/mc_objects.h"
using namespace std;
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
		queue<struct MouseButtonEvent> mouseButtonEventQueue;
		queue<struct MousePositionEvent> mousePositionEventQueue;
		mutex lockButton;
		mutex lockPosition;
	};

}

#endif
