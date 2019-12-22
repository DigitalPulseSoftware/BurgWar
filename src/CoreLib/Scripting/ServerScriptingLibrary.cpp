// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Player.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>

namespace bw
{
	namespace
	{
		struct ParameterError
		{
			const char* msg;
		};
	}

	ServerScriptingLibrary::ServerScriptingLibrary(Match& match) :
	SharedScriptingLibrary(match)
	{
	}

	void ServerScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		RegisterPlayerClass(context);

		context.Load("autorun");
	}

	void ServerScriptingLibrary::RegisterGlobalLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterGlobalLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = false;
		state["SERVER"] = true;

		state["RegisterClientAssets"] = [&](sol::this_state L, const sol::object& paths) -> std::pair<bool, sol::object>
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
				throw std::runtime_error(err.msg);
			}
		};

		state["RegisterClientScript"] = [&](sol::this_state L, const std::optional<std::string_view>& path) -> std::pair<bool, sol::object>
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
		};
	}

	void ServerScriptingLibrary::RegisterMatchLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterMatchLibrary(context, library);

		library["CreateEntity"] = [&](const sol::table& parameters)
		{
			Match& match = GetMatch();
			auto& entityStore = match.GetEntityStore();

			sol::object entityTypeObj = parameters["Type"];
			if (!entityTypeObj.is<std::string>())
				throw std::runtime_error("Missing or invalid value for LayerIndex");

			std::string entityType = entityTypeObj.as<std::string>();

			std::size_t elementIndex = entityStore.GetElementIndex(entityType);
			if (elementIndex == ServerEntityStore::InvalidIndex)
				throw std::runtime_error("Entity type \"" + entityType + "\" doesn't exist");

			sol::object layerIndexObj = parameters["LayerIndex"];
			if (!layerIndexObj.is<LayerIndex>())
				throw std::runtime_error("Missing or invalid value for LayerIndex");

			LayerIndex layerIndex = layerIndexObj.as<LayerIndex>();

			if (layerIndex > match.GetLayerCount())
				throw std::runtime_error("Layer out of range (" + std::to_string(layerIndex) + " > " + std::to_string(match.GetLayerCount()) + ")");

			PlayerHandle owner = parameters.get_or<PlayerHandle>("Owner", PlayerHandle::InvalidHandle);

			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			Nz::Vector2f position = parameters.get_or("Position", Nz::Vector2f::Zero());

			EntityProperties entityProperties;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Properties", std::nullopt); propertyTableOpt)
			{
				sol::table& propertyTable = propertyTableOpt.value();

				const auto& entityPtr = entityStore.GetElement(elementIndex);
				for (auto&& [propertyName, propertyData] : entityPtr->properties)
				{
					sol::object propertyValue = propertyTable[propertyName];
					if (propertyValue)
						entityProperties.emplace(propertyName, TranslateEntityPropertyFromLua(&match, propertyValue, propertyData.type, propertyData.isArray));
				}
			}

			std::reference_wrapper<const Ndk::EntityHandle> parentEntity = Ndk::EntityHandle::InvalidHandle;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Parent", std::nullopt); propertyTableOpt)
				parentEntity = AbstractElementLibrary::AssertScriptEntity(propertyTableOpt.value());

			Nz::Int64 uniqueId = match.AllocateUniqueId();

			const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(match.GetLayer(layerIndex), elementIndex, uniqueId, position, rotation, entityProperties, parentEntity);
			if (!entity)
				throw std::runtime_error("Failed to create \"" + entityType + "\"");

			if (owner)
				entity->AddComponent<OwnerComponent>(std::move(owner));

			match.RegisterEntity(uniqueId, entity);

			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			return scriptComponent.GetTable();
		};

		library["GetLocalTick"] = [&]()
		{
			return GetMatch().GetCurrentTick();
		};

		library["GetTick"] = [&]()
		{
			return GetMatch().GetCurrentTick();
		};
	}

	void ServerScriptingLibrary::RegisterPlayerClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<PlayerHandle>("Player", 
			"new", sol::no_constructor,
			"GetControlledEntity", [](const PlayerHandle& player) -> sol::object
			{
				if (!player)
					return sol::nil;

				const Ndk::EntityHandle& controlledEntity = player->GetControlledEntity();
				if (!controlledEntity)
					return sol::nil;

				auto& scriptComponent = controlledEntity->GetComponent<ScriptComponent>();
				return scriptComponent.GetTable();
			},
			"GetLayerIndex", [](const PlayerHandle& player) -> LayerIndex
			{
				if (!player)
					return 0;

				return player->GetLayerIndex();
			},
			"GetName", [](const PlayerHandle& player) -> std::string
			{
				if (!player)
					return "<Disconnected>";

				return player->GetName();
			},
			"GiveWeapon", [](const PlayerHandle& player, std::string weaponName)
			{
				if (!player)
					return false;

				return player->GiveWeapon(std::move(weaponName));
			},
			"HasWeapon", [](const PlayerHandle& player, const std::string& weaponName)
			{
				if (!player)
					return false;

				return player->HasWeapon(weaponName);
			},
			"IsAdmin", [](const PlayerHandle& player)
			{
				if (!player)
					return false;

				return player->IsAdmin();
			},
			"MoveToLayer", [](const PlayerHandle& player, sol::object layerIndex)
			{
				if (!player)
					return;
			
				if (layerIndex != sol::nil)
					player->MoveToLayer(layerIndex.as<LayerIndex>());
				else
					player->MoveToLayer(Player::NoLayer);
			},
			"PrintChatMessage", [](const PlayerHandle& player, std::string message)
			{
				if (!player)
					return;

				player->PrintChatMessage(std::move(message));
			},
			"RemoveWeapon", [](const PlayerHandle& player, const std::string& weaponName)
			{
				if (!player)
					return;

				return player->RemoveWeapon(weaponName);
			},
			"Spawn", [](const PlayerHandle& player)
			{
				if (!player)
					return;

				return player->Spawn();
			},
			"SetAdmin", [](const PlayerHandle& player, bool isAdmin)
			{
				if (!player)
					return;

				return player->SetAdmin(isAdmin);
			},
			"UpdateLayerVisibility", [](const PlayerHandle& player, LayerIndex layerIndex, bool visible)
			{
				if (!player)
					return;

				player->UpdateLayerVisibility(layerIndex, visible);
			}
		);
	}

	void ServerScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterScriptLibrary(context, library);

		library["ReloadAll"] = [this]()
		{
			Match& match = GetMatch();
			match.ReloadScripts();
		};
	}

	Match& ServerScriptingLibrary::GetMatch()
	{
		return static_cast<Match&>(GetSharedMatch());
	}
}
