// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTEDITORENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTEDITORENTITYSTORE_HPP

#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <CoreLib/Scripting/SharedEntityStore.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class ClientAssetStore;
	class LocalLayer;

	class BURGWAR_CLIENTLIB_API ClientEditorEntityStore : public SharedEntityStore
	{
		public:
			inline ClientEditorEntityStore(ClientAssetStore& assetStore, const Logger& logger, std::shared_ptr<ScriptingContext> context);
			ClientEditorEntityStore(ClientEditorEntityStore&&) = delete;
			~ClientEditorEntityStore() = default;

			bool InitializeEntity(const Ndk::EntityHandle& entity) const;
			virtual const Ndk::EntityHandle& InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, PropertyValueMap properties, const Ndk::EntityHandle& parentEntity = Ndk::EntityHandle::InvalidHandle) const;

		protected:
			void InitializeElementTable(sol::main_table& elementTable) override;
			void InitializeElement(sol::main_table& elementTable, ScriptedEntity& element) override;
			using SharedEntityStore::InitializeEntity;

			ClientAssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientEditorEntityStore.inl>

#endif
