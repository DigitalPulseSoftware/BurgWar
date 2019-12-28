// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NETWORK_PACKETS_HPP
#define BURGWAR_CORELIB_NETWORK_PACKETS_HPP

#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <CoreLib/Protocol/CompressedInteger.hpp>
#include <CoreLib/Protocol/PacketSerializer.hpp>
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
		ChatMessage,
		ClientAssetList,
		ClientScriptList,
		ConsoleAnswer,
		ControlEntity,
		CreateEntities,
		DeleteEntities,
		DisableLayer,
		DownloadClientScriptRequest,
		DownloadClientScriptResponse,
		EnableLayer,
		EntitiesAnimation,
		EntitiesDeath,
		EntitiesInputs,
		EntityWeapon,
		InputTimingCorrection,
		HealthUpdate,
		MatchData,
		MatchState,
		NetworkStrings,
		PlayerChat,
		PlayerConsoleCommand,
		PlayerLayer,
		PlayersInput,
		PlayerSelectWeapon,
		PlayerWeapons,
		Ready,
	};

	template<PacketType PT> struct PacketTag
	{
		static constexpr PacketType Type = PT;
	};

	namespace Packets
	{
		namespace Helper
		{
			struct EntityId
			{
				CompressedUnsigned<Nz::UInt16> layerId;
				CompressedUnsigned<Nz::UInt32> entityId;
			};

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

			struct Properties
			{
				using PropertyValue = std::variant<
					std::vector<bool>,
					std::vector<float>,
					std::vector<Nz::Int64>,
					std::vector<Nz::Vector2f>,
					std::vector<Nz::Vector2i64>,
					std::vector<Nz::Vector3f>,
					std::vector<Nz::Vector3i64>,
					std::vector<Nz::Vector4f>,
					std::vector<Nz::Vector4i64>,
					std::vector<std::string>
				>;

				CompressedUnsigned<Nz::UInt32> name;
				PropertyValue value;
				bool isArray;
			};

			struct EntityData
			{
				CompressedUnsigned<Nz::UInt32> entityClass;
				CompressedUnsigned<Nz::UInt64> uniqueId;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<std::string> name;
				std::optional<CompressedUnsigned<Nz::UInt32>> parentId;
				std::optional<HealthData> health;
				std::optional<PlayerInputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
				std::vector<Properties> properties;
			};
		}

#define DeclarePacket(Type) struct Type : PacketTag<PacketType:: Type >

		DeclarePacket(Auth)
		{
			struct Player
			{
				std::string nickname;
			};

			std::vector<Player> players;
		};

		DeclarePacket(AuthFailure)
		{
		};

		DeclarePacket(AuthSuccess)
		{
		};

		DeclarePacket(ChatMessage)
		{
			Nz::UInt8 playerIndex;
			std::string playerName; //< Temporary
			std::string content;
		};
		
		DeclarePacket(ClientAssetList)
		{
			struct Asset
			{
				std::array<Nz::UInt8, 20> sha1Checksum;
				std::string path;
				CompressedUnsigned<Nz::UInt64> size;
			};

			std::vector<Asset> assets;
			std::vector<std::string> fastDownloadUrls;
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

		DeclarePacket(ConsoleAnswer)
		{
			Nz::UInt8 playerIndex;
			std::string response;
			Nz::Color color;
		};

		DeclarePacket(ControlEntity)
		{
			Nz::UInt16 stateTick;
			Nz::UInt8 playerIndex;
			CompressedUnsigned<LayerIndex> layerIndex;
			CompressedUnsigned<Nz::UInt32> entityId;
		};

		DeclarePacket(CreateEntities)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Helper::EntityData data;
			};

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(DeleteEntities)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
			};

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(DisableLayer)
		{
			Nz::UInt16 stateTick;
			CompressedUnsigned<LayerIndex> layerIndex;
		};

		DeclarePacket(DownloadClientScriptRequest)
		{
			std::string path;
		};

		DeclarePacket(DownloadClientScriptResponse)
		{
			std::vector<Nz::UInt8> fileContent;
		};

		DeclarePacket(EnableLayer)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Helper::EntityData data;
			};

			Nz::UInt16 stateTick;
			CompressedUnsigned<LayerIndex> layerIndex;
			std::vector<Entity> layerEntities;
		};

		DeclarePacket(EntitiesAnimation)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> entityId;
				Nz::UInt8 animId;
			};

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(EntitiesDeath)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
			};

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(EntitiesInputs)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				PlayerInputData inputs;
			};

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(EntityWeapon)
		{
			Nz::UInt16 stateTick;
			CompressedUnsigned<LayerIndex> layerIndex;
			CompressedUnsigned<Nz::UInt32> entityId;
			CompressedUnsigned<Nz::UInt32> weaponEntityId;

			static constexpr Nz::UInt32 NoWeapon = 0xFFFFFFFF;
		};

		DeclarePacket(HealthUpdate)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				Nz::UInt16 currentHealth;
			};

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(InputTimingCorrection)
		{
			Nz::UInt16 serverTick;
			CompressedSigned<Nz::Int32> tickError;
		};

		DeclarePacket(MatchData)
		{
			struct Asset
			{
				std::array<Nz::UInt8, 20> sha1Checksum;
				std::string path;
				CompressedUnsigned<Nz::UInt64> size;
			};

			struct Layer
			{
				Nz::Color backgroundColor;
			};

			struct Script
			{
				std::array<Nz::UInt8, 20> sha1Checksum;
				std::string path;
			};

			std::vector<std::string> fastDownloadUrls;
			std::vector<Asset> assets;
			std::vector<Layer> layers;
			std::vector<Script> scripts;
			std::string gamemodePath;
			Nz::UInt16 currentTick;
			float tickDuration;
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

			struct Layer
			{
				CompressedUnsigned<LayerIndex> layerIndex;
				CompressedUnsigned<Nz::UInt32> entityCount;
			};

			Nz::UInt16 stateTick;
			std::vector<Entity> entities;
			std::vector<Layer> layers;
		};

		DeclarePacket(NetworkStrings)
		{
			CompressedUnsigned<Nz::UInt32> startId;
			std::vector<std::string> strings;
		};

		DeclarePacket(PlayerChat)
		{
			Nz::UInt8 playerIndex;
			std::string message;
		};

		DeclarePacket(PlayerConsoleCommand)
		{
			Nz::UInt8 playerIndex;
			std::string command;
		};

		DeclarePacket(PlayerLayer)
		{
			Nz::UInt16 stateTick;
			Nz::UInt8 playerIndex;
			CompressedUnsigned<LayerIndex> layerIndex;
		};

		DeclarePacket(PlayersInput)
		{
			Nz::UInt16 estimatedServerTick;
			std::vector<std::optional<PlayerInputData>> inputs;
		};

		DeclarePacket(PlayerSelectWeapon)
		{
			Nz::UInt8 playerIndex;
			Nz::UInt8 newWeaponIndex;

			static constexpr std::size_t NoWeapon = 0xFF;
		};

		DeclarePacket(PlayerWeapons)
		{
			Nz::UInt16 stateTick;
			Nz::UInt8 playerIndex;
			CompressedUnsigned<LayerIndex> layerIndex;
			std::vector<CompressedUnsigned<Nz::UInt32>> weaponEntities;
		};

		DeclarePacket(Ready)
		{
		};

#undef DeclarePacket

		// Packets serializer
		void Serialize(PacketSerializer& serializer, Auth& data);
		void Serialize(PacketSerializer& serializer, AuthFailure& data);
		void Serialize(PacketSerializer& serializer, AuthSuccess& data);
		void Serialize(PacketSerializer& serializer, ChatMessage& data);
		void Serialize(PacketSerializer& serializer, ClientAssetList& data);
		void Serialize(PacketSerializer& serializer, ClientScriptList& data);
		void Serialize(PacketSerializer& serializer, ConsoleAnswer& data);
		void Serialize(PacketSerializer& serializer, ControlEntity& data);
		void Serialize(PacketSerializer& serializer, CreateEntities& data);
		void Serialize(PacketSerializer& serializer, DeleteEntities& data);
		void Serialize(PacketSerializer& serializer, DisableLayer& data);
		void Serialize(PacketSerializer& serializer, DownloadClientScriptRequest& data);
		void Serialize(PacketSerializer& serializer, DownloadClientScriptResponse& data);
		void Serialize(PacketSerializer& serializer, EnableLayer& data);
		void Serialize(PacketSerializer& serializer, EntitiesAnimation& data);
		void Serialize(PacketSerializer& serializer, EntitiesDeath& data);
		void Serialize(PacketSerializer& serializer, EntitiesInputs& data);
		void Serialize(PacketSerializer& serializer, EntityWeapon& data);
		void Serialize(PacketSerializer& serializer, HealthUpdate& data);
		void Serialize(PacketSerializer& serializer, InputTimingCorrection& data);
		void Serialize(PacketSerializer& serializer, MatchData& data);
		void Serialize(PacketSerializer& serializer, MatchState& data);
		void Serialize(PacketSerializer& serializer, NetworkStrings& data);
		void Serialize(PacketSerializer& serializer, PlayerChat& data);
		void Serialize(PacketSerializer& serializer, PlayerConsoleCommand& data);
		void Serialize(PacketSerializer& serializer, PlayerLayer& data);
		void Serialize(PacketSerializer& serializer, PlayersInput& data);
		void Serialize(PacketSerializer& serializer, PlayerSelectWeapon& data);
		void Serialize(PacketSerializer& serializer, PlayerWeapons& data);
		void Serialize(PacketSerializer& serializer, Ready& data);

		// Helpers
		void Serialize(PacketSerializer& serializer, PlayerInputData& data);
		void Serialize(PacketSerializer& serializer, Helper::EntityData& data);
	}
}

#include <CoreLib/Protocol/Packets.inl>

#endif
