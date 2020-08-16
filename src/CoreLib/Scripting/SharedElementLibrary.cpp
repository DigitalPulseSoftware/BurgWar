// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <NDK/Components/LifetimeComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Thirdparty/sol3/sol.hpp>

namespace bw
{
	SharedElementLibrary::~SharedElementLibrary() = default;

	void SharedElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterCommonLibrary(elementMetatable);
	}

	void SharedElementLibrary::RegisterCommonLibrary(sol::table& elementMetatable)
	{
		elementMetatable["DeleteOnRemove"] = [](const sol::table& entityTable, const sol::table& targetEntityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			const Ndk::EntityHandle& targetEntity = AssertScriptEntity(targetEntityTable);

			if (!entity->HasComponent<EntityOwnerComponent>())
				entity->AddComponent<EntityOwnerComponent>();

			entity->GetComponent<EntityOwnerComponent>().Register(targetEntity);
		};

		elementMetatable["Disable"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			entity->Disable();
		};

		elementMetatable["Enable"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			entity->Enable();
		};

		elementMetatable["GetDirection"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f direction(nodeComponent.GetRotation(Nz::CoordSys_Global) * Nz::Vector2f::UnitX());
			if (nodeComponent.GetScale().x < 0.f)
				direction = -direction;

			return direction;
		};

		elementMetatable["GetPosition"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition(Nz::CoordSys_Global));
		};

		elementMetatable["GetRotation"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::DegreeAnglef(AngleFromQuaternion(nodeComponent.GetRotation(Nz::CoordSys_Global))); //<FIXME: not very efficient
		};

		elementMetatable["IsEnabled"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			return entity->IsEnabled();
		};

		elementMetatable["IsLookingRight"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		};

		elementMetatable["IsValid"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::RetrieveScriptEntity(entityTable);
			return entity.IsValid();
		};

		elementMetatable["On"] = [&](const sol::table& entityTable, const std::string_view& event, sol::protected_function callback)
		{
			RegisterEvent(entityTable, event, std::move(callback), false);
		};

		elementMetatable["OnAsync"] = [&](const sol::table& entityTable, const std::string_view& event, sol::protected_function callback)
		{
			RegisterEvent(entityTable, event, std::move(callback), true);
		};
    
		elementMetatable["SetScale"] = [](const sol::table& entityTable, const Nz::Vector2f& scale)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::RetrieveScriptEntity(entityTable);
			entity->GetComponent<Ndk::NodeComponent>().SetScale(scale, Nz::CoordSys_Local);
		};

		elementMetatable["SetLifeTime"] = [](const sol::table& entityTable, float lifetime)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::RetrieveScriptEntity(entityTable);
			entity->AddComponent<Ndk::LifetimeComponent>(lifetime);
		};

		elementMetatable["ToLocalPosition"] = [](const sol::table& entityTable, const Nz::Vector2f& globalPosition)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.ToLocalPosition(globalPosition));
		};

		elementMetatable["ToGlobalPosition"] = [](const sol::table& entityTable, const Nz::Vector2f& localPosition)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.ToGlobalPosition(localPosition));
		};
	}
	
	void SharedElementLibrary::RegisterEvent(const sol::table& entityTable, const std::string_view& event, sol::protected_function callback, bool async)
	{
		std::optional<ScriptingEvent> scriptingEventOpt = RetrieveScriptingEvent(event);
		if (!scriptingEventOpt)
			throw std::runtime_error("unknown event " + std::string(event));

		ScriptingEvent scriptingEvent = scriptingEventOpt.value();
		std::size_t eventIndex = static_cast<std::size_t>(scriptingEvent);

		if (async && HasReturnValue(scriptingEvent))
			throw std::runtime_error("events returning a value cannot be async");

		if (auto element = AbstractElementLibrary::RetrieveScriptElement(entityTable))
		{
			auto& callbackData = element->events[eventIndex].emplace_back();
			callbackData.async = async;
			callbackData.callback = std::move(callback);
		}
		else
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& entityScript = entity->GetComponent<ScriptComponent>();
			entityScript.RegisterCallback(scriptingEvent, std::move(callback), async);
		}
	}
}
