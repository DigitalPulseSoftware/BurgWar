// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_VISUALINTERPOLATIONSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_VISUALINTERPOLATIONSYSTEM_HPP

#include <NDK/System.hpp>

namespace bw
{
	class VisualInterpolationSystem : public Ndk::System<VisualInterpolationSystem>
	{
		public:
			VisualInterpolationSystem();
			~VisualInterpolationSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;
	};
}

#include <ClientLib/Systems/VisualInterpolationSystem.inl>

#endif
