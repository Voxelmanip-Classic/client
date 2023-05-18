/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
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


#include "emerge.h"

#include <iostream>
#include <queue>

#include "util/container.h"
#include "util/thread.h"
#include "threading/event.h"

#include "config.h"
#include "constants.h"
#include "environment.h"
#include "log.h"
#include "map.h"
#include "mapblock.h"
#include "nodedef.h"
#include "profiler.h"
#include "scripting_server.h"
#include "server.h"
#include "settings.h"
#include "voxel.h"

class EmergeThread : public Thread {
public:
	int id;

	EmergeThread(Server *server, int ethreadid);
	~EmergeThread() = default;

	void *run();
	void signal();

	// Requires queue mutex held
	bool pushBlock(const v3s16 &pos);

private:
	Server *m_server;
	ServerMap *m_map;
	EmergeManager *m_emerge;

	Event m_queue_event;
	std::queue<v3s16> m_block_queue;

	friend class EmergeManager;
};

class MapEditEventAreaIgnorer
{
public:
	MapEditEventAreaIgnorer(VoxelArea *ignorevariable, const VoxelArea &a):
		m_ignorevariable(ignorevariable)
	{
		if(m_ignorevariable->getVolume() == 0)
			*m_ignorevariable = a;
		else
			m_ignorevariable = NULL;
	}

	~MapEditEventAreaIgnorer()
	{
		if(m_ignorevariable)
		{
			assert(m_ignorevariable->getVolume() != 0);
			*m_ignorevariable = VoxelArea();
		}
	}

private:
	VoxelArea *m_ignorevariable;
};

EmergeParams::~EmergeParams()
{
	infostream << "EmergeParams: destroying " << this << std::endl;
	// Delete everything that was cloned on creation of EmergeParams
}

EmergeParams::EmergeParams(EmergeManager *parent) :
	ndef(parent->ndef),
	gen_notify_on(parent->gen_notify_on),
	gen_notify_on_deco_ids(&parent->gen_notify_on_deco_ids)
{

}

////
//// EmergeManager
////

EmergeManager::EmergeManager(Server *server, MetricsBackend *mb)
{

}


EmergeManager::~EmergeManager()
{
	for (u32 i = 0; i != m_threads.size(); i++) {
		EmergeThread *thread = m_threads[i];

		if (m_threads_active) {
			thread->stop();
			thread->signal();
			thread->wait();
		}

		delete thread;
	}
}




void EmergeManager::startThreads()
{
	if (m_threads_active)
		return;

	for (u32 i = 0; i != m_threads.size(); i++)
		m_threads[i]->start();

	m_threads_active = true;
}


void EmergeManager::stopThreads()
{
	if (!m_threads_active)
		return;

	// Request thread stop in parallel
	for (u32 i = 0; i != m_threads.size(); i++) {
		m_threads[i]->stop();
		m_threads[i]->signal();
	}

	// Then do the waiting for each
	for (u32 i = 0; i != m_threads.size(); i++)
		m_threads[i]->wait();

	m_threads_active = false;
}


bool EmergeManager::isRunning()
{
	return m_threads_active;
}


bool EmergeManager::enqueueBlockEmerge(
	session_t peer_id,
	v3s16 blockpos,
	bool allow_generate,
	bool ignore_queue_limits)
{
	u16 flags = 0;
	if (allow_generate)
		flags |= BLOCK_EMERGE_ALLOW_GEN;
	if (ignore_queue_limits)
		flags |= BLOCK_EMERGE_FORCE_QUEUE;

	return enqueueBlockEmergeEx(blockpos, peer_id, flags, NULL, NULL);
}


bool EmergeManager::enqueueBlockEmergeEx(
	v3s16 blockpos,
	session_t peer_id,
	u16 flags,
	EmergeCompletionCallback callback,
	void *callback_param)
{

	return true;
}


bool EmergeManager::isBlockInQueue(v3s16 pos)
{
	return true;
}



////
//// EmergeThread
////

EmergeThread::EmergeThread(Server *server, int ethreadid) :
	id(ethreadid),
	m_server(server),
	m_map(NULL),
	m_emerge(NULL)
{
	m_name = "Emerge-" + itos(ethreadid);
}


void EmergeThread::signal()
{
	m_queue_event.signal();
}


bool EmergeThread::pushBlock(const v3s16 &pos)
{
	m_block_queue.push(pos);
	return true;
}

void *EmergeThread::run()
{

}
