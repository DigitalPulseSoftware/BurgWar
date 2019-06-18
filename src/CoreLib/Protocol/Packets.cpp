// Copyright (C) 2019 Jérôme Leclercq
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
		void Serialize(PacketSerializer& serializer, Auth& data)
		{
			serializer.SerializeArraySize(data.players);

			for (auto& player : data.players)
				serializer &= player.nickname;
		}

		void Serialize(PacketSerializer& serializer, AuthFailure& data)
		{
		}

		void Serialize(PacketSerializer& serializer, AuthSuccess& data)
		{
		}

		void Serialize(PacketSerializer& serializer, ChatMessage& data)
		{
			serializer &= data.content;
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

		void Serialize(PacketSerializer& serializer, CreateEntities& data)
		{
			serializer &= data.stateTick;
			serializer.SerializeArraySize(data.entities);

			for (auto& entity : data.entities)
			{
				bool hasHealth;
				bool hasInputs;
				bool hasParent;
				bool hasMovementData;
				bool hasPhysicsProps;
				bool hasName;

				if (serializer.IsWriting())
				{
					hasHealth = entity.health.has_value();
					hasInputs = entity.inputs.has_value();
					hasParent = entity.parentId.has_value();
					hasMovementData = entity.playerMovement.has_value();
					hasPhysicsProps = entity.physicsProperties.has_value();
					hasName = entity.name.has_value();
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
						entity.health.emplace();

					if (hasInputs)
						entity.inputs.emplace();

					if (hasParent)
						entity.parentId.emplace();

					if (hasMovementData)
						entity.playerMovement.emplace();

					if (hasPhysicsProps)
						entity.physicsProperties.emplace();

					if (hasName)
						entity.name.emplace();
				}
			}

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.entityClass;
				serializer &= entity.position;
				serializer &= entity.rotation;

				if (entity.health)
				{
					auto& healthProperties = entity.health.value();
					serializer &= healthProperties.currentHealth;
					serializer &= healthProperties.maxHealth;
				}

				if (entity.inputs)
					Serialize(serializer, entity.inputs.value());

				if (entity.name)
					serializer &= entity.name.value();

				if (entity.parentId)
					serializer &= entity.parentId.value();

				if (entity.playerMovement)
				{
					auto& playerMovementData = entity.playerMovement.value();
					serializer &= playerMovementData.isFacingRight;
				}

				if (entity.physicsProperties)
				{
					auto& physicsProperties = entity.physicsProperties.value();
					serializer &= physicsProperties.angularVelocity;
					serializer &= physicsProperties.linearVelocity;
				}

				serializer.SerializeArraySize(entity.properties);
				for (auto& property : entity.properties)
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


					static_assert(std::variant_size_v<CreateEntities::Properties::PropertyValue> == 6);
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

						case 5: // std::string
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

		void Serialize(PacketSerializer& serializer, ControlEntity& data)
		{
			serializer &= data.stateTick;
			serializer &= data.playerIndex;
			serializer &= data.entityId;
		}

		void Serialize(PacketSerializer& serializer, DeleteEntities& data)
		{
			serializer &= data.stateTick;

			serializer.SerializeArraySize(data.entities);
			for (auto& entity : data.entities)
				serializer &= entity.id;
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

		void Serialize(PacketSerializer& serializer, EntitiesAnimation& data)
		{
			serializer &= data.stateTick;

			serializer.SerializeArraySize(data.entities);
			for (auto& entity : data.entities)
			{
				serializer &= entity.entityId;
				serializer &= entity.animId;
			}
		}

		void Serialize(PacketSerializer& serializer, EntitiesInputs& data)
		{
			serializer &= data.stateTick;

			serializer.SerializeArraySize(data.entities);
			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				Serialize(serializer, entity.inputs);
			}
		}

		void Serialize(PacketSerializer& serializer, EntityWeapon& data)
		{
			serializer &= data.stateTick;
			serializer &= data.entityId;
			serializer &= data.weaponEntityId;
		}

		void Serialize(PacketSerializer& serializer, HealthUpdate& data)
		{
			serializer &= data.stateTick;

			serializer.SerializeArraySize(data.entities);
			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.currentHealth;
			}
		}

		void Serialize(PacketSerializer& serializer, HelloWorld& data)
		{
			serializer &= data.str;
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

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
				serializer &= layer.backgroundColor;
		}

		void Serialize(PacketSerializer& serializer, MatchState& data)
		{
			serializer &= data.stateTick;

			serializer.SerializeArraySize(data.entities);

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
			}

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.position;
				serializer &= entity.rotation;

				if (entity.playerMovement)
				{
					auto& playerMovementData = entity.playerMovement.value();
					serializer &= playerMovementData.isFacingRight;
				}

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
			serializer &= data.playerIndex;
			serializer &= data.message;
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
			serializer &= data.playerIndex;
			serializer &= data.newWeaponIndex;
		}

		void Serialize(PacketSerializer& serializer, PlayerWeapons& data)
		{
			serializer &= data.stateTick;
			serializer &= data.playerIndex;
			serializer &= data.weaponEntities;
		}

		void Serialize(PacketSerializer& /*serializer*/, Ready& /*data*/)
		{
		}

		void Serialize(PacketSerializer& serializer, PlayerInputData& input)
		{
			serializer &= input.isAttacking;
			serializer &= input.isJumping;
			serializer &= input.isMovingLeft;
			serializer &= input.isMovingRight;

			serializer &= input.aimDirection;
		}
	}
}
