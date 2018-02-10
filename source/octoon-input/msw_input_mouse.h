#ifndef OCTOON_INPUT_MSW_MOUSE_H_
#define OCTOON_INPUT_MSW_MOUSE_H_

#include <octoon/input/input_mouse.h>

namespace octoon
{
	namespace input
	{
		class OCTOON_EXPORT MSWInputMouse final : public DefaultInputMouse
		{
			OctoonDeclareSubClass(MSWInputMouse, DefaultInputMouse)
		public:
			MSWInputMouse() noexcept;
			MSWInputMouse(WindHandle window) noexcept;
			~MSWInputMouse() noexcept;

			InputMousePtr clone() const noexcept;

			void getPosition(InputButton::mouse_t& x, InputButton::mouse_t& y) const noexcept;

			bool isButtonDown(InputButton::Code key) const noexcept;
			bool isButtonUp(InputButton::Code key) const noexcept;
			bool isButtonPressed(InputButton::Code key) const noexcept;

		private:
			void onShowMouse() noexcept;
			void onHideMouse() noexcept;

			void onChangePosition(InputButton::mouse_t x, InputButton::mouse_t y) noexcept;

			void onInputEvent(const InputEvent& event) noexcept;

		private:
			MSWInputMouse(const MSWInputMouse&) = delete;
			MSWInputMouse& operator=(const MSWInputMouse&) = delete;

		private:
			bool focusWindow_;
			WindHandle window_;
		};
	}
}

#endif