// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_INPUTCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_INPUTCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/InputController.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <CoreLib/Components/BaseComponent.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <array>
#include <memory>

namespace bw
{
	class BURGWAR_CORELIB_API InputComponent : public BaseComponent
	{
		public:
			inline InputComponent(entt::handle handle, std::shared_ptr<InputController> inputController);

			inline const std::shared_ptr<InputController>& GetController() const;
			inline const PlayerInputData& GetInputs() const;
			inline const PlayerInputData& GetPreviousInputs() const;

			inline void UpdateController(std::shared_ptr<InputController> inputController);
			inline void UpdateInputs(const PlayerInputData& inputData);
			inline void UpdatePreviousInputs(const PlayerInputData& inputData);

			NazaraSignal(OnInputUpdate, InputComponent* /*input*/);

		private:
			std::array<PlayerInputData, 2> m_inputData;
			std::size_t m_inputIndex;
			std::shared_ptr<InputController> m_inputController;
	};
}

#include <CoreLib/Components/InputComponent.inl>

#endif
