// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MASTERSERVERENTRY_HPP
#define BURGWAR_CORELIB_MASTERSERVERENTRY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/WebService.hpp>
#include <Nazara/Core/Time.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

namespace bw
{
	class Logger;
	class Match;
	class WebRequestResult;

	class BURGWAR_CORELIB_API MasterServerEntry
	{
		public:
			MasterServerEntry(Match& match, std::string masterServerURL);
			MasterServerEntry(const MasterServerEntry&) = delete;
			MasterServerEntry(MasterServerEntry&&) = delete;
			~MasterServerEntry() = default;

			void Update(Nz::Time elapsedTime);

			MasterServerEntry& operator=(const MasterServerEntry&) = delete;
			MasterServerEntry& operator=(MasterServerEntry&&) = delete;

		private:
			nlohmann::json BuildServerInfo() const;
			void HandleResponse(WebRequestResult&& result, bool refresh);

			void Refresh();
			void Register();

			std::string m_masterServerURL;
			std::string m_requestBody;
			std::string m_updateToken;
			Match& m_match;
			WebService m_webService;
			Nz::Time m_timeBeforeRefresh;
	};
}

#include <CoreLib/MasterServerEntry.inl>

#endif
