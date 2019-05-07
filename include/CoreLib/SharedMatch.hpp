// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDMATCH_HPP
#define BURGWAR_CORELIB_SHAREDMATCH_HPP

#include <CoreLib/TimerManager.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class SharedMatch
	{
		public:
			inline SharedMatch(BurgApp& app, float tickDuration);
			SharedMatch(const SharedMatch&) = delete;
			SharedMatch(SharedMatch&&) = delete;
			virtual ~SharedMatch();

			virtual void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) = 0;

			inline Nz::UInt16 GetCurrentTick() const;
			inline Nz::UInt64 GetCurrentTime() const;
			inline float GetTickDuration() const;
			inline TimerManager& GetTimerManager();

			void Update(float elapsedTime);

			SharedMatch& operator=(const SharedMatch&) = delete;
			SharedMatch& operator=(SharedMatch&&) = delete;

		protected:
			virtual void OnTick(bool lastTick) = 0;

		private:
			TimerManager m_timerManager;
			Nz::UInt16 m_currentTick;
			Nz::UInt64 m_currentTime;
			float m_floatingTime;
			float m_tickDuration;
			float m_tickTimer;
	};
}

#include <CoreLib/SharedMatch.inl>

#endif
