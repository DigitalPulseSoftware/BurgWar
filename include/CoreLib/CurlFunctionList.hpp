// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if !defined(BURGWAR_CURL_FUNCTION)
#error You must define BURGWAR_CURL_FUNCTION before including this file
#endif

#ifndef BURGWAR_CURL_FUNCTION_LAST
#define BURGWAR_CURL_FUNCTION_LAST(F) BURGWAR_CURL_FUNCTION(F)
#endif

BURGWAR_CURL_FUNCTION(easy_cleanup)
BURGWAR_CURL_FUNCTION(easy_getinfo)
BURGWAR_CURL_FUNCTION(easy_init)
BURGWAR_CURL_FUNCTION(easy_setopt)
BURGWAR_CURL_FUNCTION(easy_strerror)
BURGWAR_CURL_FUNCTION(global_cleanup)
BURGWAR_CURL_FUNCTION(global_init)
BURGWAR_CURL_FUNCTION(multi_add_handle)
BURGWAR_CURL_FUNCTION(multi_cleanup)
BURGWAR_CURL_FUNCTION(multi_info_read)
BURGWAR_CURL_FUNCTION(multi_init)
BURGWAR_CURL_FUNCTION(multi_perform)
BURGWAR_CURL_FUNCTION(multi_remove_handle)
BURGWAR_CURL_FUNCTION(multi_strerror)
BURGWAR_CURL_FUNCTION(slist_append)
BURGWAR_CURL_FUNCTION(slist_free_all)
BURGWAR_CURL_FUNCTION_LAST(version_info)

#undef BURGWAR_CURL_FUNCTION
#undef BURGWAR_CURL_FUNCTION_LAST
