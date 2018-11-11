// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_CLIENTENTITYSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_CLIENTENTITYSTORE_HPP

#include <Shared/Scripting/ScriptedEntity.hpp>
#include <Shared/Scripting/ScriptStore.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class ClientEntityStore : public ScriptStore<ScriptedEntity, false>
	{
		public:
			ClientEntityStore(Nz::LuaState& state);
			~ClientEntityStore() = default;

			const Ndk::EntityHandle& BuildEntity(Ndk::World& world, std::size_t entityIndex);

		private:
			void InitializeElementTable(Nz::LuaState& state) override;
			void InitializeElement(Nz::LuaState& state, ScriptedEntity& entity) override;
	};
}

#include <Client/Scripting/ClientEntityStore.hpp>

#endif
