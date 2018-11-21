// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP
#define BURGWAR_CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP

#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class BurgApp;

	class AnimationSystem : public Ndk::System<AnimationSystem>
	{
		public:
			AnimationSystem(BurgApp& app);
			~AnimationSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;

			BurgApp& m_application;
	};
}

#include <Shared/Systems/AnimationSystem.inl>

#endif
