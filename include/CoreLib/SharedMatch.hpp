// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDMATCH_HPP
#define BURGWAR_CORELIB_SHAREDMATCH_HPP

#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/TimerManager.hpp>
#include <NDK/Entity.hpp>

//#define DEBUG_PREDICTION

namespace bw
{
	class BurgApp;
	class ScriptingContext;
	class SharedEntityStore;
	class SharedLayer;
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
			inline Nz::UInt16 GetNetworkTick() const;
			inline Nz::UInt16 GetNetworkTick(Nz::UInt64 tick) const;
			inline float GetTickDuration() const;
			inline TimerManager& GetTimerManager();
			virtual SharedWeaponStore& GetWeaponStore() = 0;
			virtual const SharedWeaponStore& GetWeaponStore() const = 0;

			virtual const Ndk::EntityHandle& RetrieveEntityByUniqueId(Nz::Int64 uniqueId) const = 0;
			virtual Nz::Int64 RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const = 0;

			void Update(float elapsedTime);

			SharedMatch& operator=(const SharedMatch&) = delete;
			SharedMatch& operator=(SharedMatch&&) = delete;

		protected:
			virtual void OnTick(bool lastTick) = 0;

		private:
			std::string m_name;
			MatchLogger m_logger;
			TimerManager m_timerManager;
			Nz::UInt64 m_currentTick;
			Nz::UInt64 m_currentTime;
			float m_floatingTime;
			float m_tickDuration;
			float m_tickTimer;
	};
}

#include <CoreLib/SharedMatch.inl>

#endif
