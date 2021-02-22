// Copyright (C) 2021 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_EXPORT_HPP
#define BURGWAR_CLIENTLIB_EXPORT_HPP

#include <Nazara/Prerequisites.hpp>

#ifdef BURGWAR_CLIENTLIB_STATIC
    #define BURGWAR_CLIENTLIB_API
#else
    #ifdef BURGWAR_CLIENTLIB_BUILD
        #define BURGWAR_CLIENTLIB_API NAZARA_EXPORT
    #else
        #define BURGWAR_CLIENTLIB_API NAZARA_IMPORT
    #endif
#endif

#endif
