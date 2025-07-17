#include "CHpch.h"
#include "Cherry/Renderer/RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Cherry {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}