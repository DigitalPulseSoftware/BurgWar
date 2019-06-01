// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatchPrediction.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	LocalMatchPrediction::LocalMatchPrediction(LocalMatch& match) :
		m_world(false)
	{
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<Ndk::VelocitySystem>();
		auto& reconciliationPhysics = m_world.AddSystem<Ndk::PhysicsSystem2D>();
		reconciliationPhysics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		reconciliationPhysics.SetMaxStepCount(1);
		reconciliationPhysics.SetStepSize(match.GetTickDuration());

		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.SetFixedUpdateRate(0.f);
			system.SetMaximumUpdateRate(0.f);
		});
	}

	const Ndk::EntityHandle& LocalMatch::CreateReconciliationEntity(const Ndk::EntityHandle& serverEntity)
	{
		const Ndk::EntityHandle& entity = m_prediction.reconciliationWorld.CreateEntity();
		entity->AddComponent<Ndk::NodeComponent>();

		if (serverEntity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			const auto& originalPhys = serverEntity->GetComponent<Ndk::PhysicsComponent2D>();

			entity->AddComponent(serverEntity->GetComponent<Ndk::CollisionComponent2D>().Clone());
			auto& entityPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetAngularDamping(originalPhys.GetAngularDamping());
			entityPhys.SetElasticity(originalPhys.GetElasticity());
			entityPhys.SetFriction(originalPhys.GetFriction());
			entityPhys.SetMass(originalPhys.GetMass());
			entityPhys.SetMassCenter(originalPhys.GetMassCenter());
			entityPhys.SetMomentOfInertia(originalPhys.GetMomentOfInertia());
			entityPhys.SetSurfaceVelocity(originalPhys.GetSurfaceVelocity());
		}

		assert(m_prediction.reconciliationEntities.find(serverEntity->GetId()) == m_prediction.reconciliationEntities.end());
		m_prediction.reconciliationEntities.emplace(serverEntity->GetId(), entity);

		return entity;
	}

	void LocalMatch::DebugEntityId(ServerEntity& serverEntity)
	{
		auto& gfxComponent = serverEntity.entity->GetComponent<Ndk::GraphicsComponent>();
		auto& nodeComponent = serverEntity.entity->GetComponent<Ndk::NodeComponent>();

		const Nz::Boxf& aabb = gfxComponent.GetAABB();
		Nz::Vector3f offset = nodeComponent.GetPosition() - aabb.GetCenter();

		Nz::TextSpriteRef text = Nz::TextSprite::New(Nz::SimpleTextDrawer::Draw("S: " + std::to_string(serverEntity.serverEntityId) + ", C: " + std::to_string(serverEntity.entity->GetId()), 36));
		Nz::Boxf volume = text->GetBoundingVolume().obb.localBox;

		gfxComponent.Attach(text, Nz::Matrix4f::Translate(Nz::Vector3f(aabb.width / 2.f - volume.width / 2.f, aabb.height / 2 - 5 - volume.height / 2.f, 0.f)));
	}

	Nz::UInt16 LocalMatch::EstimateServerTick() const
	{
		return m_currentServerTick - m_averageTickError.GetAverageValue();
	}

	void LocalMatch::HandleTickPacket(TickPacketContent&& packet)
	{
		std::visit([this](auto&& packet)
		{
			HandleTickPacket(std::move(packet));
		}, std::move(packet));
	}

	void LocalMatch::HandleTickPacket(Packets::ControlEntity&& packet)
	{
		auto it = m_serverEntityIdToClient.find(packet.entityId);
		if (it == m_serverEntityIdToClient.end())
			return;

		const ServerEntity& serverEntity = it->second;

		if (m_playerData[packet.playerIndex].controlledEntity)
			m_playerData[packet.playerIndex].controlledEntity->RemoveComponent<Ndk::ListenerComponent>();

		m_playerData[packet.playerIndex].controlledEntity = serverEntity.entity;
		m_playerData[packet.playerIndex].controlledEntity->AddComponent<Ndk::ListenerComponent>();
		m_playerData[packet.playerIndex].controlledEntityServerId = packet.entityId;

		m_playerData[packet.playerIndex].reconciliationEntity = CreateReconciliationEntity(serverEntity.entity);
		m_playerData[packet.playerIndex].reconciliationEntity->AddComponent<InputComponent>();
		m_playerData[packet.playerIndex].reconciliationEntity->AddComponent<PlayerMovementComponent>();
	}

	void LocalMatch::HandleTickPacket(Packets::CreateEntities&& packet)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		const NetworkStringStore& networkStringStore = m_session.GetNetworkStringStore();

		for (auto&& entityData : packet.entities)
		{
			const std::string& entityClass = networkStringStore.GetString(entityData.entityClass);

			EntityProperties properties;
			for (const auto& property : entityData.properties)
			{
				const std::string& propertyName = networkStringStore.GetString(property.name);

				std::visit([&](auto&& value)
				{
					using T = std::decay_t<decltype(value)>;

					if constexpr (std::is_same_v<T, std::vector<bool>> ||
						std::is_same_v<T, std::vector<float>> ||
						std::is_same_v<T, std::vector<Nz::Int64>> ||
						std::is_same_v<T, std::vector<Nz::Vector2f>> ||
						std::is_same_v<T, std::vector<Nz::Vector2i64>> ||
						std::is_same_v<T, std::vector<std::string>>)
					{
						using StoredType = typename T::value_type;

						if (property.isArray)
						{
							EntityPropertyArray<StoredType> elements(value.size());
							for (std::size_t i = 0; i < value.size(); ++i)
								elements[i] = value[i];

							properties.emplace(propertyName, std::move(elements));
						}
						else
							properties.emplace(propertyName, value.front());
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, property.value);
			}

			/*const*/ ServerEntity* parent = nullptr;
			if (entityData.parentId)
			{
				auto it = m_serverEntityIdToClient.find(entityData.parentId.value());
				assert(it != m_serverEntityIdToClient.end());

				//parent = &it->second;
				parent = &it.value();
			}

			Ndk::EntityHandle entity;
			if (entityClass.compare(0, entityPrefix.size(), entityPrefix) == 0)
			{
				// Entity
				if (std::size_t entityIndex = m_entityStore->GetElementIndex(entityClass); entityIndex != ClientEntityStore::InvalidIndex)
				{
					entity = m_entityStore->InstantiateEntity(m_world, entityIndex, entityData.position, entityData.rotation, properties);
					if (!entity)
						continue;
				}
			}
			else if (entityClass.compare(0, weaponPrefix.size(), weaponPrefix) == 0)
			{
				// Weapon
				if (std::size_t weaponIndex = m_weaponStore->GetElementIndex(entityClass); weaponIndex != ClientEntityStore::InvalidIndex)
				{
					assert(parent);

					parent->weaponEntityId = entityData.id; //< TEMPORARY

					entity = m_weaponStore->InstantiateWeapon(m_world, weaponIndex, properties, parent->entity);
					if (!entity)
						continue;

					entity->GetComponent<Ndk::NodeComponent>().SetPosition(entityData.position);
				}
			}
			else
			{
				// Unknown
				std::cerr << "Failed to decode entity type: " << entityClass << std::endl;
				continue;
			}

			if (entity)
			{
				ServerEntity serverEntity;
				serverEntity.entity = entity;
				serverEntity.isPhysical = entityData.physicsProperties.has_value();
				serverEntity.maxHealth = (entityData.health.has_value()) ? entityData.health->maxHealth : 0;
				serverEntity.serverEntityId = entityData.id;

				entity->AddComponent<LocalMatchComponent>(shared_from_this());
				//entity->AddComponent<Ndk::DebugComponent>(Ndk::DebugDraw::Collider2D | Ndk::DebugDraw::GraphicsAABB | Ndk::DebugDraw::GraphicsOBB);
				//DebugEntityId(serverEntity);

				if (m_debug)
					CreateGhostEntity(serverEntity);

				if (entityData.health && entityData.health->currentHealth != entityData.health->maxHealth)
					CreateHealthBar(serverEntity, entityData.health->currentHealth);

				if (entityData.name)
					CreateName(serverEntity, entityData.name.value());

				m_serverEntityIdToClient.emplace(entityData.id, std::move(serverEntity));
			}
		}
	}

	void LocalMatch::HandleTickPacket(Packets::DeleteEntities&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			m_prediction.reconciliationEntities.erase(entityData.id);

			auto it = m_serverEntityIdToClient.find(entityData.id);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				continue;

			m_serverEntityIdToClient.erase(it);
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesAnimation&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.entityId);
			if (it == m_serverEntityIdToClient.end())
				continue;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			auto& animComponent = serverEntity.entity->GetComponent<AnimationComponent>();
			animComponent.Play(entityData.animId, m_application.GetAppTime());
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesInputs&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			if (it == m_serverEntityIdToClient.end())
				continue;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			//serverEntity.entity->GetComponent<InputComponent>().UpdateInputs(entityData.inputs);

			// TEMPORARY
			if (serverEntity.weaponEntityId != 0xFFFFFFFF)
			{
				auto weaponIt = m_serverEntityIdToClient.find(serverEntity.weaponEntityId);
				if (weaponIt == m_serverEntityIdToClient.end())
					return;

				ServerEntity& weaponEntity = weaponIt.value();
				if (!weaponEntity.entity)
					return;

				if (entityData.inputs.isAttacking)
				{
					auto& weaponCooldown = weaponEntity.entity->GetComponent<CooldownComponent>();
					if (weaponCooldown.Trigger(GetCurrentTime()))
					{
						auto& weaponScript = weaponEntity.entity->GetComponent<ScriptComponent>();
						weaponScript.ExecuteCallback("OnAttack", weaponScript.GetTable());
					}
				}
			}
		}
	}

	void LocalMatch::HandleTickPacket(Packets::HealthUpdate&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			if (it == m_serverEntityIdToClient.end())
				continue;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			if (serverEntity.health)
			{
				HealthData& healthData = serverEntity.health.value();
				healthData.currentHealth = entityData.currentHealth;
				healthData.healthSprite->SetSize(healthData.spriteWidth * healthData.currentHealth / serverEntity.maxHealth, 10);
			}
			else
				CreateHealthBar(serverEntity, entityData.currentHealth);
		}
	}

	void LocalMatch::HandleTickPacket(Packets::MatchState&& packet)
	{
		if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::A))
			return;

		static std::ofstream debugFile("prediction.txt", std::ios::trunc);
		debugFile << "---------------------------------------------------\n";
		debugFile << "Received match state for tick #" << packet.stateTick << " (current estimation: " << EstimateServerTick() << ")\n";
		Nz::Vector2f serverPos;

		//std::cout << "Received tick packet: " << packet.stateTick << std::endl;

		// Remove treated inputs
		auto firstClientInput = std::find_if(m_prediction.predictedInputs.begin(), m_prediction.predictedInputs.end(), [stateTick = packet.stateTick](const PredictedInput& input)
		{
			return input.serverTick > stateTick;
		});
		m_prediction.predictedInputs.erase(m_prediction.predictedInputs.begin(), firstClientInput);

		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				return;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				return;

			// Check if controlled by local player (should be predicted)
			bool shouldBePredicted = false;
			std::size_t playerIndex = 0;

			for (; playerIndex < m_playerData.size(); ++playerIndex)
			{
				auto& controllerData = m_playerData[playerIndex];

				if (controllerData.controlledEntity == serverEntity.entity)
				{
					shouldBePredicted = true;
					break;
				}
			}

			if (shouldBePredicted)
			{
				auto& controllerData = m_playerData[playerIndex];

				if (controllerData.reconciliationEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					assert(entityData.physicsProperties);

					auto& entityPhys = controllerData.reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();
					entityPhys.SetPosition(entityData.position);
					entityPhys.SetRotation(entityData.rotation);
					entityPhys.SetAngularVelocity(entityData.physicsProperties->angularVelocity);
					entityPhys.SetVelocity(entityData.physicsProperties->linearVelocity);

					debugFile << "Burger position: " << entityData.position.x << "\n";
					debugFile << "Burger velocity: " << entityData.physicsProperties->linearVelocity.x << "\n";

					//std::cout << "Server position: " << entityData.position << std::endl;
					serverPos = entityData.position;
				}
				else
				{
					auto& entityNode = controllerData.reconciliationEntity->GetComponent<Ndk::NodeComponent>();
					entityNode.SetPosition(entityData.position);
					entityNode.SetRotation(entityData.rotation);
				}
			}
			else
			{
				if (serverEntity.isPhysical)
				{
					assert(entityData.physicsProperties);

					auto& physComponent = serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>();

					serverEntity.positionError += physComponent.GetPosition() - entityData.position;
					serverEntity.rotationError += physComponent.GetRotation() - entityData.rotation;

					if (serverEntity.entity->HasComponent<PlayerMovementComponent>())
					{
						auto& playerMovementComponent = serverEntity.entity->GetComponent<PlayerMovementComponent>();

						if (playerMovementComponent.UpdateFacingRightState(entityData.playerMovement->isFacingRight))
						{
							auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
							entityNode.Scale(-1.f, 1.f);
						}
					}

					physComponent.SetPosition(entityData.position);
					physComponent.SetRotation(entityData.rotation);

					physComponent.SetAngularVelocity(entityData.physicsProperties->angularVelocity);
					physComponent.SetVelocity(entityData.physicsProperties->linearVelocity);
				}
				else
				{
					auto& nodeComponent = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
					nodeComponent.SetPosition(entityData.position);
					nodeComponent.SetRotation(entityData.rotation);
				}
			}
		}

		//return;

		// Reconciliate player entities
		auto& physicsSystem = m_world.GetSystem<Ndk::PhysicsSystem2D>();

		std::vector<Ndk::EntityHandle> surroundingEntities;
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			if (controllerData.controlledEntity)
			{
				if (controllerData.controlledEntity->HasComponent<Ndk::CollisionComponent2D>())
				{
					Nz::Vector2f position;
					if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
						position = controllerData.controlledEntity->GetComponent<Ndk::PhysicsComponent2D>().GetPosition();
					else
						position = Nz::Vector2f(controllerData.controlledEntity->GetComponent<Ndk::NodeComponent>().GetPosition());

					std::size_t oldVecSize = surroundingEntities.size();
					physicsSystem.RegionQuery(Nz::Rectf(position.x - 500, position.y - 500, 1000.f, 1000.f), 0, 0xFFFFFFFF, 0xFFFFFFFF, &surroundingEntities);

					for (auto it = surroundingEntities.begin() + oldVecSize; it != surroundingEntities.end(); ++it)
					{
						if (*it == controllerData.controlledEntity)
						{
							surroundingEntities.erase(it);
							break;
						}
					}
				}
			}
		}

		for (const Ndk::EntityHandle& entity : surroundingEntities)
		{
			Ndk::EntityHandle reconciliationEntity;

			auto it = m_prediction.reconciliationEntities.find(entity->GetId());
			if (it == m_prediction.reconciliationEntities.end())
				reconciliationEntity = CreateReconciliationEntity(entity);
			else
				reconciliationEntity = it->second;

			assert(reconciliationEntity);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& realPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
				auto& reconciliationPhys = reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

				reconciliationPhys.SetAngularVelocity(realPhys.GetAngularVelocity());
				reconciliationPhys.SetPosition(realPhys.GetPosition());
				reconciliationPhys.SetRotation(realPhys.GetRotation());
				reconciliationPhys.SetVelocity(realPhys.GetVelocity());
			}
			else
			{
				auto& realNode = reconciliationEntity->GetComponent<Ndk::NodeComponent>();
				auto& reconciliationNode = entity->GetComponent<Ndk::NodeComponent>();

				reconciliationNode.SetPosition(realNode.GetPosition());
				reconciliationNode.SetRotation(realNode.GetRotation());
			}
		}

		/*for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			if (controllerData.controlledEntity)
			{
				assert(controllerData.reconciliationEntity);

				if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					auto& reconciliationPhys = controllerData.reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

					//std::cout << "Before prediction position: " << reconciliationPhys.GetPosition() << std::endl;

					if (reconciliationPhys.GetVelocity().x > 0.f)
					{
						//std::cout << "Let's go" << std::endl;
					}
				}
			}
		}*/

		//std::cout << m_prediction.predictedInputs.size() << " inputs pending" << std::endl;

		for (const PredictedInput& input : m_prediction.predictedInputs)
		{
			for (std::size_t i = 0; i < m_playerData.size(); ++i)
			{
				auto& controllerData = m_playerData[i];
				if (controllerData.reconciliationEntity)
				{
					InputComponent& entityInputs = controllerData.reconciliationEntity->GetComponent<InputComponent>();
					const auto& playerInputData = input.inputs[i];
					entityInputs.UpdateInputs(playerInputData.input);

					debugFile << "---- prediction (input tick: #" << input.serverTick << ", moving: " << std::boolalpha << playerInputData.input.isMovingRight << ")\n";

					if (playerInputData.movement)
					{
						auto& playerMovement = controllerData.reconciliationEntity->GetComponent<PlayerMovementComponent>();
						auto& playerPhysics = controllerData.reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();
						playerMovement.UpdateGroundState(playerInputData.movement->isOnGround);
						playerMovement.UpdateJumpTime(playerInputData.movement->jumpTime);
						playerMovement.UpdateWasJumpingState(playerInputData.movement->wasJumping);

						playerPhysics.SetFriction(playerInputData.movement->friction);
						playerPhysics.SetSurfaceVelocity(playerInputData.movement->surfaceVelocity);
					}
				}
			}

			m_prediction.reconciliationWorld.Update(GetTickDuration());

			for (std::size_t i = 0; i < m_playerData.size(); ++i)
			{
				auto& controllerData = m_playerData[i];
				if (controllerData.controlledEntity)
				{
					assert(controllerData.reconciliationEntity);

					if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
					{
						auto& reconciliationPhys = controllerData.reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

						debugFile << "new position: " << reconciliationPhys.GetPosition().x << "\n";
						debugFile << "new velocity: " << reconciliationPhys.GetVelocity().x << "\n";

						//std::cout << "[Client][Reconciliation] After world update (by " << GetTickDuration() << "ms) position: " << reconciliationPhys.GetPosition() << std::endl;
					}
				}
			}
		}

		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			if (controllerData.controlledEntity)
			{
				assert(controllerData.reconciliationEntity);

				if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					auto& reconciliationPhys = controllerData.reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

					debugFile << "--\n";
					debugFile << "final position: " << reconciliationPhys.GetPosition().x << "\n";
					debugFile << "final velocity: " << reconciliationPhys.GetVelocity().x << "\n";
				}
			}
		}

		// Apply back predicted entities states to main world
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			if (controllerData.controlledEntity)
			{
				assert(controllerData.reconciliationEntity);

				if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					auto& realPhys = controllerData.controlledEntity->GetComponent<Ndk::PhysicsComponent2D>();
					auto& reconciliationPhys = controllerData.reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

					Nz::Vector2f positionError = realPhys.GetPosition() - reconciliationPhys.GetPosition();


					debugFile << "position error: " << positionError.GetLength() << "\n" << std::endl;

					std::cout << "POSITION ERROR: " << positionError.GetLength() << std::endl;
					if (positionError.GetSquaredLength() < Nz::IntegralPow(100, 2))
					{
						auto serverEntry = m_serverEntityIdToClient.find(controllerData.controlledEntityServerId);
						assert(serverEntry != m_serverEntityIdToClient.end());

						//serverEntry.value().positionError += positionError;
						realPhys.SetPosition(Nz::Lerp(realPhys.GetPosition(), reconciliationPhys.GetPosition(), 0.1f));
					}
					else
					{
						std::cout << "Teleport!" << std::endl;
						realPhys.SetPosition(reconciliationPhys.GetPosition());
					}

					realPhys.SetAngularVelocity(reconciliationPhys.GetAngularVelocity());
					realPhys.SetRotation(reconciliationPhys.GetRotation());
					realPhys.SetVelocity(reconciliationPhys.GetVelocity());
				}
				else
				{
					auto& realNode = controllerData.controlledEntity->GetComponent<Ndk::NodeComponent>();
					auto& reconciliationNode = controllerData.reconciliationEntity->GetComponent<Ndk::NodeComponent>();

					realNode.SetPosition(reconciliationNode.GetPosition());
					realNode.SetRotation(reconciliationNode.GetRotation());
				}
			}
		}
	}

	void LocalMatch::HandleTickError(Nz::UInt16 stateTick, Nz::Int32 tickError)
	{
		for (auto it = m_tickPredictions.begin(); it != m_tickPredictions.end(); ++it)
		{
			if (it->serverTick == stateTick)
			{
				m_averageTickError.InsertValue(it->tickError + tickError);
				m_tickPredictions.erase(it);
				return;
			}
		}

		std::cout << "input not found: " << stateTick << std::endl;

		//m_averageTickError.InsertValue(m_averageTickError.GetAverageValue() + tickError);

		/*std::cout << "----" << std::endl;
		std::cout << "Current tick error: " << m_tickError << std::endl;
		std::cout << "Target tick error: " << tickError << std::endl;
		m_tickError = Nz::Approach(m_tickError, m_tickError + tickError, std::abs(std::max(10, 1)));
		std::cout << "New tick error: " << m_tickError << std::endl;*/
	}
}
