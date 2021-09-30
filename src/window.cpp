#include "breakout/window.h"

#include "breakout/log.h"
#include "breakout/gl_debug.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

bool Initialize(GLFWwindow*& window, int width, int height, const char* windowName);

//===== Window =====

#define WINDOW_VALIDITY_CHECK() ASSERT_MSG(window != nullptr, "\tAttempting to use uninitialized window.\n")

Window& Window::Get() {
	static Window instance = Window();
	return instance;
}

void Window::Init(int width_, int height_, const char* windowName) {
	width = width_;
	height = height_;
	if (!Initialize(window, width, height, windowName)) {
		throw std::exception();
	}
}

bool Window::ShouldClose() const {
	WINDOW_VALIDITY_CHECK();
	return glfwWindowShouldClose(window);
}

void Window::Close() {
	WINDOW_VALIDITY_CHECK();
	glfwSetWindowShouldClose(window, true);
}

void Window::SwapBuffers() {
	WINDOW_VALIDITY_CHECK();
	glfwSwapBuffers(window);
	glfwPollEvents();
}

Window::Window() {
	LOG(LOG_CTOR, "[C] Window\n");
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
	LOG(LOG_DTOR, "[D] Window\n");
}

//===================

bool Initialize(GLFWwindow*& window, int width, int height, const char* windowName) {
	//glfw initialization
	if (!glfwInit()) {
		LOG(LOG_ERROR, "GLFW - Failed to initialize.\n");
		return false;
	}

	//window hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef GL_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	//context & window creation
	window = glfwCreateWindow(width, height, windowName, NULL, NULL);
	if (!window) {
		LOG(LOG_ERROR, "GLFW - Failed to create a window.\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	//glad initialization (OpenGL functions loading)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG(LOG_ERROR, "Glad - Failed to initialize OpenGL context.\n");
		return false;
	}

	//gl debug callback setup
#ifdef GL_DEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugCallback, nullptr);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	}
#endif

	//print general OpenGL info
	LOG(LOG_INFO, "OpenGL %s, %s (%s)\n", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));
	LOG(LOG_INFO, "GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	//viewport setup
	glViewport(0, 0, width, height);

	return true;
}

#undef WINDOW_VALIDITY_CHECK
