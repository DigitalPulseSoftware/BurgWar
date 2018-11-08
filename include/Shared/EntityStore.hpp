// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_ENTITYSTORE_HPP
#define BURGWAR_SHARED_ENTITYSTORE_HPP

#include <Nazara/Lua/LuaState.hpp>
#include <hopstotch/hopscotch_map.h>
#include <limits>
#include <vector>

namespace bw
{
	class EntityStore
	{
		public:
			struct Entity;

			inline EntityStore(bool isServer, Nz::LuaState& state);
			~EntityStore() = default;

			template<typename F> void ForEachEntity(const F& func);

			inline const Entity& GetEntity(std::size_t index) const;
			inline std::size_t GetEntityIndex(const std::string& name) const;

			bool Load(const std::string& folder);

			struct Entity
			{
				bool isNetworked;
				int tableRef;
				std::string name;
			};

			static constexpr std::size_t InvalidIndex = std::numeric_limits<std::size_t>::max();

		private:
			std::vector<Entity> m_entities;
			tsl::hopscotch_map<std::string /*name*/, std::size_t /*entityIndex*/> m_entitiesByName;
			Nz::LuaState& m_state;
			bool m_isServer;
	};
}

#include <Shared/EntityStore.inl>

#endif
