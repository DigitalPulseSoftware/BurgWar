// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <ClientLib/Camera.hpp>
#include <ClientLib/DummyInputController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Scoreboard.hpp>
#include <ClientLib/Scripting/ParticleGroup.hpp>
#include <ClientLib/Scripting/Music.hpp>
#include <ClientLib/Scripting/Sound.hpp>
#include <ClientLib/Scripting/Texture.hpp>
#include <Nazara/Graphics/ParticleFunctionRenderer.hpp>
#include <NDK/Components/ParticleGroupComponent.hpp>

namespace bw
{
	ClientScriptingLibrary::ClientScriptingLibrary(LocalMatch& match) :
	SharedScriptingLibrary(match)
	{
	}

	void ClientScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		sol::state& luaState = context.GetLuaState();

		RegisterCameraClass(context);
		RegisterDummyInputControllerClass(context);
		RegisterGlobalLibrary(context);
		RegisterLocalPlayerClass(context);
		RegisterMusicClass(context);
		RegisterParticleGroupClass(context);
		RegisterScoreboardClass(context);
		RegisterSoundClass(context);

		sol::table particleTable = luaState.create_named_table("particle");
		sol::table soundTable = luaState.create_named_table("sound");

		RegisterParticleLibrary(context, particleTable);
		RegisterSoundLibrary(context, soundTable);

		context.LoadDirectory("autorun");
	}

	void ClientScriptingLibrary::RegisterGlobalLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterGlobalLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = true;
		state["SERVER"] = false;

		state["RegisterClientAssets"] = []() { return true; }; // Dummy function
		state["RegisterClientScript"] = []() { return true; }; // Dummy function
	}

	void ClientScriptingLibrary::RegisterMatchLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterMatchLibrary(context, library);

		library["CreateEntity"] = [&](sol::this_state L, const sol::table& parameters)
		{
			LocalMatch& match = GetMatch();
			auto& entityStore = match.GetEntityStore();

			sol::object entityTypeObj = parameters["Type"];
			if (!entityTypeObj.is<std::string>())
				TriggerLuaArgError(L, 1, "missing or invalid value for LayerIndex");

			std::string entityType = entityTypeObj.as<std::string>();

			std::size_t elementIndex = entityStore.GetElementIndex(entityType);
			if (elementIndex == ClientEntityStore::InvalidIndex)
				TriggerLuaArgError(L, 1, "entity type \"" + entityType + "\" doesn't exist");

			sol::object layerIndexObj = parameters["LayerIndex"];
			if (!layerIndexObj.is<LayerIndex>())
				TriggerLuaArgError(L, 1, "missing or invalid value for LayerIndex");

			LayerIndex layerIndex = layerIndexObj.as<LayerIndex>();
			if (layerIndex >= match.GetLayerCount())
				TriggerLuaArgError(L, 1, "layer out of range (" + std::to_string(layerIndex) + " > " + std::to_string(match.GetLayerCount()) + ")");

			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			Nz::Vector2f position = parameters.get_or("Position", Nz::Vector2f::Zero());
			float scale = parameters.get_or("Scale", 1.f);

			Ndk::EntityHandle lifeOwner;
			if (std::optional<sol::table> lifeOwnerEntitytable = parameters.get_or<std::optional<sol::table>>("LifeOwner", std::nullopt); lifeOwnerEntitytable)
				lifeOwner = AssertScriptEntity(*lifeOwnerEntitytable);

			const auto& entityPtr = entityStore.GetElement(elementIndex);

			PropertyValueMap entityProperties;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Properties", std::nullopt); propertyTableOpt)
			{
				sol::table& propertyTable = propertyTableOpt.value();

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

			EntityId clientUniqueId = match.AllocateClientUniqueId();

			try
			{
				LocalLayer& layer = match.GetLayer(layerIndex);
				auto entityOpt = entityStore.InstantiateEntity(layer, elementIndex, LocalLayerEntity::ClientsideId, clientUniqueId, position, rotation, scale, entityProperties, parentEntity);
				if (!entityOpt)
					TriggerLuaError(L, "failed to create \"" + entityType + "\"");

				const Ndk::EntityHandle& entity = layer.RegisterEntity(std::move(entityOpt.value())).GetEntity();

				if (lifeOwner)
				{
					if (!lifeOwner->HasComponent<EntityOwnerComponent>())
						lifeOwner->AddComponent<EntityOwnerComponent>();

					lifeOwner->GetComponent<EntityOwnerComponent>().Register(entity);
				}

				auto& scriptComponent = entity->GetComponent<ScriptComponent>();
				return scriptComponent.GetTable();
			}
			catch (const std::exception& e)
			{
				TriggerLuaError(L, e.what());
			}
		};

		library["GetCamera"] = [&]() -> CameraHandle
		{
			LocalMatch& match = GetMatch();
			return match.GetCamera().CreateHandle();
		};

		library["GetPlayers"] = [&](sol::this_state L) -> sol::table
		{
			sol::state_view lua(L);

			LocalMatch& match = GetMatch();

			sol::table playerTable = lua.create_table();

			std::size_t index = 1;
			match.ForEachPlayer([&](LocalPlayer& localPlayer)
			{
				playerTable[index++] = localPlayer.CreateHandle();
			});

			return playerTable;
		};

		library["GetLocalTick"] = [&]()
		{
			return GetMatch().GetCurrentTick();
		};

		library["GetTick"] = [&]()
		{
			return GetMatch().AdjustServerTick(GetMatch().EstimateServerTick());
		};
	}

	void ClientScriptingLibrary::RegisterParticleLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["CreateGroup"] = [&](sol::this_state L, unsigned int maxParticleCount, const std::string& particleType)
		{
			LocalMatch& match = GetMatch();

			const ParticleRegistry& registry = match.GetParticleRegistry();
			const auto& layout = registry.GetLayout(particleType);
			if (!layout)
				TriggerLuaArgError(L, 2, "Invalid particle type \"" + particleType + "\"");

			Ndk::World& world = match.GetRenderWorld();
			const Ndk::EntityHandle& particleGroupEntity = world.CreateEntity();
			auto& particleGroup = particleGroupEntity->AddComponent<Ndk::ParticleGroupComponent>(maxParticleCount, layout);

			particleGroup.SetRenderer(Nz::ParticleFunctionRenderer::New([](const Nz::ParticleGroup& /*group*/, const Nz::ParticleMapper& /*mapper*/, unsigned int /*startId*/, unsigned int /*endId*/, Nz::AbstractRenderQueue* /*renderQueue*/)
			{
				// Do nothing
			}));

			return ParticleGroup(registry, particleGroupEntity);
		};
	}

	void ClientScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterScriptLibrary(context, library);

		library["ReloadAll"] = [](sol::this_state L)
		{
			TriggerLuaError(L, "only the server can reload scripts");
		};
	}

	void ClientScriptingLibrary::RegisterSoundLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["CreateMusicFromFile"] = [this](sol::this_state L, const std::string& musicPath) -> sol::object
		{
			LocalMatch& match = GetMatch();
			const auto& assetDirectory = match.GetAssetStore().GetAssetDirectory();

			VirtualDirectory::Entry entry;
			if (!assetDirectory->GetEntry(musicPath, &entry))
				return sol::make_object(L, std::make_pair(sol::nil, "file not found"));

			Nz::Music music;

			bool loaded = std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
				{
					bwLog(m_logger, LogLevel::Info, "Loading asset from memory");
					return music.OpenFromMemory(arg.data(), arg.size());
				}
				else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
				{
					bwLog(m_logger, LogLevel::Info, "Loading asset from {}", arg.generic_u8string());
					return music.OpenFromFile(arg.generic_u8string());
				}
				else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
				{
					return false;
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
			}, entry);

			if (!loaded)
				return sol::make_object(L, std::make_pair(sol::nil, "failed to open music"));

			return sol::make_object(L, Music(match.GetApplication(), std::move(music)));
		};
	}
	
	void ClientScriptingLibrary::RegisterCameraClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Camera>("Camera",
			"new", sol::no_constructor,

			"GetFOV",         ExceptToLuaErr(&Camera::GetFOV),
			"GetPosition",    ExceptToLuaErr(&Camera::GetPosition),
			"GetZoomFactor",  ExceptToLuaErr(&Camera::GetZoomFactor),
			"GetViewport",    ExceptToLuaErr(&Camera::GetViewport),

			"MoveBy",         ExceptToLuaErr(&Camera::MoveBy),
			"MoveToPosition", ExceptToLuaErr(&Camera::MoveToPosition),

			"SetFOV",         ExceptToLuaErr(&Camera::SetFOV),
			"SetZoomFactor",  ExceptToLuaErr(&Camera::SetZoomFactor),

			"Project",   sol::overload(
				ExceptToLuaErr(Overload<const Nz::Vector2f&>(&Camera::Project)),
				ExceptToLuaErr(Overload<const Nz::Vector3f&>(&Camera::Project))
			),

			"Unproject", sol::overload(
				ExceptToLuaErr(Overload<const Nz::Vector2f&>(&Camera::Unproject)),
				ExceptToLuaErr(Overload<const Nz::Vector3f&>(&Camera::Unproject))
			)
		);
	}

	void ClientScriptingLibrary::RegisterDummyInputControllerClass(ScriptingContext& context)
	{
#define BW_INPUT_PROPERTY(name, type) #name, sol::property( \
		ExceptToLuaErr([](DummyInputController& input) { return input.GetInputs(). name ; }), \
		ExceptToLuaErr([](DummyInputController& input, const type& newValue) { input.GetInputs(). name = newValue; }))

		sol::state& state = context.GetLuaState();
		state.new_usertype<InputController>("InputController");

		state.new_usertype<DummyInputController>("DummyInputController",
			sol::base_classes, sol::bases<InputController>(),
			"new", sol::factories(ExceptToLuaErr(&std::make_shared<DummyInputController>)),

			BW_INPUT_PROPERTY(aimDirection, Nz::Vector2f),
			BW_INPUT_PROPERTY(isAttacking, bool),
			BW_INPUT_PROPERTY(isCrouching, bool),
			BW_INPUT_PROPERTY(isLookingRight, bool),
			BW_INPUT_PROPERTY(isJumping, bool),
			BW_INPUT_PROPERTY(isMovingLeft, bool),
			BW_INPUT_PROPERTY(isMovingRight, bool)
		);
#undef BW_INPUT_PROPERTY
	}

	void ClientScriptingLibrary::RegisterLocalPlayerClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<LocalPlayer>("LocalPlayer",
			"new", sol::no_constructor,

			"GetName", ExceptToLuaErr(&LocalPlayer::GetName),

			"GetPing", ExceptToLuaErr([](sol::this_state L, LocalPlayer& localPlayer) -> sol::object
			{
				if (Nz::UInt16 ping = localPlayer.GetPing(); ping != LocalPlayer::InvalidPing)
					return sol::make_object(L, ping);
				else
					return sol::nil;
			}),

			"GetPlayerIndex", ExceptToLuaErr(&LocalPlayer::GetPlayerIndex)
		);
	}

	void ClientScriptingLibrary::RegisterMusicClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Music>("Music",
			"new", sol::no_constructor,

			"EnableLooping", ExceptToLuaErr(&Music::EnableLooping),

			"GetDuration",      ExceptToLuaErr(&Music::GetDuration),
			"GetPlayingOffset", ExceptToLuaErr(&Music::GetPlayingOffset),
			"GetSampleCount",   ExceptToLuaErr(&Music::GetSampleCount),
			"GetSampleRate",    ExceptToLuaErr(&Music::GetSampleRate),

			"IsLooping", ExceptToLuaErr(&Music::IsLooping),
			"IsPlaying", ExceptToLuaErr(&Music::IsPlaying),

			"Pause", ExceptToLuaErr(&Music::Pause),
			"Play",  ExceptToLuaErr(&Music::Play),

			"SetPlayingOffset", ExceptToLuaErr(&Music::SetPlayingOffset),

			"Stop", ExceptToLuaErr(&Music::Stop)
		);
	}

	void ClientScriptingLibrary::RegisterParticleGroupClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		sol::table emptyTable = state.create_table();

		state.new_usertype<ParticleGroup>("ParticleGroup",
			"new", sol::no_constructor,

			"AddController", sol::overload(
				ExceptToLuaErr([=](ParticleGroup& group, const std::string& name) { group.AddController(name, emptyTable); }),
				ExceptToLuaErr(&ParticleGroup::AddController)),

			"AddGenerator", sol::overload(
				ExceptToLuaErr([=](ParticleGroup& group, const std::string& name) { group.AddGenerator(name, emptyTable); }),
				ExceptToLuaErr(&ParticleGroup::AddGenerator)),

			"GenerateParticles", ExceptToLuaErr(&ParticleGroup::GenerateParticles),

			"GetParticleCount", ExceptToLuaErr(&ParticleGroup::GetParticleCount),

			"Kill", ExceptToLuaErr(&ParticleGroup::Kill),

			"SetRenderer", sol::overload(
				ExceptToLuaErr([=](ParticleGroup& group, const std::string& name) { group.SetRenderer(name, emptyTable); }),
				ExceptToLuaErr(&ParticleGroup::SetRenderer))
		);
	}

	void ClientScriptingLibrary::RegisterScoreboardClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Scoreboard>("Scoreboard",
			"new", sol::no_constructor,

			"AddColumn", ExceptToLuaErr(&Scoreboard::AddColumn),
			"AddTeam", ExceptToLuaErr(&Scoreboard::AddTeam),

			"RegisterPlayer", ExceptToLuaErr(&Scoreboard::RegisterPlayer),

			"UnregisterPlayer", ExceptToLuaErr(&Scoreboard::UnregisterPlayer),

			"UpdatePlayerValue", ExceptToLuaErr(&Scoreboard::UpdatePlayerValue)
		);
	}

	void ClientScriptingLibrary::RegisterSoundClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Sound>("Sound",
			"new", sol::no_constructor,

			"Stop", ExceptToLuaErr(&Sound::Stop)
		);
	}

	LocalMatch& ClientScriptingLibrary::GetMatch()
	{
		return static_cast<LocalMatch&>(GetSharedMatch());
	}
}
