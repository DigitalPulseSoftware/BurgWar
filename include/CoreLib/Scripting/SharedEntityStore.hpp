// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYSTORE_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <CoreLib/Scripting/ScriptStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API SharedEntityStore : public ScriptStore<ScriptedEntity>
	{
		public:
			SharedEntityStore(const Logger& logger, std::shared_ptr<ScriptingContext> context, bool isServer);
			~SharedEntityStore() = default;

		protected:
			virtual void BindCallbacks(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity) const;
			void InitializeElement(sol::main_table& elementTable, ScriptedEntity& element) override = 0;
			bool InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity) const;
	};
}

#include <CoreLib/Scripting/SharedEntityStore.inl>

#endif
