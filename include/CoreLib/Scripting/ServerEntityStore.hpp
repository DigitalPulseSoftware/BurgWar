// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERENTITYSTORE_HPP

#include <Nazara/Math/Angle.hpp>
#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <CoreLib/Scripting/SharedEntityStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class ServerEntityStore : public SharedEntityStore
	{
		public:
			inline ServerEntityStore(std::shared_ptr<SharedScriptingContext> context);
			~ServerEntityStore() = default;

			const Ndk::EntityHandle& InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties);

		private:
			void InitializeElementTable(sol::table& elementTable) override;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override;
	};
}

#include <CoreLib/Scripting/ServerEntityStore.inl>

#endif
