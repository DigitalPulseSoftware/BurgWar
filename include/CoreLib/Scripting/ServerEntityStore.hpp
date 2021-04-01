// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERENTITYSTORE_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <CoreLib/Scripting/SharedEntityStore.hpp>
#include <Nazara/Math/Angle.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class TerrainLayer;

	class BURGWAR_CORELIB_API ServerEntityStore : public SharedEntityStore
	{
		public:
			inline ServerEntityStore(const Logger& logger, std::shared_ptr<ScriptingContext> context);
			~ServerEntityStore() = default;

			const Ndk::EntityHandle& CreateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, const Ndk::EntityHandle& parent = Ndk::EntityHandle::InvalidHandle) const;
			bool InitializeEntity(const Ndk::EntityHandle& entity) const;
			const Ndk::EntityHandle& InstantiateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, const Ndk::EntityHandle& parent = Ndk::EntityHandle::InvalidHandle) const;

		private:
			void InitializeElementTable(sol::main_table& elementTable) override;
			void InitializeElement(sol::main_table& elementTable, ScriptedEntity& element) override;
	};
}

#include <CoreLib/Scripting/ServerEntityStore.inl>

#endif
