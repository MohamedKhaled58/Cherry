#include "CHpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <GL/gl.h>
namespace Cherry
{

	inline const char* gl_to_cstr(const GLubyte* s)
	{
		return reinterpret_cast<const char*>(s);
	}


	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		CH_CORE_ASSERT(windowHandle, "Window handle is null!");
		CH_CORE_INFO("Creating OpenGL context for window: {0}", (void*)windowHandle);
	}
	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		CH_CORE_ASSERT(status, "Failed to initialize OpenGLContext!");

		CH_CORE_INFO("OpenGL Vendor   : {}", gl_to_cstr(glGetString(GL_VENDOR)));
		CH_CORE_INFO("OpenGL Renderer : {}", gl_to_cstr(glGetString(GL_RENDERER)));
		CH_CORE_INFO("OpenGL Version  : {}", gl_to_cstr(glGetString(GL_VERSION)));
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}