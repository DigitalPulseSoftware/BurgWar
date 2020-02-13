// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_TICKCALLBACKSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_TICKCALLBACKSYSTEM_HPP

#include <NDK/EntityList.hpp>
#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class SharedMatch;

	class TickCallbackSystem : public Ndk::System<TickCallbackSystem>
	{
		public:
			TickCallbackSystem(SharedMatch& match);
			~TickCallbackSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_tickableEntities;
			SharedMatch& m_match;
	};
}

#include <CoreLib/Systems/TickCallbackSystem.inl>

#endif
