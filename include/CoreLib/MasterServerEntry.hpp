// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MASTERSERVERENTRY_HPP
#define BURGWAR_CORELIB_MASTERSERVERENTRY_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Network/WebService.hpp>
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
			MasterServerEntry(Nz::WebService& webService, Match& match, std::string masterServerURL);
			MasterServerEntry(const MasterServerEntry&) = delete;
			MasterServerEntry(MasterServerEntry&&) = delete;
			~MasterServerEntry() = default;

			void Update(Nz::Time elapsedTime);

			MasterServerEntry& operator=(const MasterServerEntry&) = delete;
			MasterServerEntry& operator=(MasterServerEntry&&) = delete;

		private:
			nlohmann::json BuildServerInfo() const;
			void HandleResponse(Nz::WebRequestResult&& result, bool refresh);

			void Refresh();
			void Register();

			std::string m_masterServerURL;
			std::string m_requestBody;
			std::string m_updateToken;
			Nz::WebService& m_webService;
			Match& m_match;
			Nz::Time m_timeBeforeRefresh;
	};
}

#include <CoreLib/MasterServerEntry.inl>

#endif
