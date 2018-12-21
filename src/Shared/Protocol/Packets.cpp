// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Protocol/Packets.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Shared/Utils.hpp>
#include <cassert>

namespace bw
{
	namespace Packets
	{
		void Serialize(PacketSerializer& serializer, Auth& data)
		{
			serializer &= data.playerCount;
		}

		void Serialize(PacketSerializer& serializer, AuthFailure& data)
		{
		}

		void Serialize(PacketSerializer& serializer, AuthSuccess& data)
		{
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
			serializer.SerializeArraySize(data.entities);

			for (auto& entity : data.entities)
			{
				bool hasHealth;
				bool hasParent;
				bool hasMovementData;
				bool hasPhysicsProps;
				if (serializer.IsWriting())
				{
					hasHealth = entity.health.has_value();
					hasParent = entity.parentId.has_value();
					hasMovementData = entity.playerMovement.has_value();
					hasPhysicsProps = entity.physicsProperties.has_value();
				}

				serializer &= hasHealth;
				serializer &= hasParent;
				serializer &= hasMovementData;
				serializer &= hasPhysicsProps;

				if (!serializer.IsWriting())
				{
					if (hasHealth)
						entity.health.emplace();

					if (hasParent)
						entity.parentId.emplace();

					if (hasMovementData)
						entity.playerMovement.emplace();

					if (hasPhysicsProps)
						entity.physicsProperties.emplace();
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

				if (entity.parentId)
					serializer &= entity.parentId.value();

				if (entity.playerMovement)
				{
					auto& playerMovementData = entity.playerMovement.value();
					serializer &= playerMovementData.isAirControlling;
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

		void Serialize(PacketSerializer& serializer, ControlEntity& data)
		{
			serializer &= data.playerIndex;
			serializer &= data.entityId;
		}

		void Serialize(PacketSerializer & serializer, DeleteEntities& data)
		{
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

		void Serialize(PacketSerializer & serializer, HealthUpdate & data)
		{
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

		void Serialize(PacketSerializer& serializer, MatchData& data)
		{
			serializer &= data.backgroundColor;
			serializer &= data.tileSize;

			serializer.SerializeArraySize(data.layers);
			for (auto& layer : data.layers)
			{
				serializer &= layer.width;
				serializer &= layer.height;

				if (!serializer.IsWriting())
					layer.tiles.resize(layer.width * layer.height);

				assert(layer.tiles.size() == layer.width * layer.height);

				for (Nz::UInt8& tile : layer.tiles)
					serializer &= tile;
			}
		}

		void Serialize(PacketSerializer& serializer, MatchState& data)
		{
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
					serializer &= playerMovementData.isAirControlling;
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

		void Serialize(PacketSerializer& serializer, PlayAnimation& data)
		{
			serializer &= data.entityId;
			serializer &= data.animId;
		}

		void Serialize(PacketSerializer& serializer, PlayersInput& data)
		{
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

				serializer &= input->isAttacking;
				serializer &= input->isJumping;
				serializer &= input->isMovingLeft;
				serializer &= input->isMovingRight;
			}
		}

		void Serialize(PacketSerializer& /*serializer*/, Ready& /*data*/)
		{
		}
	}
}
