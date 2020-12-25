// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Protocol/Packets.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Nazara/Math/Vector4.hpp>
#include <CoreLib/Utils.hpp>
#include <cassert>

namespace bw
{
	namespace Packets
	{
		std::size_t EstimateSize(const MatchState& matchState)
		{
			std::size_t size = 0;
			
			size += sizeof(MatchState::lastInputTick);
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

		void Serialize(PacketSerializer& serializer, AuthSuccess& data)
		{
			serializer.SerializeArraySize(data.players);

			for (auto& player : data.players)
				serializer &= player.playerIndex;
		}

		void Serialize(PacketSerializer& serializer, ChatMessage& data)
		{
			serializer &= data.localIndex;
			serializer &= data.playerIndex;
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

		void Serialize(PacketSerializer& serializer, DownloadClientFileFragment& data)
		{
			serializer &= data.fragmentIndex;
			serializer.SerializeArraySize(data.fragmentContent);
			if (serializer.IsWriting())
				serializer.Write(data.fragmentContent.data(), data.fragmentContent.size());
			else
				serializer.Read(data.fragmentContent.data(), data.fragmentContent.size());
		}

		void Serialize(PacketSerializer& serializer, DownloadClientFileRequest& data)
		{
			serializer &= data.path;
		}

		void Serialize(PacketSerializer& serializer, DownloadClientFileResponse& data)
		{
			static_assert(std::is_same_v<std::variant_alternative_t<0, DownloadClientFileResponse::SuccessFailureVariant>, DownloadClientFileResponse::Success>);
			static_assert(std::is_same_v<std::variant_alternative_t<1, DownloadClientFileResponse::SuccessFailureVariant>, DownloadClientFileResponse::Failure>);

			Nz::UInt8 type;
			if (serializer.IsWriting())
				type = static_cast<Nz::UInt8>(data.content.index());

			serializer &= type;
			if (!serializer.IsWriting())
			{
				switch (type)
				{
					case 0:
					{
						data.content.emplace<DownloadClientFileResponse::Success>();
						break;
					}

					case 1:
					{
						data.content.emplace<DownloadClientFileResponse::Failure>();
						break;
					}
				}
			}
			
			switch (type)
			{
				case 0:
				{
					auto& success = std::get<DownloadClientFileResponse::Success>(data.content);
					serializer &= success.fragmentCount;
					serializer &= success.fragmentSize;
					break;
				}

				case 1:
				{
					auto& failure = std::get<DownloadClientFileResponse::Failure>(data.content);
					serializer.SerializeEnum(failure.error);
					break;
				}
			}
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

		void Serialize(PacketSerializer& serializer, EntitiesScale& data)
		{
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
				serializer &= entity.newScale;
			}
		}

		void Serialize(PacketSerializer& serializer, EntityPhysics& data)
		{
			Serialize(serializer, data.entityId);
			serializer &= data.stateTick;
			serializer &= data.asleep;
			serializer &= data.mass;
			serializer &= data.momentOfInertia;

			bool hasPlayerMovement;
			if (serializer.IsWriting())
				hasPlayerMovement = data.playerMovement.has_value();

			serializer &= hasPlayerMovement;
			if (!serializer.IsWriting())
			{
				if (hasPlayerMovement)
					data.playerMovement.emplace();
			}

			if (data.playerMovement.has_value())
			{
				auto& playerMovement = data.playerMovement.value();
				serializer &= playerMovement.jumpHeight;
				serializer &= playerMovement.jumpHeightBoost;
				serializer &= playerMovement.movementSpeed;
			}
		}

		void Serialize(PacketSerializer& serializer, EntityWeapon& data)
		{
			Serialize(serializer, data.entityId);
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
			serializer &= data.gamemode;

			serializer.SerializeArraySize(data.fastDownloadUrls);

			for (auto& downloadUrl : data.fastDownloadUrls)
				serializer &= downloadUrl;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
				serializer &= layer.backgroundColor;

			serializer.SerializeArraySize(data.gamemodeProperties);
			for (auto& property : data.gamemodeProperties)
				Serialize(serializer, property);

			serializer.SerializeArraySize(data.assets);

			for (auto& script : data.assets)
			{
				serializer &= script.path;
				serializer &= script.size;

				if (serializer.IsWriting())
					serializer.Write(script.sha1Checksum.data(), script.sha1Checksum.size());
				else
					serializer.Read(script.sha1Checksum.data(), script.sha1Checksum.size());
			}

			serializer.SerializeArraySize(data.scripts);

			for (auto& script : data.scripts)
			{
				serializer &= script.path;
				serializer &= script.size;

				if (serializer.IsWriting())
					serializer.Write(script.sha1Checksum.data(), script.sha1Checksum.size());
				else
					serializer.Read(script.sha1Checksum.data(), script.sha1Checksum.size());
			}
		}

		void Serialize(PacketSerializer& serializer, MatchState& data)
		{
			// Don't forget to update EstimateSize(const MatchState&)

			serializer &= data.lastInputTick;
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

		void Serialize(PacketSerializer& serializer, PlayerLeaving& data)
		{
			serializer &= data.playerIndex;
		}

		void Serialize(PacketSerializer& serializer, PlayerNameUpdate& data)
		{
			serializer &= data.playerIndex;
			serializer &= data.newName;
		}

		void Serialize(PacketSerializer& serializer, PlayerJoined& data)
		{
			serializer &= data.playerIndex;
			serializer &= data.playerName;
		}

		void Serialize(PacketSerializer& serializer, PlayerPingUpdate& data)
		{
			serializer.SerializeArraySize(data.players);

			for (auto& playerData : data.players)
			{
				serializer &= playerData.playerIndex;
				serializer &= playerData.ping;
			}
		}


		void Serialize(PacketSerializer& serializer, PlayersInput& data)
		{
			serializer &= data.estimatedServerTick;
			serializer &= data.inputTick;

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

		void Serialize(PacketSerializer& serializer, ScriptPacket& data)
		{
			serializer &= data.nameIndex;
			
			serializer.SerializeArraySize(data.content);
			if (serializer.IsWriting())
				serializer.Write(data.content.data(), data.content.size());
			else
				serializer.Read(data.content.data(), data.content.size());
		}

		void Serialize(PacketSerializer& serializer, UpdatePlayerName& data)
		{
			serializer &= data.localIndex;
			serializer &= data.newName;
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
			bool hasScale;
			bool hasHealth;
			bool hasInputs;
			bool hasParent;
			bool hasMovementData;
			bool hasPhysicsProps;
			bool hasName;

			if (serializer.IsWriting())
			{
				hasScale = data.scale.has_value();
				hasHealth = data.health.has_value();
				hasInputs = data.inputs.has_value();
				hasParent = data.parentId.has_value();
				hasMovementData = data.playerMovement.has_value();
				hasPhysicsProps = data.physicsProperties.has_value();
				hasName = data.name.has_value();
			}

			serializer &= hasScale;
			serializer &= hasHealth;
			serializer &= hasInputs;
			serializer &= hasParent;
			serializer &= hasMovementData;
			serializer &= hasPhysicsProps;
			serializer &= hasName;

			if (!serializer.IsWriting())
			{
				if (hasScale)
					data.scale.emplace();

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

			if (data.scale)
				serializer &= data.scale.value();

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
				serializer &= physicsProperties.isAsleep;
				serializer &= physicsProperties.mass;
				serializer &= physicsProperties.momentOfInertia;
			}

			serializer.SerializeArraySize(data.properties);
			for (auto& property : data.properties)
				Serialize(serializer, property);
		}

		void Serialize(PacketSerializer& serializer, Helper::EntityId& data)
		{
			serializer &= data.layerId;
			serializer &= data.entityId;
		}
		
		void Serialize(PacketSerializer& serializer, Helper::Property& data)
		{
			serializer &= data.name;

			if (serializer.IsWriting())
			{
				auto [internalType, isArray] = ExtractPropertyType(data.value);

				Nz::UInt8 propertyType = Nz::UInt8(internalType);
				serializer &= propertyType;

				serializer &= Nz::UInt8((isArray) ? 1 : 0);

				std::visit([&](auto&& propertyValue)
				{
					using T = std::decay_t<decltype(propertyValue)>;
					using TypeExtractor = PropertyTypeExtractor<T>;
					constexpr bool IsArray = TypeExtractor::IsArray;

					if constexpr (IsArray)
					{
						CompressedUnsigned<Nz::UInt32> arraySize(Nz::UInt32(propertyValue.size()));
						serializer.Serialize(arraySize);

						for (auto& element : propertyValue)
							serializer &= element;
					}
					else
						serializer &= propertyValue.value;

				}, data.value);
			}
			else
			{
				Nz::UInt8 propertyTypeInt;
				serializer &= propertyTypeInt;

				PropertyType propertyType = static_cast<PropertyType>(propertyTypeInt);

				Nz::UInt8 isArrayInt;
				serializer &= isArrayInt;

				bool isArray = (isArrayInt != 0);

				// Waiting for template lambda in C++20
				auto Unserialize = [&](auto dummyType)
				{
					using T = std::decay_t<decltype(dummyType)>;

					static constexpr PropertyType Property = T::Property;

					if (isArray)
					{
						CompressedUnsigned<Nz::UInt32> size;
						serializer &= size;

						auto& elements = data.value.emplace<PropertyArrayValue<Property>>(size);
						for (auto& element : elements)
							serializer &= element;
					}
					else
					{
						auto& value = data.value.emplace<PropertySingleValue<Property>>();
						serializer &= value.value;
					}
				};

				switch (propertyType)
				{
#define BURGWAR_PROPERTYTYPE(V, T, UT) case PropertyType:: T: Unserialize(PropertyTag<PropertyType:: T>{}); break;

#include <CoreLib/PropertyTypeList.hpp>
				}
			}
		}
	}
}
