// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SYSTEMS_TICKCALLBACKSYSTEM_HPP
#define BURGWAR_SHARED_SYSTEMS_TICKCALLBACKSYSTEM_HPP

#include <NDK/EntityList.hpp>
#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class TickCallbackSystem : public Ndk::System<TickCallbackSystem>
	{
		public:
			TickCallbackSystem();
			~TickCallbackSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_tickableEntities;
	};
}

#include <Shared/Systems/TickCallbackSystem.inl>

#endif
