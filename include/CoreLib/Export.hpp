// Copyright (C) 2021 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_EXPORT_HPP
#define BURGWAR_CORELIB_EXPORT_HPP

#include <NazaraUtils/Prerequisites.hpp>

#ifdef BURGWAR_CORELIB_STATIC
    #define BURGWAR_CORELIB_API
#else
    #ifdef BURGWAR_CORELIB_BUILD
        #define BURGWAR_CORELIB_API NAZARA_EXPORT
    #else
        #define BURGWAR_CORELIB_API NAZARA_IMPORT
    #endif
#endif

#endif
