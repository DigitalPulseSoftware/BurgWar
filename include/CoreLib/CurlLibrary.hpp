// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CURLLIBRARY_HPP
#define BURGWAR_CORELIB_CURLLIBRARY_HPP

#include <Nazara/Core/DynLib.hpp>
#include <curl/curl.h>

namespace bw
{	
	struct CurlLibrary
	{
		Nz::DynLib library;

#define BURGWAR_CURL_FUNCTION(F) using F##_ptr = decltype(&F); \
                                 F##_ptr F = nullptr;

#include <CoreLib/CurlFunctionList.hpp>
	};
}

#endif
