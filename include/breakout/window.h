#pragma once

struct GLFWwindow;

//Manages window properties & operations. Singleton pattern.
class Window {
public:
	static Window& Get();
public:
	void Init(int width, int height, const char* windowName);
	bool IsInitialized() const { return (window != nullptr); }

	bool ShouldClose() const;
	void Close();
	void SwapBuffers();

	GLFWwindow* Handle() { return window; }
private:
	Window();
	~Window();

	//copy disabled
	Window(const Window&) = delete;
	Window& operator = (const Window&) = delete;

	//move disabled
	Window(Window&&) = delete;
	Window& operator= (Window&&) = delete;
private:
	int width = 640;
	int height = 480;

	GLFWwindow* window = nullptr;
};
