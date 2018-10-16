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
				serializer &= entity.id;
				serializer &= entity.angularVelocity;
				serializer &= entity.linearVelocity;
				serializer &= entity.position;
				serializer &= entity.rotation;
			}
		}

		void Serialize(PacketSerializer & serializer, DeleteEntities& data)
		{
			serializer.SerializeArraySize(data.entityIds);
			for (auto& entity : data.entityIds)
				serializer &= entity.id;
		}

		void Serialize(PacketSerializer& serializer, HelloWorld& data)
		{
			serializer &= data.str;
		}

		void Serialize(PacketSerializer& serializer, MatchData& data)
		{
			serializer &= data.backgroundColor;
			serializer &= data.width;
			serializer &= data.height;
			serializer &= data.tileSize;

			if (!serializer.IsWriting())
				data.tiles.resize(data.width * data.height);

			assert(data.tiles.size() == data.width * data.height);

			for (Nz::UInt8& tile : data.tiles)
				serializer &= tile;
		}

		void Serialize(PacketSerializer& serializer, MatchState& data)
		{
			serializer.SerializeArraySize(data.entities);
			for (auto& entity : data.entities)
			{
				serializer &= entity.id;
				serializer &= entity.angularVelocity;
				serializer &= entity.linearVelocity;
				serializer &= entity.position;
				serializer &= entity.rotation;
			}
		}

		void Serialize(PacketSerializer& serializer, NetworkStrings& data)
		{
			serializer &= data.startId;

			serializer.SerializeArraySize(data.strings);
			for (auto& string : data.strings)
				serializer &= string;
		}
	}
}
