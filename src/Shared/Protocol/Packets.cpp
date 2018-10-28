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

		void Serialize(PacketSerializer& serializer, CreateEntities& data)
		{
			serializer.SerializeArraySize(data.entities);

			for (auto& entity : data.entities)
			{
				bool hasMovementData;
				if (serializer.IsWriting())
					hasMovementData = entity.playerMovement.has_value();

				serializer &= hasMovementData;

				if (!serializer.IsWriting())
					entity.playerMovement.emplace();
			}

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.angularVelocity;
				serializer &= entity.linearVelocity;
				serializer &= entity.position;
				serializer &= entity.rotation;

				if (entity.playerMovement)
				{
					auto& playerMovementData = entity.playerMovement.value();
					serializer &= playerMovementData.isAirControlling;
					serializer &= playerMovementData.isFacingRight;
				}
			}
		}

		void Serialize(PacketSerializer & serializer, DeleteEntities& data)
		{
			serializer.SerializeArraySize(data.entities);
			for (auto& entity : data.entities)
				serializer &= entity.id;
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
				if (serializer.IsWriting())
					hasMovementData = entity.playerMovement.has_value();

				serializer &= hasMovementData;

				if (!serializer.IsWriting() && hasMovementData)
					entity.playerMovement.emplace();
			}

			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.angularVelocity;
				serializer &= entity.linearVelocity;
				serializer &= entity.position;
				serializer &= entity.rotation;

				if (entity.playerMovement)
				{
					auto& playerMovementData = entity.playerMovement.value();
					serializer &= playerMovementData.isAirControlling;
					serializer &= playerMovementData.isFacingRight;
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

		void Serialize(PacketSerializer& serializer, PlayerInput& data)
		{
			serializer &= data.isJumping;
			serializer &= data.isMovingLeft;
			serializer &= data.isMovingRight;
		}
	}
}
