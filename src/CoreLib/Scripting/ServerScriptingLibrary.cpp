// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Scripting/ServerTexture.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Scripting/SharedElementLibrary.hpp>

namespace bw
{
	namespace
	{
		struct ParameterError
		{
			const char* msg;
		};
	}

	ServerScriptingLibrary::ServerScriptingLibrary(Match& match, AssetStore& assetStore) :
	SharedScriptingLibrary(match),
	m_assetStore(assetStore)
	{
	}

	void ServerScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		sol::state& luaState = context.GetLuaState();
		sol::table assetTable = luaState.create_named_table("assets");

		RegisterAssetLibrary(context, assetTable);
		RegisterPlayerClass(context);
		RegisterServerTextureClass(context);

		context.LoadDirectory("autorun");
	}

	void ServerScriptingLibrary::RegisterAssetLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["GetTexture"] = LuaFunction([this](const std::string& texturePath) -> std::optional<ServerTexture>
		{
			const Nz::ImageRef& image = m_assetStore.GetImage(texturePath);
			if (image)
				return ServerTexture(image);
			else
				return {};
		});
	}

	void ServerScriptingLibrary::RegisterGlobalLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterGlobalLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = false;
		state["SERVER"] = true;

		state["RegisterClientAssets"] = LuaFunction([&](sol::this_state L, const sol::object& paths) -> std::pair<bool, sol::object>
		{
			try
			{
				if (paths.is<sol::table>())
				{
					sol::table pathTable = paths.as<sol::table>();
					for (auto&& [k, v] : pathTable)
					{
						if (v.is<std::string>())
							GetMatch().RegisterAsset(v.as<std::string>());
					}
				}
				else if (paths.is<std::string>())
				{
					GetMatch().RegisterAsset(paths.as<std::string>());
				}
				else
					throw ParameterError{ "expected table or string" };

				return { true, sol::nil };
			}
			catch (const std::exception& err)
			{
				bwLog(GetMatch().GetLogger(), LogLevel::Warning, "RegisterClientAssets failed: {}", err.what());
				return { false, sol::make_object<std::string>(L, err.what()) };
			}
			catch (const ParameterError& err)
			{
				TriggerLuaError(L, err.msg);
			}
		});

		state["RegisterClientScript"] = LuaFunction([&](sol::this_state L, const sol::optional<std::string_view>& path) -> std::pair<bool, sol::object>
		{
			try
			{
				if (path)
					GetMatch().RegisterClientScript(context.GetCurrentFolder() / *path);
				else
				{
					const auto& currentFilepath = context.GetCurrentFile();
					if (currentFilepath.empty())
						throw ParameterError{ "RegisterClientScript cannot be called without argument outside of a file" };

					GetMatch().RegisterClientScript(currentFilepath);
				}

				return { true, sol::nil };
			}
			catch (const std::exception& err)
			{
				bwLog(GetMatch().GetLogger(), LogLevel::Warning, "RegisterClientScript failed: {}", err.what());
				return { false, sol::make_object<std::string>(L, err.what()) };
			}
			catch (const ParameterError& err)
			{
				throw std::runtime_error(err.msg);
			}
		});
	}

	void ServerScriptingLibrary::RegisterMatchLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterMatchLibrary(context, library);

		library["BroadcastPacket"] = LuaFunction([&](const OutgoingNetworkPacket& outgoingPacket)
		{
			Match& match = GetMatch();

			const NetworkStringStore& networkStringStore = match.GetNetworkStringStore();
			match.BroadcastPacket(outgoingPacket.ToPacket(networkStringStore));
		});

		library["CreateEntity"] = LuaFunction([&](sol::this_state L, const sol::table& parameters)
		{
			Match& match = GetMatch();
			auto& entityStore = match.GetEntityStore();

			sol::object entityTypeObj = parameters["Type"];
			if (!entityTypeObj.is<std::string>())
				TriggerLuaArgError(L, 1, "missing or invalid value for LayerIndex");

			std::string entityType = entityTypeObj.as<std::string>();

			std::size_t elementIndex = entityStore.GetElementIndex(entityType);
			if (elementIndex == ServerEntityStore::InvalidIndex)
				TriggerLuaArgError(L, 1, "entity type \"" + entityType + "\" doesn't exist");

			sol::object layerIndexObj = parameters["LayerIndex"];
			if (!layerIndexObj.is<LayerIndex>())
				TriggerLuaArgError(L, 1, "missing or invalid value for LayerIndex");

			LayerIndex layerIndex = layerIndexObj.as<LayerIndex>();
			if (layerIndex >= match.GetLayerCount())
				TriggerLuaArgError(L, 1, "layer out of range (" + std::to_string(layerIndex) + " > " + std::to_string(match.GetLayerCount()) + ")");

			PlayerHandle owner = parameters.get_or<PlayerHandle>("Owner", PlayerHandle::InvalidHandle);

			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			Nz::Vector2f position = parameters.get_or("Position", Nz::Vector2f::Zero());

			Ndk::EntityHandle lifeOwner;
			if (std::optional<sol::table> lifeOwnerEntitytable = parameters.get_or<std::optional<sol::table>>("LifeOwner", std::nullopt); lifeOwnerEntitytable)
				lifeOwner = AssertScriptEntity(*lifeOwnerEntitytable);

			PropertyValueMap entityProperties;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Properties", std::nullopt); propertyTableOpt)
			{
				sol::table& propertyTable = propertyTableOpt.value();

				const auto& entityPtr = entityStore.GetElement(elementIndex);
				for (auto&& [propertyName, propertyData] : entityPtr->properties)
				{
					sol::object propertyValue = propertyTable[propertyName];
					if (propertyValue)
						entityProperties.emplace(propertyName, TranslatePropertyFromLua(&match, propertyValue, propertyData.type, propertyData.isArray));
				}
			}

			Ndk::EntityHandle parentEntity;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Parent", std::nullopt); propertyTableOpt)
				parentEntity = AssertScriptEntity(propertyTableOpt.value());

			EntityId uniqueId = match.AllocateUniqueId();

			const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(match.GetLayer(layerIndex), elementIndex, uniqueId, position, rotation, entityProperties, parentEntity);
			if (!entity)
				TriggerLuaError(L, "failed to create \"" + entityType + "\"");

			if (owner)
				entity->AddComponent<OwnerComponent>(std::move(owner));

			match.RegisterEntity(uniqueId, entity);

			if (lifeOwner)
			{
				if (!lifeOwner->HasComponent<EntityOwnerComponent>())
					lifeOwner->AddComponent<EntityOwnerComponent>();

				lifeOwner->GetComponent<EntityOwnerComponent>().Register(entity);
			}

			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			return scriptComponent.GetTable();
		});

		library["CreateWeapon"] = LuaFunction([&](sol::this_state L, const sol::table& parameters)
		{
			Match& match = GetMatch();
			auto& weaponStore = match.GetWeaponStore();

			sol::object entityTypeObj = parameters["Type"];
			if (!entityTypeObj.is<std::string>())
				TriggerLuaArgError(L, 1, "missing or invalid value for LayerIndex");

			std::string entityType = entityTypeObj.as<std::string>();

			std::size_t elementIndex = weaponStore.GetElementIndex(entityType);
			if (elementIndex == ServerEntityStore::InvalidIndex)
				TriggerLuaArgError(L, 1, "entity type \"" + entityType + "\" doesn't exist");

			PropertyValueMap entityProperties;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Properties", std::nullopt); propertyTableOpt)
			{
				sol::table& propertyTable = propertyTableOpt.value();

				const auto& entityPtr = weaponStore.GetElement(elementIndex);
				for (auto&& [propertyName, propertyData] : entityPtr->properties)
				{
					sol::object propertyValue = propertyTable[propertyName];
					if (propertyValue)
						entityProperties.emplace(propertyName, TranslatePropertyFromLua(&match, propertyValue, propertyData.type, propertyData.isArray));
				}
			}

			Ndk::EntityHandle owner = AssertScriptEntity(parameters["Owner"]);
			auto& ownerMatchComponent = owner->GetComponent<MatchComponent>();

			LayerIndex layerIndex = ownerMatchComponent.GetLayerIndex();
			auto& layer = match.GetTerrain().GetLayer(layerIndex);

			// Create weapon
			EntityId uniqueId = match.AllocateUniqueId();

			const Ndk::EntityHandle& weapon = weaponStore.InstantiateWeapon(layer, elementIndex, uniqueId, std::move(entityProperties), owner);
			if (!weapon)
				TriggerLuaError(L, "failed to create \"" + entityType + "\"");

			weapon->GetComponent<WeaponComponent>().SetActive(true);

			match.RegisterEntity(uniqueId, weapon);

			if (owner->HasComponent<OwnerComponent>())
			{
				if (Player* ownerPlayer = owner->GetComponent<OwnerComponent>().GetOwner())
					weapon->AddComponent<OwnerComponent>(ownerPlayer->CreateHandle());
			}

			auto& scriptComponent = weapon->GetComponent<ScriptComponent>();
			return scriptComponent.GetTable();
		});

		library["GetLocalTick"] = LuaFunction([&]()
		{
			return GetMatch().GetCurrentTick();
		});
		
		library["GetPlayers"] = LuaFunction([&](sol::this_state L) -> sol::table
		{
			sol::state_view lua(L);

			Match& match = GetMatch();

			sol::table playerTable = lua.create_table();

			std::size_t index = 1;
			match.ForEachPlayer([&](Player* player)
			{
				playerTable[index++] = player->CreateHandle();
			});

			return playerTable;
		});

		library["GetTick"] = LuaFunction([&]()
		{
			return GetMatch().GetCurrentTick();
		});
	}

	void ServerScriptingLibrary::RegisterNetworkLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterNetworkLibrary(context, library);

		library["RegisterPacket"] = LuaFunction([&](std::string packetName)
		{
			GetMatch().RegisterNetworkString(std::move(packetName));
		});
	}

	void ServerScriptingLibrary::RegisterPlayerClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<PlayerHandle>("Player", 
			"new", sol::no_constructor,
			"GetControlledEntity", LuaFunction([](const Player& player) -> sol::object
			{
				const Ndk::EntityHandle& controlledEntity = player.GetControlledEntity();
				if (!controlledEntity)
					return sol::nil;

				auto& scriptComponent = controlledEntity->GetComponent<ScriptComponent>();
				return scriptComponent.GetTable();
			}),
			"GetLayerIndex", LuaFunction(&Player::GetLayerIndex),
			"GetPlayerIndex", LuaFunction(&Player::GetPlayerIndex),
			"GetName", LuaFunction(&Player::GetName),
			"IsAdmin", LuaFunction(&Player::IsAdmin),
			"MoveToLayer", LuaFunction([](Player& player, std::optional<LayerIndex> layerIndex)
			{
				if (layerIndex)
					player.MoveToLayer(layerIndex.value());
				else
					player.MoveToLayer(Player::NoLayer);
			}),
			"PrintChatMessage", LuaFunction(&Player::PrintChatMessage),
			"SendPacket", LuaFunction([this](Player& player, const OutgoingNetworkPacket& outgoingPacket)
			{
				const NetworkStringStore& networkStringStore = GetSharedMatch().GetNetworkStringStore();
				player.SendPacket(outgoingPacket.ToPacket(networkStringStore));
			}),
			"SetAdmin", LuaFunction(&Player::SetAdmin),
			"UpdateControlledEntity", LuaFunction([](Player& player, sol::optional<sol::table> entityTable)
			{
				if (entityTable)
				{
					const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable.value());

					player.UpdateControlledEntity(entity);
				}
				else
					player.UpdateControlledEntity(Ndk::EntityHandle::InvalidHandle);
			}),
			"UpdateLayerVisibility", LuaFunction(&Player::UpdateLayerVisibility)
		);
	}

	void ServerScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterScriptLibrary(context, library);

		library["ReloadAll"] = LuaFunction([this]()
		{
			Match& match = GetMatch();
			match.ReloadScripts();
		});
	}

	void ServerScriptingLibrary::RegisterServerTextureClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<ServerTexture>("Texture",
			"new", sol::no_constructor,

			"GetSize", LuaFunction(&ServerTexture::GetSize)
		);
	}

	Match& ServerScriptingLibrary::GetMatch()
	{
		return static_cast<Match&>(GetSharedMatch());
	}
}
