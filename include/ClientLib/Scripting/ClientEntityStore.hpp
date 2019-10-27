// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTENTITYSTORE_HPP

#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <ClientLib/Scripting/ClientEditorEntityStore.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>

namespace bw
{
	class AssetStore;
	class LocalLayer;

	class ClientEntityStore : public ClientEditorEntityStore
	{
		public:
			using ClientEditorEntityStore::ClientEditorEntityStore;
			ClientEntityStore(ClientEntityStore&&) = delete;
			~ClientEntityStore() = default;

			const Ndk::EntityHandle& InstantiateEntity(LocalLayer& layer, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const;
	};
}

#include <ClientLib/Scripting/ClientEntityStore.inl>

#endif
