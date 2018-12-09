// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SHAREDENTITYSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_SHAREDENTITYSTORE_HPP

#include <Shared/Scripting/ScriptedEntity.hpp>
#include <Shared/Scripting/ScriptStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class SharedEntityStore : public ScriptStore<ScriptedEntity>
	{
		public:
			SharedEntityStore(std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context, bool isServer);
			~SharedEntityStore() = default;

		protected:
			void InitializeElementTable(Nz::LuaState& state) override = 0;
			void InitializeElement(Nz::LuaState& state, ScriptedEntity& element) override = 0;
			bool InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity);
	};
}

#include <Shared/Scripting/SharedEntityStore.inl>

#endif
