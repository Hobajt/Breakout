#pragma once

struct GLFWwindow;

typedef void (*ResizeCallbackType)(int width, int height);

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

	void Resize(int width, int height);

	int Width() const { return width; }
	int Height() const { return height; }
	float AspectRatio() const { return float(width) / float(height); }

	void SetResizeCallback(ResizeCallbackType ResizeCallback_) { ResizeCallback = ResizeCallback_; }
private:
	Window();
	~Window();

	//copy disabled
	Window(const Window&) = delete;
	Window& operator = (const Window&) = delete;

	//move disabled
	Window(Window&&) = delete;
	Window& operator= (Window&&) = delete;
public:
	static int sampleCount;
private:
	int width = 640;
	int height = 480;

	GLFWwindow* window = nullptr;
	ResizeCallbackType ResizeCallback = nullptr;
};
