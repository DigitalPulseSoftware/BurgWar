// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if !defined(BURGWAR_CURL_FUNCTION)
#error You must define BURGWAR_CURL_FUNCTION before including this file
#endif

#ifndef BURGWAR_CURL_FUNCTION_LAST
#define BURGWAR_CURL_FUNCTION_LAST(F) BURGWAR_CURL_FUNCTION(F)
#endif

BURGWAR_CURL_FUNCTION(curl_easy_cleanup)
BURGWAR_CURL_FUNCTION(curl_easy_getinfo)
BURGWAR_CURL_FUNCTION(curl_easy_init)
BURGWAR_CURL_FUNCTION(curl_easy_setopt)
BURGWAR_CURL_FUNCTION(curl_easy_strerror)
BURGWAR_CURL_FUNCTION(curl_global_cleanup)
BURGWAR_CURL_FUNCTION(curl_global_init)
BURGWAR_CURL_FUNCTION(curl_multi_add_handle)
BURGWAR_CURL_FUNCTION(curl_multi_cleanup)
BURGWAR_CURL_FUNCTION(curl_multi_info_read)
BURGWAR_CURL_FUNCTION(curl_multi_init)
BURGWAR_CURL_FUNCTION(curl_multi_perform)
BURGWAR_CURL_FUNCTION(curl_multi_remove_handle)
BURGWAR_CURL_FUNCTION(curl_multi_strerror)
BURGWAR_CURL_FUNCTION(curl_slist_append)
BURGWAR_CURL_FUNCTION(curl_slist_free_all)
BURGWAR_CURL_FUNCTION_LAST(curl_version_info)

#undef BURGWAR_CURL_FUNCTION
#undef BURGWAR_CURL_FUNCTION_LAST
