// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_POSTFRAMECALLBACKSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_POSTFRAMECALLBACKSYSTEM_HPP

#include <NDK/EntityList.hpp>
#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class PostFrameCallbackSystem : public Ndk::System<PostFrameCallbackSystem>
	{
		public:
			PostFrameCallbackSystem();
			~PostFrameCallbackSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_frameUpdateEntities;
	};
}

#include <ClientLib/Systems/PostFrameCallbackSystem.inl>

#endif
