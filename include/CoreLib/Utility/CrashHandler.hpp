// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CRASHHANDLER_HPP
#define BURGWAR_CORELIB_CRASHHANDLER_HPP

#include <memory>

namespace bw
{
	class CrashHandler
	{
		public:
			CrashHandler();
			~CrashHandler();

			bool Install();
			void Uninstall();

		private:
			struct InternalData;

			std::unique_ptr<InternalData> m_internalData;
	};
}

#include <CoreLib/Utility/CrashHandler.inl>

#endif
