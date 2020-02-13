// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <ClientLib/DummyInputController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Scoreboard.hpp>
#include <ClientLib/Scripting/ParticleGroup.hpp>
#include <ClientLib/Scripting/Sound.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
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

		RegisterDummyInputControllerClass(context);
		RegisterGlobalLibrary(context);
		RegisterLocalPlayerClass(context);
		RegisterParticleGroupClass(context);
		RegisterScoreboardClass(context);
		RegisterSoundClass(context);
		RegisterSpriteClass(context);

		sol::table particleTable = luaState.create_named_table("particle");
		RegisterParticleLibrary(context, particleTable);

		context.Load("autorun");
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

		library["CreateEntity"] = [&](const sol::table& parameters)
		{
			LocalMatch& match = GetMatch();
			auto& entityStore = match.GetEntityStore();

			sol::object entityTypeObj = parameters["Type"];
			if (!entityTypeObj.is<std::string>())
				throw std::runtime_error("Missing or invalid value for LayerIndex");

			std::string entityType = entityTypeObj.as<std::string>();

			std::size_t elementIndex = entityStore.GetElementIndex(entityType);
			if (elementIndex == ClientEntityStore::InvalidIndex)
				throw std::runtime_error("Entity type \"" + entityType + "\" doesn't exist");

			sol::object layerIndexObj = parameters["LayerIndex"];
			if (!layerIndexObj.is<LayerIndex>())
				throw std::runtime_error("Missing or invalid value for LayerIndex");

			LayerIndex layerIndex = layerIndexObj.as<LayerIndex>();

			if (layerIndex > match.GetLayerCount())
				throw std::runtime_error("Layer out of range (" + std::to_string(layerIndex) + " > " + std::to_string(match.GetLayerCount()) + ")");

			Nz::DegreeAnglef rotation = parameters.get_or("Rotation", Nz::DegreeAnglef::Zero());
			Nz::Vector2f position = parameters.get_or("Position", Nz::Vector2f::Zero());

			Ndk::EntityHandle lifeOwner;
			if (std::optional<sol::table> lifeOwnerEntitytable = parameters.get_or<std::optional<sol::table>>("LifeOwner", std::nullopt); lifeOwnerEntitytable)
				lifeOwner = AbstractElementLibrary::AssertScriptEntity(*lifeOwnerEntitytable);

			const auto& entityPtr = entityStore.GetElement(elementIndex);

			EntityProperties entityProperties;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Properties", std::nullopt); propertyTableOpt)
			{
				sol::table& propertyTable = propertyTableOpt.value();

				for (auto&& [propertyName, propertyData] : entityPtr->properties)
				{
					sol::object propertyValue = propertyTable[propertyName];
					if (propertyValue)
						entityProperties.emplace(propertyName, TranslateEntityPropertyFromLua(&match, propertyValue, propertyData.type, propertyData.isArray));
				}
			}

			Ndk::EntityHandle parentEntity;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Parent", std::nullopt); propertyTableOpt)
				parentEntity = AbstractElementLibrary::AssertScriptEntity(propertyTableOpt.value());

			Nz::Int64 clientUniqueId = match.AllocateClientUniqueId();

			LocalLayer& layer = match.GetLayer(layerIndex);
			auto entityOpt = entityStore.InstantiateEntity(layer, elementIndex, LocalLayerEntity::ClientsideId, clientUniqueId, position, rotation, entityProperties, parentEntity);
			if (!entityOpt)
				throw std::runtime_error("Failed to create \"" + entityType + "\"");

			const Ndk::EntityHandle& entity = layer.RegisterEntity(std::move(entityOpt.value())).GetEntity();

			if (lifeOwner)
			{
				if (!lifeOwner->HasComponent<EntityOwnerComponent>())
					lifeOwner->AddComponent<EntityOwnerComponent>();

				lifeOwner->GetComponent<EntityOwnerComponent>().Register(entity);
			}

			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			return scriptComponent.GetTable();
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
		library["CreateGroup"] = [&](unsigned int maxParticleCount, const std::string& particleType)
		{
			LocalMatch& match = GetMatch();

			const ParticleRegistry& registry = match.GetParticleRegistry();
			const auto& layout = registry.GetLayout(particleType);
			if (!layout)
				throw std::runtime_error("Invalid particle type \"" + particleType + "\"");

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

		library["ReloadAll"] = [this]()
		{
			throw std::runtime_error("Only the server can reload scripts");
		};
	}
	
	void ClientScriptingLibrary::RegisterDummyInputControllerClass(ScriptingContext& context)
	{
#define BW_INPUT_PROPERTY(name, type) #name, sol::property( \
		[](DummyInputController& input) { return input.GetInputs(). name ; }, \
		[](DummyInputController& input, const type& newValue) { input.GetInputs(). name = newValue; })

		sol::state& state = context.GetLuaState();
		state.new_usertype<InputController>("InputController");

		state.new_usertype<DummyInputController>("DummyInputController",
			sol::base_classes, sol::bases<InputController>(),
			"new", sol::factories(&std::make_shared<DummyInputController>),

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

			"GetName", &LocalPlayer::GetName,

			"GetPing", [](sol::this_state L, LocalPlayer& localPlayer) -> sol::object
			{
				if (Nz::UInt16 ping = localPlayer.GetPing(); ping != LocalPlayer::InvalidPing)
					return sol::make_object(L, ping);
				else
					return sol::nil;
			},

			"GetPlayerIndex", &LocalPlayer::GetPlayerIndex
		);
	}

	void ClientScriptingLibrary::RegisterParticleGroupClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		sol::table emptyTable = state.create_table();

		state.new_usertype<ParticleGroup>("ParticleGroup",
			"new", sol::no_constructor,

			"AddController", sol::overload(
				[=](ParticleGroup& group, const std::string& name) { group.AddController(name, emptyTable); },
				&ParticleGroup::AddController),

			"AddGenerator", sol::overload(
				[=](ParticleGroup& group, const std::string& name) { group.AddGenerator(name, emptyTable); },
				&ParticleGroup::AddGenerator),

			"GenerateParticles", &ParticleGroup::GenerateParticles,

			"GetParticleCount", &ParticleGroup::GetParticleCount,

			"Kill", &ParticleGroup::Kill,

			"SetRenderer", sol::overload(
				[=](ParticleGroup& group, const std::string& name) { group.SetRenderer(name, emptyTable); },
				&ParticleGroup::SetRenderer)
		);
	}

	void ClientScriptingLibrary::RegisterScoreboardClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Scoreboard>("Scoreboard",
			"new", sol::no_constructor,

			"AddColumn", &Scoreboard::AddColumn,
			"AddTeam", &Scoreboard::AddTeam,

			"RegisterPlayer", &Scoreboard::RegisterPlayer,

			"UnregisterPlayer", &Scoreboard::UnregisterPlayer
		);
	}

	void ClientScriptingLibrary::RegisterSoundClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Sound>("Sound",
			"new", sol::no_constructor,

			"Stop", &Sound::Stop
		);
	}

	void ClientScriptingLibrary::RegisterSpriteClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Sprite>("Sprite",
			"new", sol::no_constructor,

			"GetOrigin", &Sprite::GetOrigin,
			"GetSize", &Sprite::GetSize,

			"Hide", &Sprite::Hide,

			"SetOffset", &Sprite::SetOffset,
			"SetRotation", &Sprite::SetRotation,
			"SetSize", &Sprite::SetSize,

			"Show", sol::overload(&Sprite::Show, [](Sprite* sprite) { return sprite->Show(); })
		);
	}

	LocalMatch& ClientScriptingLibrary::GetMatch()
	{
		return static_cast<LocalMatch&>(GetSharedMatch());
	}
}
