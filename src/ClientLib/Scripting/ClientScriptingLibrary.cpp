// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/DummyInputController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
#include <ClientLib/Scripting/Texture.hpp>

namespace bw
{
	ClientScriptingLibrary::ClientScriptingLibrary(LocalMatch& match) :
	SharedScriptingLibrary(match)
	{
	}

	void ClientScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		RegisterDummyInputControllerClass(context);
		RegisterGlobalLibrary(context);
		RegisterSpriteClass(context);

		context.Load("autorun");
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
			BW_INPUT_PROPERTY(isJumping, bool),
			BW_INPUT_PROPERTY(isMovingLeft, bool),
			BW_INPUT_PROPERTY(isMovingRight, bool)
		);

#undef BW_INPUT_PROPERTY
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

			EntityProperties entityProperties;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Properties", std::nullopt); propertyTableOpt)
			{
				sol::table& propertyTable = propertyTableOpt.value();

				const auto& entityPtr = entityStore.GetElement(elementIndex);
				for (auto&& [propertyName, propertyData] : entityPtr->properties)
				{
					sol::object propertyValue = propertyTable[propertyName];
					if (propertyValue)
						entityProperties.emplace(propertyName, TranslateEntityPropertyFromLua(propertyValue, propertyData.type, propertyData.isArray));
				}
			}

			Ndk::EntityHandle parentEntity;
			if (std::optional<sol::table> propertyTableOpt = parameters.get_or<std::optional<sol::table>>("Parent", std::nullopt); propertyTableOpt)
				parentEntity = AbstractElementLibrary::AssertScriptEntity(propertyTableOpt.value());

			LocalLayer& layer = match.GetLayer(layerIndex);
			auto entityOpt = entityStore.InstantiateEntity(layer, elementIndex, LocalLayerEntity::ClientsideId, position, rotation, entityProperties, parentEntity);
			if (!entityOpt)
				throw std::runtime_error("Failed to create \"" + entityType + "\"");

			const Ndk::EntityHandle& entity = layer.RegisterEntity(std::move(entityOpt.value())).GetEntity();

			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			return scriptComponent.GetTable();
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

	void ClientScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context, sol::table& library)
	{
		SharedScriptingLibrary::RegisterScriptLibrary(context, library);

		library["ReloadAll"] = [this]()
		{
			throw std::runtime_error("Only the server can reload scripts");
		};
	}

	void ClientScriptingLibrary::RegisterSpriteClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Sprite>("Sprite",
			"new", sol::no_constructor,

			"GetOrigin", &Sprite::GetOrigin,
			"GetSize", &Sprite::GetSize,

			"Hide", &Sprite::Hide,

			"SetSize", &Sprite::SetSize,

			"Show", sol::overload(&Sprite::Show, [](Sprite* sprite) { return sprite->Show(); })
		);
	}

	LocalMatch& ClientScriptingLibrary::GetMatch()
	{
		return static_cast<LocalMatch&>(GetSharedMatch());
	}
}
