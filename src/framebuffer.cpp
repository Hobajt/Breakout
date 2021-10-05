#include "breakout/framebuffer.h"

#include "breakout/log.h"
#include "breakout/window.h"

#include "breakout/texture.h"
#include <memory>

#define HANDLE_CHECK() ASSERT_MSG(handle != 0, "\tAttempting to use uninitialized framebuffer.\n")

Framebuffer::Framebuffer(int width_, int height_, GLenum internalFormat_, const TextureParams& tParams) : width(width_), height(height_), internalFormat(internalFormat_) {
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);

	texture = std::make_shared<Texture>(width, height, internalFormat, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, tParams);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->Handle(), 0);

	if (!IsComplete()) {
		throw std::exception();
	}

	LOG(LOG_CTOR, "[C] Framebuffer %d\n", handle);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(int newWidth, int newHeight) {
	width = newWidth;
	height = newHeight;

	texture->Resize(width, height);

	if (!IsComplete()) {
		throw std::exception();
	}
}

void Framebuffer::Bind() const {
	HANDLE_CHECK();
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::IsComplete() const {
	Bind();

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		const char* statusText = "OTHER ERROR";
		switch (status) {
			case GL_FRAMEBUFFER_UNDEFINED:						statusText = "GL_FRAMEBUFFER_UNDEFINED "; break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			statusText = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT "; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	statusText = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT "; break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			statusText = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER "; break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			statusText = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER "; break;
			case GL_FRAMEBUFFER_UNSUPPORTED:					statusText = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			statusText = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE "; break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:		statusText = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS "; break;
		}

		LOG(LOG_ERROR, "Framebuffer - completeness check failed (status = '%s').\n", statusText);
		return false;
	}
	return true;
}

#undef HANDLE_CHECK
