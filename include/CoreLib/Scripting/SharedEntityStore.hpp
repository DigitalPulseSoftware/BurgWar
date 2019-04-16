// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYSTORE_HPP

#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <CoreLib/Scripting/ScriptStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class SharedEntityStore : public ScriptStore<ScriptedEntity>
	{
		public:
			SharedEntityStore(std::shared_ptr<ScriptingContext> context, bool isServer);
			~SharedEntityStore() = default;

		protected:
			void InitializeElementTable(sol::table& elementTable) override = 0;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override = 0;
			bool InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity) const;
	};
}

#include <CoreLib/Scripting/SharedEntityStore.inl>

#endif
