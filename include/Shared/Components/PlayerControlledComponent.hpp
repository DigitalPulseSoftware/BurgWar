// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_PLAYERCONTROLLED_HPP
#define BURGWAR_SHARED_COMPONENTS_PLAYERCONTROLLED_HPP

#include <NDK/Component.hpp>
#include <Shared/Player.hpp>
#include <vector>

namespace bw
{
	class PlayerControlledComponent : public Ndk::Component<PlayerControlledComponent>
	{
		friend class PlayerControlledSystem;
		friend class TerrainLayer;

		public:
			PlayerControlledComponent(PlayerHandle owner);
			~PlayerControlledComponent() = default;

			inline Player* GetOwner() const;

			inline bool IsJumping() const;
			inline bool IsMovingLeft() const;
			inline bool IsMovingRight() const;
			inline bool IsOnGround() const;

			inline void UpdateJumpingState(bool isJumping);
			inline void UpdateMovingLeftState(bool isMovingLeft);
			inline void UpdateMovingRightState(bool isMovingRight);

			static Ndk::ComponentIndex componentIndex;

		private:
			inline bool UpdateGroundState(bool isOnGround);

			PlayerHandle m_owner;
			bool m_isJumping;
			bool m_isMovingLeft;
			bool m_isMovingRight;
			bool m_isOnGround;
	};
}

#include <Shared/Components/PlayerControlledComponent.inl>

#endif
