// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SHARED_NETWORK_PACKETS_HPP
#define EREWHON_SHARED_NETWORK_PACKETS_HPP

#include <Shared/InputData.hpp>
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
#include <optional>
#include <variant>
#include <vector>

namespace bw
{
	enum class PacketType
	{
		Auth,
		AuthFailure,
		AuthSuccess,
		ClientScriptList,
		ControlEntity,
		CreateEntities,
		DeleteEntities,
		DownloadClientScriptRequest,
		DownloadClientScriptResponse,
		EntitiesInputs,
		HealthUpdate,
		HelloWorld,
		MatchData,
		MatchState,
		NetworkStrings,
		PlayAnimation,
		PlayersInput,
		Ready
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

		DeclarePacket(ClientScriptList)
		{
			struct Script
			{
				std::array<Nz::UInt8, 20> sha1Checksum;
				std::string path;
			};

			std::vector<Script> scripts;
		};

		DeclarePacket(ControlEntity)
		{
			Nz::UInt8 playerIndex;
			CompressedUnsigned<Nz::UInt32> entityId;
		};

		DeclarePacket(CreateEntities)
		{
			struct HealthData
			{
				Nz::UInt16 maxHealth;
				Nz::UInt16 currentHealth;
			};

			struct PlayerMovementData
			{
				bool isFacingRight;
			};

			struct PhysicsProperties
			{
				Nz::RadianAnglef angularVelocity;
				Nz::Vector2f linearVelocity;
			};

			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				CompressedUnsigned<Nz::UInt32> entityClass;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<CompressedUnsigned<Nz::UInt32>> parentId;
				std::optional<HealthData> health;
				std::optional<InputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(DeleteEntities)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(DownloadClientScriptRequest)
		{
			std::string path;
		};

		DeclarePacket(DownloadClientScriptResponse)
		{
			std::vector<Nz::UInt8> fileContent;
		};

		DeclarePacket(EntitiesInputs)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				InputData inputs;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(HealthUpdate)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Nz::UInt16 currentHealth;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(HelloWorld)
		{
			std::string str;
		};

		DeclarePacket(MatchData)
		{
			struct Layer
			{
				Nz::UInt16 height;
				Nz::UInt16 width;
				std::vector<Nz::UInt8> tiles; //< 0 = empty, 1 = dirt, 2 = dirt w/ grass
			};

			std::vector<Layer> layers;
			Nz::Color backgroundColor;
			float tileSize;
			std::string gamemodePath;
		};

		DeclarePacket(MatchState)
		{
			struct PlayerMovementData
			{
				bool isFacingRight;
			};

			struct PhysicsProperties
			{
				Nz::RadianAnglef angularVelocity;
				Nz::Vector2f linearVelocity;
			};

			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
			};

			std::vector<Entity> entities;
		};

		DeclarePacket(NetworkStrings)
		{
			CompressedUnsigned<Nz::UInt32> startId;
			std::vector<std::string> strings;
		};

		DeclarePacket(PlayAnimation)
		{
			CompressedUnsigned<Nz::UInt32> entityId;
			Nz::UInt8 animId;
		};

		DeclarePacket(PlayersInput)
		{
			std::vector<std::optional<InputData>> inputs;
		};

		DeclarePacket(Ready)
		{
		};

#undef DeclarePacket

		// Packets serializer
		void Serialize(PacketSerializer& serializer, Auth& data);
		void Serialize(PacketSerializer& serializer, AuthFailure& data);
		void Serialize(PacketSerializer& serializer, AuthSuccess& data);
		void Serialize(PacketSerializer& serializer, ClientScriptList& data);
		void Serialize(PacketSerializer& serializer, ControlEntity& data);
		void Serialize(PacketSerializer& serializer, CreateEntities& data);
		void Serialize(PacketSerializer& serializer, DeleteEntities& data);
		void Serialize(PacketSerializer& serializer, DownloadClientScriptRequest& data);
		void Serialize(PacketSerializer& serializer, DownloadClientScriptResponse& data);
		void Serialize(PacketSerializer& serializer, EntitiesInputs& data);
		void Serialize(PacketSerializer& serializer, HealthUpdate& data);
		void Serialize(PacketSerializer& serializer, HelloWorld& data);
		void Serialize(PacketSerializer& serializer, MatchData& data);
		void Serialize(PacketSerializer& serializer, MatchState& data);
		void Serialize(PacketSerializer& serializer, NetworkStrings& data);
		void Serialize(PacketSerializer& serializer, PlayAnimation& data);
		void Serialize(PacketSerializer& serializer, PlayersInput& data);
		void Serialize(PacketSerializer& serializer, Ready& data);

		// Helpers
		void Serialize(PacketSerializer& serializer, InputData& data);
	}
}

#include <Shared/Protocol/Packets.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETS_HPP
