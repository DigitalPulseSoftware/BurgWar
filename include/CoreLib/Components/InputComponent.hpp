// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_INPUTCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_INPUTCOMPONENT_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <array>

namespace bw
{
	class InputComponent : public Ndk::Component<InputComponent>
	{
		public:
			inline InputComponent();

			inline const PlayerInputData& GetInputs() const;
			inline const PlayerInputData& GetPreviousInputs() const;

			inline void UpdateInputs(const PlayerInputData& inputData);
			inline void UpdatePreviousInputs(const PlayerInputData& inputData);

			static Ndk::ComponentIndex componentIndex;

			NazaraSignal(OnInputUpdate, InputComponent* /*input*/);

		private:
			std::array<PlayerInputData, 2> m_inputData;
			std::size_t m_inputIndex;
	};
}

#include <CoreLib/Components/InputComponent.inl>

#endif
