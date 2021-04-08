// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/Utility/TileMapData.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <sol/sol.hpp>
#include <cassert>
#include <type_traits>
#include <vector>

namespace bw
{
	class InputController;
	class DummyInputController;
}

SOL_BASE_CLASSES(bw::DummyInputController, bw::InputController);
SOL_DERIVED_CLASSES(bw::InputController, bw::DummyInputController);

namespace sol
{
	template<>
	struct lua_size<std::vector<bw::TileData>> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<std::vector<bw::TileData>> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_size<std::vector<bw::TileMaterialData>> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<std::vector<bw::TileMaterialData>> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_size<bw::TileMapData> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<bw::TileMapData> : std::integral_constant<sol::type, sol::type::table> {};

	inline std::vector<bw::TileData> sol_lua_get(sol::types<std::vector<bw::TileData>>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		std::vector<bw::TileData> tileVec;

		sol::table tileTable(L, absoluteIndex);
		std::size_t tileCount = tileTable.size();

		tileVec.resize(tileCount);
		for (std::size_t i = 0; i < tileCount; ++i)
		{
			auto& tileData = tileVec[i];

			lua_geti(L, absoluteIndex, i + 1);
			{
				sol::stack_table tileDataTable(L);

				tileData.materialPath = tileDataTable["materialPath"];
				tileData.texCoords = tileDataTable["texCoords"];
			}
			lua_pop(L, 1);
		}

		tracking.use(1);

		return tileVec;
	}

	inline std::vector<bw::TileMaterialData> sol_lua_get(sol::types<std::vector<bw::TileMaterialData>>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		std::vector<bw::TileMaterialData> materials;

		sol::table materialTable(L, absoluteIndex);
		std::size_t materialCount = materialTable.size();

		materials.resize(materialCount);
		for (std::size_t i = 0; i < materialCount; ++i)
		{
			auto& materialData = materials[i];

			lua_geti(L, absoluteIndex, i + 1);
			{
				sol::stack_table materialDataTable(L);

				materialData.group = materialDataTable["group"];
				materialData.path = materialDataTable["path"];
				materialData.tileCount = materialDataTable["tileCount"];
			}
			lua_pop(L, 1);
		}

		tracking.use(1);

		return materials;
	}

	inline bw::TileMapData sol_lua_get(sol::types<bw::TileMapData>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		bw::TileMapData tileMapData;

		sol::table tileMapTable(L, absoluteIndex);
		tileMapData.mapSize = tileMapTable["mapSize"];
		tileMapData.origin = tileMapTable["origin"];
		tileMapData.rotation = tileMapTable["rotation"];
		tileMapData.tileSize = tileMapTable["tileSize"];

		sol::table content = tileMapTable["content"];
		std::size_t contentSize = content.size();

		tileMapData.content.resize(contentSize);
		for (std::size_t i = 0; i < contentSize; ++i)
			tileMapData.content[i] = content[i + 1];

		tracking.use(1);

		return tileMapData;
	}

	inline int sol_lua_push(sol::types<bw::TileMapData>, lua_State* L, const bw::TileMapData& tileMapData)
	{
		lua_createtable(L, 0, 5);
		sol::stack_table vec(L);
		vec["mapSize"] = tileMapData.mapSize;
		vec["origin"] = tileMapData.origin;
		vec["rotation"] = tileMapData.rotation;
		vec["tileSize"] = tileMapData.tileSize;

		sol::state_view state(L);

		sol::table content = state.create_table(tileMapData.content.size());

		std::size_t index = 1;
		for (Nz::UInt32 tileType : tileMapData.content)
			content[index++] = tileType;

		vec["content"] = content;

		return 1;
	}
}
