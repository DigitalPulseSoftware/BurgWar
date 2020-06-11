// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_BACKGROUNDSTATE_HPP
#define BURGWAR_STATES_BACKGROUNDSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/EntityOwner.hpp>

namespace bw
{
	class BackgroundState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~BackgroundState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			Ndk::EntityOwner m_spriteEntity;
			Nz::SpriteRef m_backgroundSprite;
	};
}

#include <Client/States/BackgroundState.inl>

#endif
