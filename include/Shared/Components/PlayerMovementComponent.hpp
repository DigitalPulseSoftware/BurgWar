// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_PLAYERMOVEMENT_HPP
#define BURGWAR_SHARED_COMPONENTS_PLAYERMOVEMENT_HPP

#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class PlayerMovementComponent : public Ndk::Component<PlayerMovementComponent>
	{
		public:
			PlayerMovementComponent();
			~PlayerMovementComponent() = default;

			inline bool IsAirControlling() const;
			inline bool IsFacingRight() const;

			inline bool UpdateAirControlState(bool isAirControlling);
			inline bool UpdateFacingRightState(bool isFacingRight);

			static Ndk::ComponentIndex componentIndex;

		private:
			bool m_isFacingRight;
			bool m_isAirControlling;
	};
}

#include <Shared/Components/PlayerMovementComponent.inl>

#endif
