// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_PACKETS_HPP
#define EREWHON_SHARED_NETWORK_PACKETS_HPP

#include <Shared/Protocol/CompressedInteger.hpp>
#include <Shared/Protocol/PacketSerializer.hpp>
#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Box.hpp>
#include <Nazara/Math/Quaternion.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <array>
#include <variant>
#include <vector>

namespace bw
{
	enum class PacketType
	{
		Auth,
		AuthFailure,
		AuthSuccess,
		CreateEntities,
		DeleteEntities,
		HelloWorld,
		MatchData,
		MatchState,
		NetworkStrings
	};

	template<PacketType PT> struct PacketTag
	{
		static constexpr PacketType Type = PT;
	};

	namespace Packets
	{
#define DeclarePacket(Type) struct Type : PacketTag<PacketType:: Type >

		DeclarePacket(Auth)
		{
			Nz::UInt8 playerCount;
		};

		DeclarePacket(AuthFailure)
		{
		};

		DeclarePacket(AuthSuccess)
		{
		};

		DeclarePacket(CreateEntities)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Nz::RadianAnglef angularVelocity;
				Nz::RadianAnglef rotation;
				Nz::Vector2f linearVelocity;
				Nz::Vector2f position;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(DeleteEntities)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
			};

			std::vector<Entity> entityIds;
		};

		DeclarePacket(HelloWorld)
		{
			std::string str;
		};

		DeclarePacket(MatchData)
		{
			std::vector<Nz::UInt8> tiles; //< 0 = empty, 1 = dirt, 2 = dirt w/ grass
			Nz::Color backgroundColor;
			Nz::UInt16 height;
			Nz::UInt16 width;
			float tileSize;
		};

		DeclarePacket(MatchState)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Nz::RadianAnglef angularVelocity;
				Nz::RadianAnglef rotation;
				Nz::Vector2f linearVelocity;
				Nz::Vector2f position;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(NetworkStrings)
		{
			CompressedUnsigned<Nz::UInt32> startId;
			std::vector<std::string> strings;
		};

#undef DeclarePacket

		void Serialize(PacketSerializer& serializer, Auth& data);
		void Serialize(PacketSerializer& serializer, AuthFailure& data);
		void Serialize(PacketSerializer& serializer, AuthSuccess& data);
		void Serialize(PacketSerializer& serializer, CreateEntities& data);
		void Serialize(PacketSerializer& serializer, DeleteEntities& data);
		void Serialize(PacketSerializer& serializer, HelloWorld& data);
		void Serialize(PacketSerializer& serializer, MatchData& data);
		void Serialize(PacketSerializer& serializer, MatchState& data);
		void Serialize(PacketSerializer& serializer, NetworkStrings& data);
	}
}

#include <Shared/Protocol/Packets.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETS_HPP
