// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_BACKGROUNDSTATE_HPP
#define BURGWAR_STATES_BACKGROUNDSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <CoreLib/EntityOwner.hpp>

namespace bw
{
	class BackgroundState final : public AbstractState
	{
		public:
			using AbstractState::AbstractState;
			~BackgroundState() = default;

		private:
			void Enter(Nz::StateMachine& fsm) override;
			void Leave(Nz::StateMachine& fsm) override;
			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

			void LayoutWidgets() override;

			EntityOwner m_spriteEntity;
			std::shared_ptr<Nz::Sprite> m_backgroundSprite;
	};
}

#include <Client/States/BackgroundState.inl>

#endif
