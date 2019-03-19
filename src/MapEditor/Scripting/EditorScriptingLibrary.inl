// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorScriptingLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <sol2/sol.hpp>
#include <cassert>
#include <type_traits>
#include <vector>

namespace bw
{
}

namespace sol
{
	template<>
	struct lua_size<bw::TileMapEditorMode::TileMapData> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<bw::TileMapEditorMode::TileMapData> : std::integral_constant<sol::type, sol::type::table> {};

	namespace stack
	{
		template <>
		struct getter<std::vector<bw::TileMapEditorMode::TileData>>
		{
			static std::vector<bw::TileMapEditorMode::TileData> get(lua_State* L, int index, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);

				std::vector<bw::TileMapEditorMode::TileData> tileVec;

				sol::table tileTable(L, absoluteIndex);
				std::size_t tileCount = tileTable.size();

				tileVec.resize(tileCount);
				for (std::size_t i = 0; i < tileCount; ++i)
				{
					auto& tileData = tileVec[i];

					lua_geti(L, absoluteIndex, i + 1);
					{
						sol::stack_table tileTable(L);

						std::string materialPath = tileTable["material"];

						tileData.material = Nz::MaterialManager::Get(materialPath);
						if (!tileData.material)
							tileData.material = Nz::Material::GetDefault();

						tileData.texCoords = tileTable["texCoords"];
					}
					lua_pop(L, 1);
				}

				tracking.use(1);

				return tileVec;
			}
		};

		template <>
		struct getter<bw::TileMapEditorMode::TileMapData>
		{
			static bw::TileMapEditorMode::TileMapData get(lua_State* L, int index, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);

				bw::TileMapEditorMode::TileMapData tileMapData;

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
		};

		template <>
		struct pusher<bw::TileMapEditorMode::TileMapData>
		{
			static int push(lua_State* L, const bw::TileMapEditorMode::TileMapData& tileMapData)
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
		};

	}
}