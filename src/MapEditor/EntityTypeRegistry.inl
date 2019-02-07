// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Nazara Development Kit Qt Layer"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <MapEditor/EntityTypeRegistry.hpp>
#include <cassert>

namespace bw
{
	inline EntityTypeRegistry::EntityTypeRegistry(std::filesystem::path entityTypeFolder) :
	m_entityTypeFolder(std::move(entityTypeFolder))
	{
		assert(!s_instance);
		s_instance = this;

		Reload();
	}

	inline EntityTypeRegistry::~EntityTypeRegistry()
	{
		assert(s_instance);
		s_instance = nullptr;
	}

	inline std::optional<std::size_t> EntityTypeRegistry::FindTypeIndex(const std::string& typeName) const
	{
		auto it = m_entityTypeByName.find(typeName);
		if (it == m_entityTypeByName.end())
			return std::nullopt;

		return it->second;
	}

	inline auto EntityTypeRegistry::GetType(std::size_t typeIndex) const -> const EntityType&
	{
		assert(typeIndex < m_entityTypes.size());

		return m_entityTypes[typeIndex];
	}

	inline std::size_t EntityTypeRegistry::GetTypeCount() const
	{
		return m_entityTypes.size();
	}

	inline EntityTypeRegistry* EntityTypeRegistry::Instance()
	{
		return s_instance;
	}
}
