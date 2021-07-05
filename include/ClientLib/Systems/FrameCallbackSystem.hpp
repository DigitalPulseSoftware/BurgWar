// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_FRAMECALLBACKSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_FRAMECALLBACKSYSTEM_HPP

#include <ClientLib/Export.hpp>
#include <NDK/EntityList.hpp>
#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class ClientMatch;

	class BURGWAR_CLIENTLIB_API FrameCallbackSystem : public Ndk::System<FrameCallbackSystem>
	{
		public:
			FrameCallbackSystem();
			~FrameCallbackSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_frameUpdateEntities;
	};
}

#include <ClientLib/Systems/FrameCallbackSystem.inl>

#endif
