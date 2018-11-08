// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/EntityStore.hpp>
#include <cassert>

namespace bw
{
	inline EntityStore::EntityStore(bool isServer, Nz::LuaState& state) :
	m_state(state),
	m_isServer(isServer)
	{
	}

	template<typename F>
	void EntityStore::ForEachEntity(const F& func)
	{
		for (const auto& entity : m_entities)
			func(entity);
	}
	
	inline auto EntityStore::GetEntity(std::size_t index) const -> const Entity&
	{
		assert(index < m_entities.size());
		return m_entities[index];
	}

	inline std::size_t EntityStore::GetEntityIndex(const std::string& name) const
	{
		auto it = m_entitiesByName.find(name);
		if (it == m_entitiesByName.end())
			return InvalidIndex;

		return it->second;
	}
}
