// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDMATCH_HPP
#define BURGWAR_CORELIB_SHAREDMATCH_HPP

#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/TimerManager.hpp>
#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Scripting/ScriptHandlerRegistry.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class BurgApp;
	class ScriptingContext;
	class SharedEntityStore;
	class SharedLayer;
	class SharedGamemode;
	class SharedWeaponStore;

	class SharedMatch
	{
		public:
			SharedMatch(BurgApp& app, LogSide side, std::string matchName, float tickDuration);
			SharedMatch(const SharedMatch&) = delete;
			SharedMatch(SharedMatch&&) = delete;
			virtual ~SharedMatch();

			virtual void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) = 0;

			inline Nz::UInt64 GetCurrentTick() const;
			inline Nz::UInt64 GetCurrentTime() const;
			virtual SharedEntityStore& GetEntityStore() = 0;
			virtual const SharedEntityStore& GetEntityStore() const = 0;
			inline MatchLogger& GetLogger();
			virtual SharedLayer& GetLayer(LayerIndex layerIndex) = 0;
			virtual const SharedLayer& GetLayer(LayerIndex layerIndex) const = 0;
			virtual LayerIndex GetLayerCount() const = 0;
			inline const std::string& GetName() const;
			virtual const NetworkStringStore& GetNetworkStringStore() const = 0;
			inline Nz::UInt16 GetNetworkTick() const;
			inline Nz::UInt16 GetNetworkTick(Nz::UInt64 tick) const;
			inline ScriptHandlerRegistry& GetScriptPacketHandlerRegistry();
			inline const ScriptHandlerRegistry& GetScriptPacketHandlerRegistry() const;
			virtual std::shared_ptr<const SharedGamemode> GetSharedGamemode() const = 0;
			inline float GetTickDuration() const;
			inline TimerManager& GetTimerManager();
			virtual SharedWeaponStore& GetWeaponStore() = 0;
			virtual const SharedWeaponStore& GetWeaponStore() const = 0;

			virtual const Ndk::EntityHandle& RetrieveEntityByUniqueId(EntityId uniqueId) const = 0;
			virtual EntityId RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const = 0;

			void Update(float elapsedTime);

			SharedMatch& operator=(const SharedMatch&) = delete;
			SharedMatch& operator=(SharedMatch&&) = delete;

		protected:
			virtual void OnTick(bool lastTick) = 0;

		private:
			std::string m_name;
			MatchLogger m_logger;
			ScriptHandlerRegistry m_scriptPacketHandler;
			TimerManager m_timerManager;
			Nz::UInt64 m_currentTick;
			Nz::UInt64 m_currentTime;
			float m_floatingTime;
			float m_maxTickTimer;
			float m_tickDuration;
			float m_tickTimer;
	};
}

#include <CoreLib/SharedMatch.inl>

#endif
