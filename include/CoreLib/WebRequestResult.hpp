// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_WEBREQUESTRESULT_HPP
#define BURGWAR_CORELIB_WEBREQUESTRESULT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Utils/MovablePtr.hpp>
#include <tsl/hopscotch_map.h>
#include <functional>
#include <string>

using CURL = void;

namespace bw
{
	class BURGWAR_CORELIB_API WebRequestResult
	{
		friend class WebRequest;

		public:
			WebRequestResult(const WebRequestResult&) = delete;
			WebRequestResult(WebRequestResult&&) = delete;
			~WebRequestResult() = default;

			inline std::string& GetBody();
			inline const std::string& GetBody() const;
			Nz::UInt64 GetDownloadedSize() const;
			Nz::UInt64 GetDownloadSpeed() const;
			inline const std::string& GetErrorMessage() const;
			long GetReponseCode() const;

			inline bool HasSucceeded() const;

			inline explicit operator bool() const;

			WebRequestResult& operator=(const WebRequestResult&) = delete;
			WebRequestResult& operator=(WebRequestResult&&) = delete;

		private:
			inline WebRequestResult(CURL* curl, std::string body);
			inline WebRequestResult(std::string errMessage);

			CURL* m_curlHandle;
			std::string m_bodyOrErr;
	};
}

#include <CoreLib/WebRequestResult.inl>

#endif
