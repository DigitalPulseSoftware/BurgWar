// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/ElementEventConnection.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/Components.hpp>
#include <Nazara/Utility/Components.hpp>
#include <sol/sol.hpp>

namespace bw
{
	SharedElementLibrary::~SharedElementLibrary() = default;

	void SharedElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterCommonLibrary(elementMetatable);
	}

	void SharedElementLibrary::RegisterCommonLibrary(sol::table& elementMetatable)
	{
		elementMetatable["DeleteOnRemove"] = LuaFunction([](const sol::table& entityTable, const sol::table& targetEntityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			entt::handle targetEntity = AssertScriptEntity(targetEntityTable);

			entity.get_or_emplace<EntityOwnerComponent>(entity).Register(targetEntity);
		});

		elementMetatable["Disable"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			//entity->Disable();
			//TODO
		});

		elementMetatable["Disconnect"] = LuaFunction([&](const sol::table& entityTable, const ElementEventConnection& eventConnection)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& entityScript = entity.get<ScriptComponent>();
			return std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, ElementEvent>)
					return entityScript.UnregisterCallback(arg, eventConnection.callbackId);
				else if constexpr (std::is_same_v<T, std::size_t>)
					return entityScript.UnregisterCallbackCustom(arg, eventConnection.callbackId);
				else
					static_assert(AlwaysFalse<T>(), "non-exhaustive visitor");
			}, eventConnection.event);
		});

		elementMetatable["Enable"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			//entity->Enable();
			//TODO
		});

		elementMetatable["GetDirection"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();

			Nz::Vector2f direction(nodeComponent.GetRotation(Nz::CoordSys::Global) * Nz::Vector2f::UnitX());
			if (nodeComponent.GetScale().x < 0.f)
				direction = -direction;

			return direction;
		});

		elementMetatable["GetPosition"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition(Nz::CoordSys::Global));
		});

		elementMetatable["GetRotation"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();
			return Nz::DegreeAnglef(AngleFromQuaternion(nodeComponent.GetRotation(Nz::CoordSys::Global))); //<FIXME: not very efficient
		});

		elementMetatable["GetScale"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();
			Nz::Vector2f scale = Nz::Vector2f(nodeComponent.GetScale(Nz::CoordSys::Global));
			return std::abs(scale.y);
		});

		elementMetatable["IsEnabled"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			//return entity->IsEnabled();
		});

		elementMetatable["IsLookingRight"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		});

		elementMetatable["IsValid"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = RetrieveScriptEntity(entityTable);
			return entity.valid();
		});
		
		elementMetatable["Kill"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			HealthComponent* entityHealth = entity.try_get<HealthComponent>();
			if (entityHealth)
				entityHealth->Damage(entityHealth->GetHealth(), entity);
			else
				entity.destroy();
		});

		elementMetatable["On"] = LuaFunction([&](sol::this_state L, const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback)
		{
			return RegisterEvent(L, entityTable, event, std::move(callback), false);
		});

		elementMetatable["OnAsync"] = LuaFunction([&](sol::this_state L, const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback)
		{
			return RegisterEvent(L, entityTable, event, std::move(callback), true);
		});

		elementMetatable["SetLifeTime"] = LuaFunction([](const sol::table& entityTable, float lifetime)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			entity.emplace<Nz::LifetimeComponent>(Nz::Time::Seconds(lifetime));
		});

		elementMetatable["SetScale"] = LuaFunction([&](const sol::table& entityTable, float scale)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			SetScale(entity, scale);
		});

		elementMetatable["ToLocalPosition"] = LuaFunction([](const sol::table& entityTable, const Nz::Vector2f& globalPosition)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();
			return Nz::Vector2f(nodeComponent.ToLocalPosition(globalPosition));
		});

		elementMetatable["ToGlobalPosition"] = LuaFunction([](const sol::table& entityTable, const Nz::Vector2f& localPosition)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity.get<Nz::NodeComponent>();
			return Nz::Vector2f(nodeComponent.ToGlobalPosition(localPosition));
		});

		elementMetatable["Trigger"] = LuaFunction([](const sol::table& entityTable, const std::string_view& event, sol::variadic_args parameters)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& entityScript = entity.get<ScriptComponent>();
			const auto& element = entityScript.GetElement();

			std::string eventName = std::string(event);
			auto it = element->customEventByName.find(eventName);
			if (it == element->customEventByName.end())
				throw std::runtime_error("unknown event " + eventName);

			const auto& eventData = element->customEvents[it->second];

			return entityScript.ExecuteCustomCallback(eventData.index, parameters);
		});
	}

	ElementEventConnection SharedElementLibrary::RegisterCustomEvent(const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async)
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

		if (entt::handle entity = RetrieveScriptEntity(entityTable))
		{
			auto& entityScript = entity.get<ScriptComponent>();
			std::size_t eventIndex = RetrieveEventIndex(entityScript.GetElement());

			std::size_t callbackId = entityScript.RegisterCallbackCustom(eventIndex, std::move(callback), async);
			return ElementEventConnection{ eventIndex, callbackId };
		}
		else
		{
			auto element = AssertScriptElement(entityTable);

			std::size_t eventIndex = RetrieveEventIndex(element);

			if (element->customEventCallbacks.size() <= eventIndex)
				element->customEventCallbacks.resize(eventIndex + 1);

			auto& callbackData = element->customEventCallbacks[eventIndex].emplace_back();
			callbackData.async = async;
			callbackData.callback = std::move(callback);
			callbackData.callbackId = element->nextCallbackId++;

			return ElementEventConnection{ eventIndex, callbackData.callbackId };
		}
	}

	ElementEventConnection SharedElementLibrary::RegisterEvent(lua_State* L, const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async)
	{
		std::optional<ElementEvent> scriptingEventOpt = RetrieveElementEvent(event);
		if (!scriptingEventOpt)
			return RegisterCustomEvent(entityTable, event, std::move(callback), async);

		ElementEvent scriptingEvent = scriptingEventOpt.value();
		std::size_t eventIndex = static_cast<std::size_t>(scriptingEvent);

		if (async && HasReturnValue(scriptingEvent))
			TriggerLuaArgError(L, 2, "events returning a value cannot be async");

		if (entt::handle entity = RetrieveScriptEntity(entityTable))
		{
			auto& entityScript = entity.get<ScriptComponent>();
			std::size_t callbackId = entityScript.RegisterCallback(scriptingEvent, std::move(callback), async);

			return ElementEventConnection{ scriptingEvent, callbackId };
		}
		else
		{
			auto element = AssertScriptElement(entityTable);

			auto& callbackData = element->eventCallbacks[eventIndex].emplace_back();
			callbackData.async = async;
			callbackData.callback = std::move(callback);
			callbackData.callbackId = element->nextCallbackId++;

			return ElementEventConnection{ scriptingEvent, callbackData.callbackId };
		}
	}
}
