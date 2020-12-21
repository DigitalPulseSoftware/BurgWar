// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NETWORK_PACKETS_HPP
#define BURGWAR_CORELIB_NETWORK_PACKETS_HPP

#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <CoreLib/PropertyValues.hpp>
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
		DownloadClientFileFragment,
		DownloadClientFileRequest,
		DownloadClientFileResponse,
		EnableLayer,
		EntitiesAnimation,
		EntitiesDeath,
		EntitiesInputs,
		EntitiesScale,
		EntityPhysics,
		EntityWeapon,
		InputTimingCorrection,
		HealthUpdate,
		MatchData,
		MatchState,
		NetworkStrings,
		PlayerChat,
		PlayerConsoleCommand,
		PlayerJoined,
		PlayerLayer,
		PlayerLeaving,
		PlayerNameUpdate,
		PlayerPingUpdate,
		PlayersInput,
		PlayerSelectWeapon,
		PlayerWeapons,
		Ready,
		ScriptPacket,
		UpdatePlayerName
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
				bool isAsleep;
				float mass;
				float momentOfInertia;
			};

			struct Property
			{
				CompressedUnsigned<Nz::UInt32> name;
				PropertyValue value;
			};

			struct EntityData
			{
				CompressedUnsigned<Nz::UInt32> entityClass;
				CompressedUnsigned<Nz::UInt64> uniqueId;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<float> scale;
				std::optional<std::string> name;
				std::optional<CompressedUnsigned<Nz::UInt32>> parentId;
				std::optional<HealthData> health;
				std::optional<PlayerInputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
				std::vector<Property> properties;
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
			struct Player
			{
				Nz::UInt16 playerIndex;
			};

			std::vector<Player> players;
		};

		DeclarePacket(ChatMessage)
		{
			Nz::UInt8 localIndex;
			CompressedUnsigned<Nz::UInt16> playerIndex = CompressedUnsigned<Nz::UInt16>(InvalidPlayer);
			std::string content;

			static constexpr Nz::UInt16 InvalidPlayer = 0xFFFF;
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
			Nz::UInt8 localIndex;
			std::string response;
			Nz::Color color;
		};

		DeclarePacket(ControlEntity)
		{
			Nz::UInt16 stateTick;
			Nz::UInt8 localIndex;
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

		DeclarePacket(DownloadClientFileFragment)
		{
			CompressedUnsigned<Nz::UInt32> fragmentIndex;
			std::vector<Nz::UInt8> fragmentContent;
		};

		DeclarePacket(DownloadClientFileRequest)
		{
			std::string path;
		};

		DeclarePacket(DownloadClientFileResponse)
		{
			enum class Error : Nz::UInt8
			{
				FileNotFound
			};

			struct Success
			{
				CompressedUnsigned<Nz::UInt32> fragmentCount;
				CompressedUnsigned<Nz::UInt64> fragmentSize;
			};

			struct Failure
			{
				Error error;
			};

			using SuccessFailureVariant = std::variant<Success, Failure>; //< TODO: bw::Option?

			SuccessFailureVariant content;
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

		DeclarePacket(EntitiesScale)
		{
			struct Entity
			{
				CompressedUnsigned<Nz::UInt32> id;
				float newScale;
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

		DeclarePacket(EntityPhysics)
		{
			struct PlayerMovement
			{
				float movementSpeed;
				float jumpHeight;
				float jumpHeightBoost;
			};

			Nz::UInt16 stateTick;
			Helper::EntityId entityId;
			bool asleep;
			float mass;
			float momentOfInertia;
			std::optional<PlayerMovement> playerMovement;
		};

		DeclarePacket(EntityWeapon)
		{
			Nz::UInt16 stateTick;
			Helper::EntityId entityId;
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

			std::string gamemode;
			std::vector<std::string> fastDownloadUrls;
			std::vector<Asset> assets;
			std::vector<Layer> layers;
			std::vector<Helper::Property> gamemodeProperties;
			std::vector<Script> scripts;
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

			Nz::UInt16 lastInputTick;
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
			Nz::UInt8 localIndex;
			std::string message;
		};

		DeclarePacket(PlayerConsoleCommand)
		{
			Nz::UInt8 localIndex;
			std::string command;
		};

		DeclarePacket(PlayerJoined)
		{
			CompressedUnsigned<Nz::UInt16> playerIndex;
			std::string playerName;
		};

		DeclarePacket(PlayerLayer)
		{
			Nz::UInt16 stateTick;
			Nz::UInt8 localIndex;
			CompressedUnsigned<LayerIndex> layerIndex;
		};

		DeclarePacket(PlayerLeaving)
		{
			CompressedUnsigned<Nz::UInt16> playerIndex;
		};

		DeclarePacket(PlayerNameUpdate)
		{
			CompressedUnsigned<Nz::UInt16> playerIndex;
			std::string newName;
		};

		DeclarePacket(PlayerPingUpdate)
		{
			struct PlayerData
			{
				CompressedUnsigned<Nz::UInt16> playerIndex;
				Nz::UInt16 ping;
			};

			std::vector<PlayerData> players;
		};

		DeclarePacket(PlayersInput)
		{
			Nz::UInt16 estimatedServerTick;
			Nz::UInt16 inputTick;
			std::vector<std::optional<PlayerInputData>> inputs;
		};

		DeclarePacket(PlayerSelectWeapon)
		{
			Nz::UInt8 localIndex;
			Nz::UInt8 newWeaponIndex;

			static constexpr std::size_t NoWeapon = 0xFF;
		};

		DeclarePacket(PlayerWeapons)
		{
			Nz::UInt16 stateTick;
			Nz::UInt8 localIndex;
			CompressedUnsigned<LayerIndex> layerIndex;
			std::vector<CompressedUnsigned<Nz::UInt32>> weaponEntities;
		};

		DeclarePacket(Ready)
		{
		};

		DeclarePacket(ScriptPacket)
		{
			CompressedUnsigned<Nz::UInt32> nameIndex;
			std::vector<Nz::UInt8> content;
		};

		DeclarePacket(UpdatePlayerName)
		{
			Nz::UInt8 localIndex;
			std::string newName;
		};

#undef DeclarePacket

		// Compute size
		std::size_t EstimateSize(const MatchState& matchState);

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
		void Serialize(PacketSerializer& serializer, DownloadClientFileFragment& data);
		void Serialize(PacketSerializer& serializer, DownloadClientFileRequest& data);
		void Serialize(PacketSerializer& serializer, DownloadClientFileResponse& data);
		void Serialize(PacketSerializer& serializer, EnableLayer& data);
		void Serialize(PacketSerializer& serializer, EntitiesAnimation& data);
		void Serialize(PacketSerializer& serializer, EntitiesDeath& data);
		void Serialize(PacketSerializer& serializer, EntitiesInputs& data);
		void Serialize(PacketSerializer& serializer, EntitiesScale& data);
		void Serialize(PacketSerializer& serializer, EntityPhysics& data);
		void Serialize(PacketSerializer& serializer, EntityWeapon& data);
		void Serialize(PacketSerializer& serializer, HealthUpdate& data);
		void Serialize(PacketSerializer& serializer, InputTimingCorrection& data);
		void Serialize(PacketSerializer& serializer, MatchData& data);
		void Serialize(PacketSerializer& serializer, MatchState& data);
		void Serialize(PacketSerializer& serializer, NetworkStrings& data);
		void Serialize(PacketSerializer& serializer, PlayerChat& data);
		void Serialize(PacketSerializer& serializer, PlayerConsoleCommand& data);
		void Serialize(PacketSerializer& serializer, PlayerJoined& data);
		void Serialize(PacketSerializer& serializer, PlayerLayer& data);
		void Serialize(PacketSerializer& serializer, PlayerLeaving& data);
		void Serialize(PacketSerializer& serializer, PlayerNameUpdate& data);
		void Serialize(PacketSerializer& serializer, PlayerPingUpdate& data);
		void Serialize(PacketSerializer& serializer, PlayersInput& data);
		void Serialize(PacketSerializer& serializer, PlayerSelectWeapon& data);
		void Serialize(PacketSerializer& serializer, PlayerWeapons& data);
		void Serialize(PacketSerializer& serializer, Ready& data);
		void Serialize(PacketSerializer& serializer, ScriptPacket& data);
		void Serialize(PacketSerializer& serializer, UpdatePlayerName& data);

		// Helpers
		void Serialize(PacketSerializer& serializer, PlayerInputData& data);
		void Serialize(PacketSerializer& serializer, Helper::EntityData& data);
		void Serialize(PacketSerializer& serializer, Helper::EntityId& data);
		void Serialize(PacketSerializer& serializer, Helper::Property& data);
	}
}

#include <CoreLib/Protocol/Packets.inl>

#endif
