#pragma once
#include <Cherry/Input.h>

namespace Cherry {
	class  WindowsInput : public Input {
	public:


	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
		virtual std::pair<float, float> GetMousePositionImp() override;
	};
} // namespace Cherry