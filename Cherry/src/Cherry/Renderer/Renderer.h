#pragma once

namespace Cherry {

	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};

	class Renderer 
	{
	public:
		inline static RendererAPI GetAPI() { return s_RendererAPI; }
		inline static RendererAPI SetAPI() { return s_RendererAPI; }
	private:
		static RendererAPI s_RendererAPI;
	};

}