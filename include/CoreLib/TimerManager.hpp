// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TIMERMANAGER_HPP
#define BURGWAR_CORELIB_TIMERMANAGER_HPP

#include <Nazara/Prerequisites.hpp>
#include <functional>
#include <vector>

namespace bw
{
	class SharedMatch;

	class TimerManager
	{
		public:
			using Callback = std::function<void()>;

			TimerManager() = default;
			~TimerManager() = default;

			inline void Clear();

			inline void PushCallback(Nz::UInt64 expirationTime, Callback finish);

			inline void Update(Nz::UInt64 now);

		private:
			struct Timer
			{
				Callback callback;
				Nz::UInt64 expirationTime;
			};

			std::vector<Timer> m_pendingTimers;
	};
}

#include <CoreLib/TimerManager.inl>

#endif
