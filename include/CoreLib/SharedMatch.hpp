// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDMATCH_HPP
#define BURGWAR_CORELIB_SHAREDMATCH_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/TimerManager.hpp>
#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Scripting/ScriptHandlerRegistry.hpp>
#include <entt/entt.hpp>
#include <tl/function_ref.hpp>

namespace bw
{
	class BurgAppComponent;
	class ScriptingContext;
	class SharedEntityStore;
	class SharedLayer;
	class SharedGamemode;
	class SharedWeaponStore;

	class BURGWAR_CORELIB_API SharedMatch
	{
		public:
			SharedMatch(BurgAppComponent& app, LogSide side, std::string matchName, Nz::Time tickDuration);
			SharedMatch(const SharedMatch&) = delete;
			SharedMatch(SharedMatch&&) = delete;
			virtual ~SharedMatch();

			virtual void ForEachEntity(tl::function_ref<void(entt::handle entity)> func) = 0;

			inline Nz::UInt64 GetCurrentTick() const;
			inline Nz::Time GetCurrentTime() const;
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
			inline Nz::Time GetTickDuration() const;
			inline TimerManager& GetTimerManager();
			virtual SharedWeaponStore& GetWeaponStore() = 0;
			virtual const SharedWeaponStore& GetWeaponStore() const = 0;

			virtual entt::handle RetrieveEntityByUniqueId(EntityId uniqueId) const = 0;
			virtual EntityId RetrieveUniqueIdByEntity(entt::handle entity) const = 0;

			void Update(Nz::Time elapsedTime);

			SharedMatch& operator=(const SharedMatch&) = delete;
			SharedMatch& operator=(SharedMatch&&) = delete;

		protected:
			virtual void OnTick(bool lastTick) = 0;

		private:
			std::string m_name;
			MatchLogger m_logger;
			ScriptHandlerRegistry m_scriptPacketHandler;
			TimerManager m_timerManager;
			Nz::Time m_currentTime;
			Nz::Time m_tickTimer;
			Nz::Time m_tickDuration;
			Nz::UInt64 m_currentTick;
	};
}

#include <CoreLib/SharedMatch.inl>

#endif
