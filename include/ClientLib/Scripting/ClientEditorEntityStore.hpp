// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTEDITORENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTEDITORENTITYSTORE_HPP

#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <CoreLib/Scripting/SharedEntityStore.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class AssetStore;
	class LocalLayer;

	class ClientEditorEntityStore : public SharedEntityStore
	{
		public:
			inline ClientEditorEntityStore(AssetStore& assetStore, const Logger& logger, std::shared_ptr<ScriptingContext> context);
			ClientEditorEntityStore(ClientEditorEntityStore&&) = delete;
			~ClientEditorEntityStore() = default;

			bool InitializeEntity(const Ndk::EntityHandle& entity) const;
			virtual const Ndk::EntityHandle& InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const;

		protected:
			void InitializeElementTable(sol::table& elementTable) override;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override;
			using SharedEntityStore::InitializeEntity;

			AssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientEditorEntityStore.inl>

#endif
