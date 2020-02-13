// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Protocol/Packets.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <CoreLib/Utils.hpp>
#include <cassert>

namespace bw
{
	namespace Packets
	{
		std::size_t EstimateSize(const MatchState& matchState)
		{
			std::size_t size = 0;
			
			size += sizeof(MatchState::stateTick);

			size += sizeof(Nz::UInt8); // layer count
			size += (sizeof(MatchState::Layer::layerIndex) + sizeof(MatchState::Layer::entityCount)) * matchState.layers.size();

			// entity property bit size (2 bits per entity), rounded up
			size += (matchState.entities.size() * 2 + 7) / 8;

			size += (sizeof(MatchState::Entity::id) + sizeof(MatchState::Entity::position) + sizeof(MatchState::Entity::rotation)) * matchState.entities.size();

			std::size_t playerEntity = 0;
			std::size_t physicalEntity = 0;

			for (auto& entity : matchState.entities)
			{
				if (entity.playerMovement)
					playerEntity++;

				if (entity.physicsProperties)
					physicalEntity++;
			}

			size += (playerEntity + 7) / 8; // one bit per player entity, rounded up
			size += (sizeof(MatchState::PhysicsProperties::angularVelocity) + sizeof(MatchState::PhysicsProperties::linearVelocity)) * physicalEntity;

			return size;
		}

		void Serialize(PacketSerializer& serializer, Auth& data)
		{
			serializer.SerializeArraySize(data.players);

			for (auto& player : data.players)
				serializer &= player.nickname;
		}

		void Serialize(PacketSerializer& /*serializer*/, AuthFailure& /*data*/)
		{
		}

		void Serialize(PacketSerializer& /*serializer*/, AuthSuccess& /*data*/)
		{
		}

		void Serialize(PacketSerializer& serializer, ChatMessage& data)
		{
			serializer &= data.localIndex;
			serializer &= data.playerName;
			serializer &= data.content;
		}

		void Serialize(PacketSerializer& serializer, ClientAssetList& data)
		{
			serializer.SerializeArraySize(data.assets);
			serializer.SerializeArraySize(data.fastDownloadUrls);

			for (auto& downloadUrl : data.fastDownloadUrls)
				serializer &= downloadUrl;

			for (auto& script : data.assets)
			{
				serializer &= script.path;
				serializer &= script.size;

				if (serializer.IsWriting())
					serializer.Write(script.sha1Checksum.data(), script.sha1Checksum.size());
				else
					serializer.Read(script.sha1Checksum.data(), script.sha1Checksum.size());
			}
		}

		void Serialize(PacketSerializer& serializer, ClientScriptList& data)
		{
			serializer.SerializeArraySize(data.scripts);

			for (auto& script : data.scripts)
			{
				serializer &= script.path;

				if (serializer.IsWriting())
					serializer.Write(script.sha1Checksum.data(), script.sha1Checksum.size());
				else
					serializer.Read(script.sha1Checksum.data(), script.sha1Checksum.size());
			}
		}

		void Serialize(PacketSerializer& serializer, ConsoleAnswer& data)
		{
			serializer &= data.localIndex;
			serializer &= data.response;
			serializer &= data.color;
		}

		void Serialize(PacketSerializer& serializer, CreateEntities& data)
		{
			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				Serialize(serializer, entity.data);
			}
		}

		void Serialize(PacketSerializer& serializer, ControlEntity& data)
		{
			serializer &= data.stateTick;
			serializer &= data.localIndex;
			serializer &= data.layerIndex;
			serializer &= data.entityId;
		}

		void Serialize(PacketSerializer& serializer, DeleteEntities& data)
		{
			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
			}
		}

		void Serialize(PacketSerializer& serializer, DisableLayer& data)
		{
			serializer &= data.stateTick;
			serializer &= data.layerIndex;
		}

		void Serialize(PacketSerializer& serializer, DownloadClientScriptRequest& data)
		{
			serializer &= data.path;
		}

		void Serialize(PacketSerializer& serializer, DownloadClientScriptResponse& data)
		{
			serializer.SerializeArraySize(data.fileContent);
			if (serializer.IsWriting())
				serializer.Write(data.fileContent.data(), data.fileContent.size());
			else
				serializer.Read(data.fileContent.data(), data.fileContent.size());
		}

		void Serialize(PacketSerializer& serializer, EnableLayer& data)
		{
			serializer &= data.stateTick;
			serializer &= data.layerIndex;

			serializer.SerializeArraySize(data.layerEntities);
			for (auto& entity : data.layerEntities)
			{
				serializer &= entity.id;
				Serialize(serializer, entity.data);
			}
		}

		void Serialize(PacketSerializer& serializer, EntitiesAnimation& data)
		{
			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.entityId;
				serializer &= entity.animId;
			}
		}

		void Serialize(PacketSerializer& serializer, EntitiesDeath& data)
		{
			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
			}
		}

		void Serialize(PacketSerializer& serializer, EntitiesInputs& data)
		{
			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				Serialize(serializer, entity.inputs);
			}
		}

		void Serialize(PacketSerializer& serializer, EntityWeapon& data)
		{
			serializer &= data.layerIndex;
			serializer &= data.entityId;
			serializer &= data.stateTick;
			serializer &= data.weaponEntityId;
		}

		void Serialize(PacketSerializer& serializer, HealthUpdate& data)
		{
			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.currentHealth;
			}
		}

		void Serialize(PacketSerializer& serializer, InputTimingCorrection& data)
		{
			serializer &= data.serverTick;
			serializer &= data.tickError;
		}

		void Serialize(PacketSerializer& serializer, MatchData& data)
		{
			serializer &= data.currentTick;
			serializer &= data.tickDuration;
			serializer &= data.gamemodePath;

			serializer.SerializeArraySize(data.assets);
			serializer.SerializeArraySize(data.fastDownloadUrls);

			for (auto& downloadUrl : data.fastDownloadUrls)
				serializer &= downloadUrl;

			for (auto& script : data.assets)
			{
				serializer &= script.path;
				serializer &= script.size;

				if (serializer.IsWriting())
					serializer.Write(script.sha1Checksum.data(), script.sha1Checksum.size());
				else
					serializer.Read(script.sha1Checksum.data(), script.sha1Checksum.size());
			}

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
				serializer &= layer.backgroundColor;

			serializer.SerializeArraySize(data.scripts);

			for (auto& script : data.scripts)
			{
				serializer &= script.path;

				if (serializer.IsWriting())
					serializer.Write(script.sha1Checksum.data(), script.sha1Checksum.size());
				else
					serializer.Read(script.sha1Checksum.data(), script.sha1Checksum.size());
			}
		}

		void Serialize(PacketSerializer& serializer, MatchState& data)
		{
			// Don't forget to update EstimateSize(const MatchState&)

			serializer &= data.stateTick;

			Nz::UInt32 entityCount = 0;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.layerIndex;
				serializer &= layer.entityCount;

				entityCount += layer.entityCount;
			}

			if (serializer.IsWriting())
				assert(data.entities.size() == entityCount);
			else
				data.entities.resize(entityCount);

			for (auto& entity : data.entities)
			{
				bool hasMovementData;
				bool hasPhysicsProps;
				if (serializer.IsWriting())
				{
					hasMovementData = entity.playerMovement.has_value();
					hasPhysicsProps = entity.physicsProperties.has_value();
				}

				serializer &= hasMovementData;
				serializer &= hasPhysicsProps;

				if (!serializer.IsWriting())
				{
					if (hasMovementData)
						entity.playerMovement.emplace();

					if (hasPhysicsProps)
						entity.physicsProperties.emplace();
				}

				if (entity.playerMovement)
				{
					auto& playerMovementData = entity.playerMovement.value();
					serializer &= playerMovementData.isFacingRight;
				}
			}

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.position;
				serializer &= entity.rotation;

				if (entity.physicsProperties)
				{
					auto& physicsProperties = entity.physicsProperties.value();
					serializer &= physicsProperties.angularVelocity;
					serializer &= physicsProperties.linearVelocity;
				}
			}
		}

		void Serialize(PacketSerializer& serializer, NetworkStrings& data)
		{
			serializer &= data.startId;

			serializer.SerializeArraySize(data.strings);
			for (auto& string : data.strings)
				serializer &= string;
		}

		void Serialize(PacketSerializer& serializer, PlayerChat& data)
		{
			serializer &= data.localIndex;
			serializer &= data.message;
		}

		void Serialize(PacketSerializer& serializer, PlayerConsoleCommand& data)
		{
			serializer &= data.localIndex;
			serializer &= data.command;
		}

		void Serialize(PacketSerializer& serializer, PlayerLayer& data)
		{
			serializer &= data.stateTick;
			serializer &= data.localIndex;
			serializer &= data.layerIndex;
		}

		void Serialize(PacketSerializer& serializer, PlayersInput& data)
		{
			serializer &= data.estimatedServerTick;

			serializer.SerializeArraySize(data.inputs);

			for (auto& input : data.inputs)
			{
				bool hasInput;
				if (serializer.IsWriting())
					hasInput = input.has_value();

				serializer &= hasInput;

				if (!serializer.IsWriting() && hasInput)
					input.emplace();
			}

			for (auto& input : data.inputs)
			{
				if (!input.has_value())
					continue;

				Serialize(serializer, *input);
			}
		}

		void Serialize(PacketSerializer& serializer, PlayerSelectWeapon& data)
		{
			serializer &= data.localIndex;
			serializer &= data.newWeaponIndex;
		}

		void Serialize(PacketSerializer& serializer, PlayerWeapons& data)
		{
			serializer &= data.stateTick;
			serializer &= data.localIndex;
			serializer &= data.layerIndex;
			serializer.Serialize(data.weaponEntities);
		}

		void Serialize(PacketSerializer& /*serializer*/, Ready& /*data*/)
		{
		}

		void Serialize(PacketSerializer& serializer, PlayerInputData& input)
		{
			serializer &= input.isAttacking;
			serializer &= input.isCrouching;
			serializer &= input.isLookingRight;
			serializer &= input.isJumping;
			serializer &= input.isMovingLeft;
			serializer &= input.isMovingRight;

			serializer &= input.aimDirection;
		}

		void Serialize(PacketSerializer& serializer, Helper::EntityData& data)
		{
			bool hasHealth;
			bool hasInputs;
			bool hasParent;
			bool hasMovementData;
			bool hasPhysicsProps;
			bool hasName;

			if (serializer.IsWriting())
			{
				hasHealth = data.health.has_value();
				hasInputs = data.inputs.has_value();
				hasParent = data.parentId.has_value();
				hasMovementData = data.playerMovement.has_value();
				hasPhysicsProps = data.physicsProperties.has_value();
				hasName = data.name.has_value();
			}

			serializer &= hasHealth;
			serializer &= hasInputs;
			serializer &= hasParent;
			serializer &= hasMovementData;
			serializer &= hasPhysicsProps;
			serializer &= hasName;

			if (!serializer.IsWriting())
			{
				if (hasHealth)
					data.health.emplace();

				if (hasInputs)
					data.inputs.emplace();

				if (hasParent)
					data.parentId.emplace();

				if (hasMovementData)
					data.playerMovement.emplace();

				if (hasPhysicsProps)
					data.physicsProperties.emplace();

				if (hasName)
					data.name.emplace();
			}

			serializer &= data.entityClass;
			serializer &= data.uniqueId;
			serializer &= data.position;
			serializer &= data.rotation;

			if (data.health)
			{
				auto& healthProperties = data.health.value();
				serializer &= healthProperties.currentHealth;
				serializer &= healthProperties.maxHealth;
			}

			if (data.inputs)
				Serialize(serializer, data.inputs.value());

			if (data.name)
				serializer &= data.name.value();

			if (data.parentId)
				serializer &= data.parentId.value();

			if (data.playerMovement)
			{
				auto& playerMovementData = data.playerMovement.value();
				serializer &= playerMovementData.isFacingRight;
			}

			if (data.physicsProperties)
			{
				auto& physicsProperties = data.physicsProperties.value();
				serializer &= physicsProperties.angularVelocity;
				serializer &= physicsProperties.linearVelocity;
			}

			serializer.SerializeArraySize(data.properties);
			for (auto& property : data.properties)
			{
				serializer &= property.name;

				// Serialize type
				Nz::UInt8 dataType;
				if (serializer.IsWriting())
					dataType = static_cast<Nz::UInt8>(property.value.index());

				serializer &= dataType;
				serializer &= property.isArray;

				// Read/write value

				// Waiting for template lambda in C++20
				auto SerializeValue = [&](auto dummyType)
				{
					using T = std::decay_t<decltype(dummyType)>;

					auto& elements = (serializer.IsWriting()) ? std::get<std::vector<T>>(property.value) : property.value.emplace<std::vector<T>>();

					if (property.isArray)
					{
						serializer.SerializeArraySize(elements);
						for (auto& element : elements)
							serializer &= element;
					}
					else
					{
						assert(!serializer.IsWriting() || elements.size() == 1);
						if (!serializer.IsWriting())
							elements.resize(1);

						serializer &= elements.front();
					}
				};


				static_assert(std::variant_size_v<Helper::Properties::PropertyValue> == 10);
				switch (dataType)
				{
					case 0:
					{
						// Handle std::vector<bool> specialization
						auto& elements = (serializer.IsWriting()) ? std::get<std::vector<bool>>(property.value) : property.value.emplace<std::vector<bool>>();

						serializer.SerializeArraySize(elements);
						if (serializer.IsWriting())
						{
							for (bool val : elements)
								serializer &= val;
						}
						else
						{
							for (std::size_t i = 0; i < elements.size(); ++i)
							{
								bool val;
								serializer &= val;

								elements[i] = val;
							}
						}

						break;
					}

					case 1: SerializeValue(float()); break;
					case 2: SerializeValue(Nz::Int64()); break;
					case 3: SerializeValue(Nz::Vector2f()); break;
					case 4: SerializeValue(Nz::Vector2i64()); break;
					case 5: SerializeValue(Nz::Vector3f()); break;
					case 6: SerializeValue(Nz::Vector3i64()); break;
					case 7: SerializeValue(Nz::Vector4f()); break;
					case 8: SerializeValue(Nz::Vector4i64()); break;

					case 9: // std::string
					{
						auto& elements = (serializer.IsWriting()) ? std::get<std::vector<std::string>>(property.value) : property.value.emplace<std::vector<std::string>>();

						serializer.SerializeArraySize(elements);
						if (serializer.IsWriting())
						{
							for (const auto& element : elements)
							{
								serializer.SerializeArraySize(element);
								serializer.Write(element.data(), element.size());
							}
						}
						else
						{
							for (auto& element : elements)
							{
								serializer.SerializeArraySize(element);
								serializer.Read(element.data(), element.size());
							}
						}
						break;
					}

					default:
						assert(!"Unexpected datatype");
						break;
				}
			}
		}
	}
}
