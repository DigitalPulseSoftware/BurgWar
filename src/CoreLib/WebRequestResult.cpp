// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebRequestResult.hpp>
#include <curl/curl.h>

namespace bw
{
	Nz::UInt64 WebRequestResult::GetDownloadedSize() const
	{
		assert(HasSucceeded());

		curl_off_t downloadedSize = 0;
		curl_easy_getinfo(m_curlHandle, CURLINFO_SIZE_DOWNLOAD_T, &downloadedSize);

		return downloadedSize;
	}

	std::size_t WebRequestResult::GetDownloadSpeed() const
	{
		assert(HasSucceeded());

		curl_off_t downloadSpeed = 0;
		curl_easy_getinfo(m_curlHandle, CURLINFO_SPEED_DOWNLOAD_T, &downloadSpeed);

		return downloadSpeed;
	}

	long WebRequestResult::GetReponseCode() const
	{
		assert(HasSucceeded());

		long responseCode;
		curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &responseCode);

		return responseCode;
	}
}
