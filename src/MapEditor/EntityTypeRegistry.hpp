// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_ENTITYTYPEREGISTRY_HPP
#define BURGWAR_MAPEDITOR_ENTITYTYPEREGISTRY_HPP

#include <Common/EntityProperties.hpp>
#include <Nazara/Prerequisites.hpp>
#include <hopscotch/hopscotch_map.h>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace bw
{
	class EntityTypeRegistry
	{
		public:
			struct Property;
			struct EntityType;

			inline EntityTypeRegistry(std::filesystem::path entityTypeFolder);
			inline ~EntityTypeRegistry();

			inline std::optional<std::size_t> FindTypeIndex(const std::string& typeName) const;

			inline const EntityType& GetType(std::size_t typeIndex) const;
			inline std::size_t GetTypeCount() const;

			void Reload();

			static inline EntityTypeRegistry* Instance();

			struct Property
			{
				std::string keyName;
				std::string visualName;
				PropertyType type;
			};

			struct EntityType
			{
				std::string name;
				std::string description;
				std::vector<Property> properties;
			};

		private:
			std::filesystem::path m_entityTypeFolder;
			std::vector<EntityType> m_entityTypes;
			tsl::hopscotch_map<std::string /*type*/, std::size_t /*typeIndex*/> m_entityTypeByName;

			static EntityTypeRegistry* s_instance;
	};
}

#include <MapEditor/EntityTypeRegistry.inl>

#endif
