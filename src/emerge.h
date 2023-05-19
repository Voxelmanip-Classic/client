/*
Minetest
Copyright (C) 2010-2013 kwolekr, Ryan Kwolek <kwolekr@minetest.net>

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

#pragma once

#include <map>
#include <mutex>
#include "network/networkprotocol.h"
#include "irr_v3d.h"
#include "util/container.h"
#include "util/metricsbackend.h"
#include "map.h"

#define BLOCK_EMERGE_ALLOW_GEN   (1 << 0)
#define BLOCK_EMERGE_FORCE_QUEUE (1 << 1)

class NodeDefManager;
class Settings;

class BiomeManager;
class SchematicManager;
class Server;

// Structure containing inputs/outputs for chunk generation
struct BlockMakeData {
	MMVManip *vmanip = nullptr;
	u64 seed = 0;
	v3s16 blockpos_min;
	v3s16 blockpos_max;
	UniqueQueue<v3s16> transforming_liquid;
	const NodeDefManager *nodedef = nullptr;

	BlockMakeData() = default;

	~BlockMakeData() { delete vmanip; }
};

// Result from processing an item on the emerge queue
enum EmergeAction {
	EMERGE_CANCELLED,
	EMERGE_ERRORED,
	EMERGE_FROM_MEMORY,
	EMERGE_FROM_DISK,
	EMERGE_GENERATED,
};

const static std::string emergeActionStrs[] = {
	"cancelled",
	"errored",
	"from_memory",
	"from_disk",
	"generated",
};

// Callback
typedef void (*EmergeCompletionCallback)(
	v3s16 blockpos, EmergeAction action, void *param);

typedef std::vector<
	std::pair<
		EmergeCompletionCallback,
		void *
	>
> EmergeCallbackList;

struct BlockEmergeData {
	u16 peer_requested;
	u16 flags;
	EmergeCallbackList callbacks;
};

class EmergeManager {

public:
	const NodeDefManager *ndef;

	// Generation Notify
	u32 gen_notify_on = 0;
	std::set<u32> gen_notify_on_deco_ids;

	// Methods
	EmergeManager(Server *server, MetricsBackend *mb);
	~EmergeManager();
	DISABLE_CLASS_COPY(EmergeManager);

	bool isRunning();

};
