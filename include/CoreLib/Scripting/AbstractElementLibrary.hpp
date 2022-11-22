// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ABSTRACTELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_ABSTRACTELEMENTLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <sol/forward.hpp>
#include <memory>

namespace bw
{
	class Logger;

	class BURGWAR_CORELIB_API AbstractElementLibrary
	{
		public:
			inline AbstractElementLibrary(const Logger& logger);
			virtual ~AbstractElementLibrary();

			inline const Logger& GetLogger() const;

			virtual void RegisterLibrary(sol::table& elementMetatable) = 0;

		private:
			const Logger& m_logger;
	};
}

#include <CoreLib/Scripting/AbstractElementLibrary.inl>

#endif
