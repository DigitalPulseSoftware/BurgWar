// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/BasicPlayerMovementController.hpp>
#include <CoreLib/NoclipPlayerMovementController.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/Constraint.hpp>
#include <CoreLib/Scripting/ElementEventConnection.hpp>
#include <CoreLib/Scripting/GamemodeEventConnection.hpp>
#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Scripting/SharedElementLibrary.hpp>
#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
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

		sol::table gameTable = luaState.create_named_table("game");
		sol::table matchTable = luaState.create_named_table("match");
		sol::table networkTable = luaState.create_named_table("network");
		sol::table physicsTable = luaState.create_named_table("physics");
		sol::table scriptsTable = luaState.create_named_table("scripts");
		sol::table timerTable = luaState.create_named_table("timer");

		RegisterConstraintClass(context);
		RegisterEventConnectionClass(context);
		RegisterGameLibrary(context, gameTable);
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

			"EnableBodyCollision", LuaFunction(&Constraint::EnableBodyCollision),

			"GetErrorBias", LuaFunction(&Constraint::GetErrorBias),
			"GetMaxForce", LuaFunction(&Constraint::GetMaxForce),

			"IsBodyCollisionEnabled", LuaFunction(&Constraint::IsBodyCollisionEnabled),

			"Remove", LuaFunction(&Constraint::Remove),

			"SetErrorBias", LuaFunction(&Constraint::SetErrorBias),
			"SetMaxForce", LuaFunction(&Constraint::SetMaxForce)
		);

		state.new_usertype<DampedSpringConstraint>("SpringConstraint",
			"new", sol::no_constructor,

			sol::base_classes, sol::bases<Constraint>()
		);

		state.new_usertype<PinConstraint>("PinConstraint",
			"new", sol::no_constructor,

			"GetDistance", LuaFunction(&PinConstraint::GetDistance),
			"SetDistance", LuaFunction(&PinConstraint::SetDistance),

			sol::base_classes, sol::bases<Constraint>()
		);

		state.new_usertype<PivotConstraint>("PivotConstraint",
			"new", sol::no_constructor,

			sol::base_classes, sol::bases<Constraint>()
		);

		state.new_usertype<RotaryLimitConstraint>("RotaryLimitConstraint",
			"new", sol::no_constructor,

			"GetMaxAngle", LuaFunction(&RotaryLimitConstraint::GetMaxAngle),
			"GetMinAngle", LuaFunction(&RotaryLimitConstraint::GetMinAngle),
			"SetMaxAngle", LuaFunction(&RotaryLimitConstraint::SetMaxAngle),
			"SetMinAngle", LuaFunction(&RotaryLimitConstraint::SetMinAngle),

			sol::base_classes, sol::bases<Constraint>()
		);
	}

	void SharedScriptingLibrary::RegisterEventConnectionClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<ElementEventConnection>("ElementEventConnection", "new", sol::no_constructor);
		state.new_usertype<GamemodeEventConnection>("GamemodeEventConnection", "new", sol::no_constructor);
	}

	void SharedScriptingLibrary::RegisterNetworkPacketClasses(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<IncomingNetworkPacket>("IncomingNetworkPacket",
			"new", sol::no_constructor,

			"ReadCompressedInteger",  LuaFunction(&IncomingNetworkPacket::ReadCompressedInteger),
			"ReadCompressedUnsigned", LuaFunction(&IncomingNetworkPacket::ReadCompressedUnsigned),
			"ReadColor",   LuaFunction(&IncomingNetworkPacket::ReadColor),
			"ReadDouble",  LuaFunction(&IncomingNetworkPacket::ReadDouble),
			"ReadSingle",  LuaFunction(&IncomingNetworkPacket::ReadSingle),
			"ReadString",  LuaFunction(&IncomingNetworkPacket::ReadString),
			"ReadVector2", LuaFunction(&IncomingNetworkPacket::ReadVector2),

			"ReadEntity", LuaFunction([&](IncomingNetworkPacket& incomingPacket) -> sol::object
			{
				Nz::Int64 entityId = incomingPacket.ReadCompressedInteger();
				const Ndk::EntityHandle& entity = m_match.RetrieveEntityByUniqueId(entityId);
				if (entity && entity->HasComponent<ScriptComponent>())
					return entity->GetComponent<ScriptComponent>().GetTable();
				else
					return sol::nil;
			})
		);

		state.new_usertype<OutgoingNetworkPacket>("OutgoingNetworkPacket",
			"new", sol::no_constructor,

			"WriteCompressedInteger",  LuaFunction(&OutgoingNetworkPacket::WriteCompressedInteger),
			"WriteCompressedUnsigned", LuaFunction (&OutgoingNetworkPacket::WriteCompressedUnsigned),
			"WriteColor",   LuaFunction(&OutgoingNetworkPacket::WriteColor),
			"WriteDouble",  LuaFunction(&OutgoingNetworkPacket::WriteDouble),
			"WriteSingle",  LuaFunction(&OutgoingNetworkPacket::WriteSingle),
			"WriteString",  LuaFunction(&OutgoingNetworkPacket::WriteString),
			"WriteVector2", LuaFunction(&OutgoingNetworkPacket::WriteVector2),

			"WriteEntity", LuaFunction([&](OutgoingNetworkPacket& outgoingPacket, const sol::table& entityTable)
			{
				const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);
				outgoingPacket.WriteCompressedInteger(m_match.RetrieveUniqueIdByEntity(entity));
			})
		);
	}

	void SharedScriptingLibrary::RegisterPlayerMovementControllerClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<PlayerMovementController>("PlayerMovementController");

		state.new_usertype<BasicPlayerMovementController>("BasicPlayerMovementController",
			sol::base_classes, sol::bases<PlayerMovementController>(),
			"new", sol::factories(LuaFunction(&std::make_shared<BasicPlayerMovementController>))
		);
		
		state.new_usertype<NoclipPlayerMovementController>("NoclipPlayerMovementController",
			sol::base_classes, sol::bases<PlayerMovementController>(),
			"new", sol::factories(LuaFunction(&std::make_shared<NoclipPlayerMovementController>))
		);
	}

	void SharedScriptingLibrary::RegisterGameLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["GetVersions"] = LuaFunction([]
		{
			return std::make_tuple(BURGWAR_VERSION_MAJOR, BURGWAR_VERSION_MINOR, BURGWAR_VERSION_PATCH);
		});

		library["GetVersionNumber"] = LuaFunction([]
		{
			return BURGWAR_VERSION;
		});
	}

	void SharedScriptingLibrary::RegisterGlobalLibrary(ScriptingContext& context)
	{
		AbstractScriptingLibrary::RegisterGlobalLibrary(context);

		sol::state& state = context.GetLuaState();
		state["BuildVersion"] = LuaFunction([](Nz::UInt32 major, Nz::UInt32 minor, std::optional<Nz::UInt32> patch)
		{
			return BURGWAR_BUILD_VERSION(major, minor, patch.value_or(0));
		});
	}

	void SharedScriptingLibrary::RegisterMatchLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["GetEntityByUniqueId"] = LuaFunction([&](EntityId uniqueId)
		{
			return TranslateEntityToLua(m_match.RetrieveEntityByUniqueId(uniqueId));
		});

		library["GetEntities"] = LuaFunction([&](sol::this_state L, std::optional<LayerIndex> layerIndexOpt)
		{
			sol::state_view state(L);
			sol::table result = state.create_table();

			std::size_t index = 1;
			auto entityFunc = [&](const Ndk::EntityHandle& entity)
			{
				if (!entity->HasComponent<ScriptComponent>())
					return;

				auto& entityScript = entity->GetComponent<ScriptComponent>();
				result[index++] = entityScript.GetTable();
			};

			if (layerIndexOpt)
			{
				LayerIndex layerIndex = layerIndexOpt.value();
				if (layerIndex >= m_match.GetLayerCount())
					TriggerLuaArgError(L, 2, "invalid layer index");

				m_match.GetLayer(layerIndex).ForEachEntity(entityFunc);
			}
			else
				m_match.ForEachEntity(entityFunc);

			return result;
		});

		library["GetEntitiesByClass"] = LuaFunction([&](sol::this_state L, const std::string& entityClass, std::optional<LayerIndex> layerIndexOpt)
		{
			sol::state_view state(L);
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
					TriggerLuaArgError(L, 2, "invalid layer index");

				m_match.GetLayer(layerIndex).ForEachEntity(entityFunc);
			}
			else
				m_match.ForEachEntity(entityFunc);

			return result;
		});

		library["GetGamemode"] = LuaFunction([this]()
		{
			return m_match.GetSharedGamemode()->GetTable();
		});

		library["GetLayerCount"] = LuaFunction([this]()
		{
			return m_match.GetLayerCount();
		});

		library["GetMilliseconds"] = LuaFunction([this]()
		{
			return m_match.GetCurrentTime();
		});

		library["GetSeconds"] = LuaFunction([this]()
		{
			return m_match.GetCurrentTime() / 1000.f;
		});

		library["GetTickDuration"] = LuaFunction([&]()
		{
			return m_match.GetTickDuration();
		});
	}

	void SharedScriptingLibrary::RegisterNetworkLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["NewPacket"] = LuaFunction([this](sol::this_state L, std::string name) -> OutgoingNetworkPacket
		{
			const NetworkStringStore& networkStringStore = m_match.GetNetworkStringStore();
			if (networkStringStore.GetStringIndex(name) == networkStringStore.InvalidIndex)
				TriggerLuaError(L, "Packet name \"" + name + "\" has not been registered");

			return OutgoingNetworkPacket(std::move(name));
		});

		library["SetHandler"] = LuaFunction([this](std::string name, sol::main_protected_function handler)
		{
			ScriptHandlerRegistry& packetHandlerRegistry = GetSharedMatch().GetScriptPacketHandlerRegistry();

			if (handler)
				packetHandlerRegistry.Register(std::move(name), std::move(handler));
			else
				packetHandlerRegistry.Unregister(name);
		});
	}

	void SharedScriptingLibrary::RegisterPhysicsLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["CreateDampenedSpringConstraint"] = LuaFunction([](sol::this_state L, const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::Vector2f& firstAnchor, const Nz::Vector2f& secondAnchor, float restLength, float stiffness, float damping)
		{
			const Ndk::EntityHandle& firstEntity = AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				TriggerLuaArgError(L, 1, "Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return DampedSpringConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::DampedSpringConstraint2D>(firstEntity, secondEntity, firstAnchor, secondAnchor, restLength, stiffness, damping));
		});

		library["CreatePinConstraint"] = LuaFunction([](sol::this_state L, const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::Vector2f& firstAnchor, const Nz::Vector2f& secondAnchor)
		{
			const Ndk::EntityHandle& firstEntity = AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				TriggerLuaArgError(L, 1, "Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return PinConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::PinConstraint2D>(firstEntity, secondEntity, firstAnchor, secondAnchor));
		});

		library["CreatePivotConstraint"] = LuaFunction([](sol::this_state L, const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::Vector2f& firstAnchor, const Nz::Vector2f& secondAnchor)
		{
			const Ndk::EntityHandle& firstEntity = AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				TriggerLuaArgError(L, 1, "Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return PivotConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::PivotConstraint2D>(firstEntity, secondEntity, firstAnchor, secondAnchor));
		});

		library["CreateRotaryLimitConstraint"] = LuaFunction([](sol::this_state L, const sol::table& firstEntityTable, const sol::table& secondEntityTable, const Nz::RadianAnglef& minAngle, const Nz::RadianAnglef& maxAngle)
		{
			const Ndk::EntityHandle& firstEntity = AssertScriptEntity(firstEntityTable);
			const Ndk::EntityHandle& secondEntity = AssertScriptEntity(secondEntityTable);

			if (firstEntity == secondEntity)
				TriggerLuaArgError(L, 1, "Cannot apply a constraint to the same entity");

			const Ndk::EntityHandle& constraintEntity = firstEntity->GetWorld()->CreateEntity();
			auto& constraintComponent = constraintEntity->AddComponent<Ndk::ConstraintComponent2D>();

			return RotaryLimitConstraint(constraintEntity, constraintComponent.CreateConstraint<Nz::RotaryLimitConstraint2D>(firstEntity, secondEntity, minAngle, maxAngle));
		});

		library["RegionQuery"] = LuaFunction([this](sol::this_state L, LayerIndex layer, const Nz::Rectf& rect, const sol::protected_function& callback)
		{
			if (layer >= m_match.GetLayerCount())
				TriggerLuaArgError(L, 1, "invalid layer index");

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
		});

		library["Trace"] = LuaFunction([this](sol::this_state L, LayerIndex layer, Nz::Vector2f startPos, Nz::Vector2f endPos) -> sol::object
		{
			if (layer >= m_match.GetLayerCount())
				TriggerLuaArgError(L, 1, "invalid layer index");

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
		});

		library["TraceMultiple"] = LuaFunction([this](sol::this_state L, LayerIndex layer, Nz::Vector2f startPos, Nz::Vector2f endPos, const sol::protected_function& callback)
		{
			if (layer >= m_match.GetLayerCount())
				TriggerLuaArgError(L, 1, "invalid layer index");

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
		});
	}

	void SharedScriptingLibrary::RegisterScriptLibrary(ScriptingContext& /*context*/, sol::table& /*library*/)
	{
		// empty for now
	}

	void SharedScriptingLibrary::RegisterTimerLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["Create"] = LuaFunction([&](Nz::UInt64 time, sol::main_protected_function callback)
		{
			m_match.GetTimerManager().PushCallback(m_match.GetCurrentTime() + time, [this, callback = std::move(callback)]()
			{
				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(GetLogger(), LogLevel::Error, "timer.Create callback failed: {0}", err.what());
				}
			});
		});
	}
}
