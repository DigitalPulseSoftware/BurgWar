// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_CLIENTENTITYSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_CLIENTENTITYSTORE_HPP

#include <Shared/Scripting/ScriptedEntity.hpp>
#include <Shared/Scripting/SharedEntityStore.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class ClientEntityStore : public SharedEntityStore
	{
		public:
			inline ClientEntityStore(std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context);
			~ClientEntityStore() = default;

			const Ndk::EntityHandle& InstantiateEntity(Ndk::World& world, std::size_t entityIndex);

		private:
			void InitializeElementTable(sol::table& elementTable) override = 0;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override = 0;
	};
}

#include <Client/Scripting/ClientEntityStore.inl>

#endif
