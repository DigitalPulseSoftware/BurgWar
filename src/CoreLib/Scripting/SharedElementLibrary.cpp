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

		elementMetatable["On"] = [&](const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback)
		{
			RegisterEvent(entityTable, event, std::move(callback), false);
		};

		elementMetatable["OnAsync"] = [&](const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback)
		{
			RegisterEvent(entityTable, event, std::move(callback), true);
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

		elementMetatable["Trigger"] = [](const sol::table& entityTable, const std::string_view& event, sol::variadic_args parameters)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& entityScript = entity->GetComponent<ScriptComponent>();
			const auto& element = entityScript.GetElement();

			std::string eventName = std::string(event);
			auto it = element->customEventByName.find(eventName);
			if (it == element->customEventByName.end())
				throw std::runtime_error("unknown event " + eventName);

			const auto& eventData = element->customEvents[it->second];

			return entityScript.ExecuteCustomCallback(eventData.index, parameters);
		};
	}

	void SharedElementLibrary::RegisterCustomEvent(const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async)
	{
		auto RetrieveEventIndex = [&](const std::shared_ptr<const ScriptedElement>& element) -> std::size_t
		{
			std::string eventName = std::string(event);

			auto it = element->customEventByName.find(eventName);
			if (it == element->customEventByName.end())
				throw std::runtime_error("unknown event " + eventName);

			const auto& eventData = element->customEvents[it->second];

			if (async && !eventData.returnType.empty())
				throw std::runtime_error("events returning a value cannot be async");

			return eventData.index;
		};

		if (auto element = AbstractElementLibrary::RetrieveScriptElement(entityTable))
		{
			std::size_t eventIndex = RetrieveEventIndex(element);

			if (element->customEventCallbacks.size() <= eventIndex)
				element->customEventCallbacks.resize(eventIndex + 1);

			auto& callbackData = element->customEventCallbacks[eventIndex].emplace_back();
			callbackData.async = async;
			callbackData.callback = std::move(callback);
		}
		else
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& entityScript = entity->GetComponent<ScriptComponent>();
			std::size_t eventIndex = RetrieveEventIndex(entityScript.GetElement());

			entityScript.RegisterCallbackCustom(eventIndex, std::move(callback), async);
		}
	}

	void SharedElementLibrary::RegisterEvent(const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async)
	{
		std::optional<ElementEvent> scriptingEventOpt = RetrieveElementEvent(event);
		if (!scriptingEventOpt)
			return RegisterCustomEvent(entityTable, event, std::move(callback), async);

		ElementEvent scriptingEvent = scriptingEventOpt.value();
		std::size_t eventIndex = static_cast<std::size_t>(scriptingEvent);

		if (async && HasReturnValue(scriptingEvent))
			throw std::runtime_error("events returning a value cannot be async");

		if (auto element = AbstractElementLibrary::RetrieveScriptElement(entityTable))
		{
			auto& callbackData = element->eventCallbacks[eventIndex].emplace_back();
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
