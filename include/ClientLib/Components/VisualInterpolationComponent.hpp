// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_VISUALINTERPOLATIONCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_VISUALINTERPOLATIONCOMPONENT_HPP

#include <ClientLib/LocalLayerEntity.hpp>
#include <Nazara/Math/Angle.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class VisualInterpolationComponent : public Ndk::Component<VisualInterpolationComponent>
	{
		friend class VisualInterpolationSystem;

		public:
			inline VisualInterpolationComponent();
			~VisualInterpolationComponent() = default;

			static Ndk::ComponentIndex componentIndex;

		private:
			inline const Nz::Vector2f& GetLastPosition();
			inline const Nz::RadianAnglef& GetLastRotation();

			inline void UpdateLastStates(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation);

			Nz::RadianAnglef m_lastRotation;
			Nz::Vector2f m_lastPosition;
	};
}

#include <ClientLib/Components/VisualInterpolationComponent.inl>

#endif
