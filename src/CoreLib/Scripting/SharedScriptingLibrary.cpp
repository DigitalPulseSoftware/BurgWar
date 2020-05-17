// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/BasicPlayerMovementController.hpp>
#include <CoreLib/NoclipPlayerMovementController.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/Constraint.hpp>
#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <Nazara/Physics2D/Constraint2D.hpp>
#include <NDK/Components/ConstraintComponent2D.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <CoreLib/SharedMatch.hpp>

namespace bw
{
	SharedScriptingLibrary::SharedScriptingLibrary(SharedMatch& sharedMatch) :
	AbstractScriptingLibrary(sharedMatch.GetLogger()),
	m_match(sharedMatch)
	{
	}

	SharedScriptingLibrary::~SharedScriptingLibrary() = default;

	void SharedScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState.open_libraries();

		sol::table matchTable = luaState.create_named_table("match");
		sol::table networkTable = luaState.create_named_table("network");
		sol::table physicsTable = luaState.create_named_table("physics");
		sol::table scriptsTable = luaState.create_named_table("scripts");
		sol::table timerTable = luaState.create_named_table("timer");

		RegisterConstraintClass(context);
		RegisterGlobalLibrary(context);
		RegisterMatchLibrary(context, matchTable);
		RegisterMetatableLibrary(context);
		RegisterNetworkLibrary(context, networkTable);
		RegisterNetworkPacketClasses(context);
		RegisterRandomEngineClass(context);
		RegisterPhysicsLibrary(context, physicsTable);
		RegisterPlayerMovementControllerClass(context);
		RegisterScriptLibrary(context, scriptsTable);
		RegisterTimerLibrary(context, timerTable);
	}

	void SharedScriptingLibrary::RegisterConstraintClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<Constraint>("Constraint",
			"new", sol::no_constructor,

			"EnableBodyCollision", &Constraint::EnableBodyCollision,

			"GetErrorBias", &Constraint::GetErrorBias,
			"GetMaxForce", &Constraint::GetMaxForce,

			"IsBodyCollisionEnabled", &Constraint::IsBodyCollisionEnabled,

			"Remove", &Constraint::Remove,

			"SetErrorBias", &Constraint::SetErrorBias,
			"SetMaxForce",  &Constraint::SetMaxForce
		);

		state.new_usertype<DampedSpringConstraint>("SpringConstraint",
			"new", sol::no_constructor,

			sol::base_classes, sol::bases<Constraint>()
		);

		state.new_usertype<PinConstraint>("PinConstraint",
			"new", sol::no_constructor,

			"GetDistance", &PinConstraint::GetDistance,
			"SetDistance", &PinConstraint::SetDistance,

			sol::base_classes, sol::bases<Constraint>()
		);

		state.new_usertype<PivotConstraint>("PivotConstraint",
			"new", sol::no_constructor,

			sol::base_classes, sol::bases<Constraint>()
		);

		state.new_usertype<RotaryLimitConstraint>("RotaryLimitConstraint",
			"new", sol::no_constructor,

			"GetMaxAngle", &RotaryLimitConstraint::GetMaxAngle,
			"GetMinAngle", &RotaryLimitConstraint::GetMinAngle,
			"SetMaxAngle", &RotaryLimitConstraint::SetMaxAngle,
			"SetMinAngle", &RotaryLimitConstraint::SetMinAngle,

			sol::base_classes, sol::bases<Constraint>()
		);
	}

	void SharedScriptingLibrary::RegisterNetworkPacketClasses(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<IncomingNetworkPacket>("IncomingNetworkPacket",
			"new", sol::no_constructor,

			"ReadCompressedUnsigned", &IncomingNetworkPacket::ReadCompressedUnsigned,
			"ReadString", &IncomingNetworkPacket::ReadString
		);

		state.new_usertype<OutgoingNetworkPacket>("OutgoingNetworkPacket",
			"new", sol::no_constructor,

			"WriteCompressedUnsigned", &OutgoingNetworkPacket::WriteCompressedUnsigned,
			"WriteString", &OutgoingNetworkPacket::WriteString
		);
	}

	void SharedScriptingLibrary::RegisterPlayerMovementControllerClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<PlayerMovementController>("PlayerMovementController");

		state.new_usertype<BasicPlayerMovementController>("BasicPlayerMovementController",
			sol::base_classes, sol::bases<PlayerMovementController>(),
			"new", sol::factories(&std::make_shared<BasicPlayerMovementController>)
		);
		
		state.new_usertype<NoclipPlayerMovementController>("NoclipPlayerMovementController",
			sol::base_classes, sol::bases<PlayerMovementController>(),
			"new", sol::factories(&std::make_shared<NoclipPlayerMovementController>)
		);
	}

	void SharedScriptingLibrary::RegisterMatchLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["GetEntitiesByClass"] = [&](sol::this_state s, const std::string& entityClass, std::optional<LayerIndex> layerIndexOpt)
		{
			sol::state_view state(s);
			sol::table result = state.create_table();

			std::size_t index = 1;
			auto entityFunc = [&](const Ndk::EntityHandle& entity)
			{
				if (!entity->HasComponent<ScriptComponent>())
					return;

				auto& entityScript = entity->GetComponent<ScriptComponent>();
				if (entityScript.GetElement()->fullName == entityClass)
					result[index++] = entityScript.GetTable();
			};

			if (layerIndexOpt)
			{
				LayerIndex layerIndex = layerIndexOpt.value();
				if (layerIndex >= m_match.GetLayerCount())
					throw std::runtime_error("Invalid layer index");

				m_match.GetLayer(layerIndex).ForEachEntity(entityFunc);
			}
			else
				m_match.ForEachEntity(entityFunc);

			return result;
		};

		library["GetMilliseconds"] = [this]()
		{
			return m_match.GetCurrentTime();
		};

		library["GetSeconds"] = [this]()
		{
			return m_match.GetCurrentTime() / 1000.f;
		};

		library["GetTickDuration"] = [&]()
		{
			return m_match.GetTickDuration();
		};
	}

	void SharedScriptingLibrary::RegisterNetworkLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["NewPacket"] = [this](std::string name) -> OutgoingNetworkPacket
		{
			const NetworkStringStore& networkStringStore = m_match.GetNetworkStringStore();
			if (networkStringStore.GetStringIndex(name) == networkStringStore.InvalidIndex)
				throw std::runtime_error("Packet name \"" + name + "\" has not been registered");

			return OutgoingNetworkPacket(std::move(name));
		};

		library["SetHandler"] = [this](std::string name, sol::protected_function handler)
		{
			ScriptHandlerRegistry& packetHandlerRegistry = GetSharedMatch().GetScriptPacketHandlerRegistry();

			if (handler)
				packetHandlerRegistry.Register(std::move(name), std::move(handler));
			else
				packetHandlerRegistry.Unregister(name);
		};
	}

	void SharedScriptingLibrary::RegisterPhysicsLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["CreateDampenedSpringConstraint"] = [this](const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::Vector2f& firstAnchor, const Nz::Vector2f& secondAnchor, float restLength, float stiffness, float damping)
		{
			const Ndk::EntityHandle& firstEntity = SharedElementLibrary::AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = SharedElementLibrary::AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				throw std::runtime_error("Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return DampedSpringConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::DampedSpringConstraint2D>(firstEntity, secondEntity, firstAnchor, secondAnchor, restLength, stiffness, damping));
		};

		library["CreatePinConstraint"] = [this](const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::Vector2f& firstAnchor, const Nz::Vector2f& secondAnchor)
		{
			const Ndk::EntityHandle& firstEntity = SharedElementLibrary::AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = SharedElementLibrary::AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				throw std::runtime_error("Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return PinConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::PinConstraint2D>(firstEntity, secondEntity, firstAnchor, secondAnchor));
		};

		library["CreatePivotConstraint"] = [this](const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::Vector2f& firstAnchor, const Nz::Vector2f& secondAnchor)
		{
			const Ndk::EntityHandle& firstEntity = SharedElementLibrary::AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = SharedElementLibrary::AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				throw std::runtime_error("Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return PivotConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::PivotConstraint2D>(firstEntity, secondEntity, firstAnchor, secondAnchor));
		};

		library["CreateRotaryLimitConstraint"] = [this](const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::RadianAnglef& minAngle, const Nz::RadianAnglef& maxAngle)
		{
			const Ndk::EntityHandle& firstEntity = SharedElementLibrary::AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = SharedElementLibrary::AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				throw std::runtime_error("Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return RotaryLimitConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::RotaryLimitConstraint2D>(firstEntity, secondEntity, minAngle, maxAngle));
		};

		library["RegionQuery"] = [this](sol::this_state L, LayerIndex layer, const Nz::Rectf& rect, const sol::protected_function& callback)
		{
			if (layer >= m_match.GetLayerCount())
				throw std::runtime_error("Invalid layer index");

			Ndk::World& world = m_match.GetLayer(layer).GetWorld();
			auto& physSystem = world.GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::EntityList hitEntities; //< FIXME: RegionQuery hit multiples entities

			sol::state_view state(L);
			auto resultCallback = [&](const Ndk::EntityHandle& hitEntity)
			{
				if (hitEntities.Has(hitEntity))
					return;

				hitEntities.Insert(hitEntity);

				if (hitEntity->HasComponent<ScriptComponent>())
				{
					auto callbackResult = callback(hitEntity->GetComponent<ScriptComponent>().GetTable());
					if (!callbackResult.valid())
					{
						sol::error err = callbackResult;
						bwLog(m_match.GetLogger(), LogLevel::Error, "physics.RegionQuery callback failed: {}", err.what());
					}
				}
			};

			physSystem.RegionQuery(rect, 0, 0xFFFFFFFF, 0xFFFFFFFF, resultCallback);
		};

		library["Trace"] = [this](sol::this_state L, LayerIndex layer, Nz::Vector2f startPos, Nz::Vector2f endPos) -> sol::object
		{
			if (layer >= m_match.GetLayerCount())
				throw std::runtime_error("Invalid layer index");

			Ndk::World& world = m_match.GetLayer(layer).GetWorld();
			auto& physSystem = world.GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::PhysicsSystem2D::RaycastHit hitInfo;
			if (physSystem.RaycastQueryFirst(startPos, endPos, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitInfo))
			{
				sol::state_view state(L);
				sol::table result = state.create_table();
				result["fraction"] = hitInfo.fraction;
				result["hitPos"] = hitInfo.hitPos;
				result["hitNormal"] = hitInfo.hitNormal;

				const Ndk::EntityHandle& hitEntity = hitInfo.body;
				if (hitEntity->HasComponent<ScriptComponent>())
					result["hitEntity"] = hitEntity->GetComponent<ScriptComponent>().GetTable();

				return result;
			}
			else
				return sol::nil;
		};

		library["TraceMultiple"] = [this](sol::this_state L, LayerIndex layer, Nz::Vector2f startPos, Nz::Vector2f endPos, const sol::protected_function& callback)
		{
			if (layer >= m_match.GetLayerCount())
				throw std::runtime_error("Invalid layer index");

			Ndk::World& world = m_match.GetLayer(layer).GetWorld();
			auto& physSystem = world.GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::EntityList hitEntities; //< FIXME: RegionQuery hit multiples entities

			sol::state_view state(L);
			auto resultCallback = [&](const Ndk::PhysicsSystem2D::RaycastHit& hitInfo)
			{
				const Ndk::EntityHandle& hitEntity = hitInfo.body;
				if (hitEntities.Has(hitEntity))
					return;

				hitEntities.Insert(hitEntity);

				sol::table result = state.create_table();
				result["fraction"] = hitInfo.fraction;
				result["hitPos"] = hitInfo.hitPos;
				result["hitNormal"] = hitInfo.hitNormal;

				if (hitEntity->HasComponent<ScriptComponent>())
					result["hitEntity"] = hitEntity->GetComponent<ScriptComponent>().GetTable();

				auto callbackResult = callback(result);
				if (!callbackResult.valid())
				{
					sol::error err = callbackResult;
					bwLog(m_match.GetLogger(), LogLevel::Error, "physics.RegionQuery callback failed: {}", err.what());
				}
			};

			physSystem.RaycastQuery(startPos, endPos, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, resultCallback);
		};
	}

	void SharedScriptingLibrary::RegisterScriptLibrary(ScriptingContext& /*context*/, sol::table& /*library*/)
	{
		// empty for now
	}

	void SharedScriptingLibrary::RegisterTimerLibrary(ScriptingContext& context, sol::table& library)
	{
		sol::state& state = context.GetLuaState();
		library["Create"] = [&](Nz::UInt64 time, sol::object callbackObject)
		{
			m_match.GetTimerManager().PushCallback(m_match.GetCurrentTime() + time, [this, &state, callbackObject]()
			{
				sol::protected_function callback(state, sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(GetLogger(), LogLevel::Error, "engine_SetTimer failed: {0}", err.what());
				}
			});
		};
	}
}
