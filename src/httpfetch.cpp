/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "httpfetch.h"
#include "porting.h" // for sleep_ms(), get_sysinfo()
#include <iostream>
#include <sstream>
#include <list>
#include <unordered_map>
#include <cerrno>
#include <mutex>
#include "network/socket.h" // for select()
#include "threading/event.h"
#include "config.h"
#include "exceptions.h"
#include "debug.h"
#include "log.h"
#include "util/container.h"
#include "util/thread.h"
#include "version.h"
#include "settings.h"
#include "noise.h"

static std::mutex g_httpfetch_mutex;
static std::unordered_map<u64, std::queue<HTTPFetchResult>>
	g_httpfetch_results;

HTTPFetchRequest::HTTPFetchRequest() :
	timeout(20),
	connect_timeout(10 * 1000),
	useragent(std::string(PROJECT_NAME_C "/") + g_version_hash + " (" + porting::get_sysinfo() + ")")
{
}


static void httpfetch_deliver_result(const HTTPFetchResult &fetch_result)
{
	u64 caller = fetch_result.caller;
	if (caller != HTTPFETCH_DISCARD) {
		MutexAutoLock lock(g_httpfetch_mutex);
		g_httpfetch_results[caller].emplace(fetch_result);
	}
}

static void httpfetch_request_clear(u64 caller);

u64 httpfetch_caller_alloc()
{
	MutexAutoLock lock(g_httpfetch_mutex);

	// Check each caller ID except reserved ones
	for (u64 caller = HTTPFETCH_CID_START; caller != 0; ++caller) {
		auto it = g_httpfetch_results.find(caller);
		if (it == g_httpfetch_results.end()) {
			verbosestream << "httpfetch_caller_alloc: allocating "
					<< caller << std::endl;
			// Access element to create it
			g_httpfetch_results[caller];
			return caller;
		}
	}

	FATAL_ERROR("httpfetch_caller_alloc: ran out of caller IDs");
}

void httpfetch_caller_free(u64 caller)
{
	verbosestream<<"httpfetch_caller_free: freeing "
			<<caller<<std::endl;

	httpfetch_request_clear(caller);
	if (caller != HTTPFETCH_DISCARD) {
		MutexAutoLock lock(g_httpfetch_mutex);
		g_httpfetch_results.erase(caller);
	}
}

bool httpfetch_async_get(u64 caller, HTTPFetchResult &fetch_result)
{
	MutexAutoLock lock(g_httpfetch_mutex);

	// Check that caller exists
	auto it = g_httpfetch_results.find(caller);
	if (it == g_httpfetch_results.end())
		return false;

	// Check that result queue is nonempty
	std::queue<HTTPFetchResult> &caller_results = it->second;
	if (caller_results.empty())
		return false;

	// Pop first result
	fetch_result = std::move(caller_results.front());
	caller_results.pop();
	return true;
}

/*
	USE_CURL is off:

	Dummy httpfetch implementation that always returns an error.
*/

void httpfetch_init(int parallel_limit)
{
}

void httpfetch_cleanup()
{
}

void httpfetch_async(const HTTPFetchRequest &fetch_request)
{
	errorstream << "httpfetch_async: unable to fetch " << fetch_request.url
			<< " because USE_CURL=0" << std::endl;

	HTTPFetchResult fetch_result(fetch_request); // sets succeeded = false etc.
	httpfetch_deliver_result(fetch_result);
}

static void httpfetch_request_clear(u64 caller)
{
}

void httpfetch_sync(const HTTPFetchRequest &fetch_request,
		HTTPFetchResult &fetch_result)
{
	errorstream << "httpfetch_sync: unable to fetch " << fetch_request.url
			<< " because USE_CURL=0" << std::endl;

	fetch_result = HTTPFetchResult(fetch_request); // sets succeeded = false etc.
}
