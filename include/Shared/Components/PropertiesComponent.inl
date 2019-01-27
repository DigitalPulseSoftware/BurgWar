// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PropertiesComponent.hpp>

namespace bw
{
	inline PropertiesComponent::PropertiesComponent(Properties properties) :
	m_properties(std::move(properties))
	{
	}

	inline std::optional<std::reference_wrapper<const EntityProperty>> PropertiesComponent::GetProperty(const std::string& keyName) const
	{
		auto it = m_properties.find(keyName);
		if (it == m_properties.end())
			return std::nullopt;

		return it->second;
	}
}

