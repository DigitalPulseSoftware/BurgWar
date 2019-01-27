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
			inline ClientEntityStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context);
			~ClientEntityStore() = default;

			const Ndk::EntityHandle& InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const EntityProperties& properties);

		private:
			void InitializeElementTable(sol::table& elementTable) override;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override;
	};
}

#include <Client/Scripting/ClientEntityStore.inl>

#endif
