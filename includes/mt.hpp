/*
 * Copyright (C) 2013 midnightBITS
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __MT_HPP__
#define __MT_HPP__

#include <stdexcept>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef POSIX
#include <pthread.h>
#endif

namespace mt
{
#ifdef POSIX
	static void* thread_run(void*);
#endif

	class Thread
	{
#ifdef _WIN32
		uintptr_t m_thread;
		unsigned int m_threadId;

		friend unsigned int __stdcall thread_run(void*);
#endif
#ifdef POSIX
		pthread_t m_thread;
		friend void* thread_run(void*);
#endif
		bool stopThread;

		void __run();
	public:
		Thread() : m_thread(0), stopThread(false) {}
		static unsigned long currentId();
		void attach();
		void start();
		bool stop();
		bool shouldStop() const { return stopThread; }
		virtual void run() = 0;
		virtual void shutdown() = 0;
#ifdef _WIN32
		unsigned int threadId() const { return m_threadId; }
#endif
#ifdef POSIX
		pthread_t threadId() const { return m_thread; }
#endif

	};

	class AsyncData
	{
	protected:
		std::mutex m_mtx;
	public:
		AsyncData() {}
		~AsyncData() {}
		void lock() { m_mtx.lock(); }
		void unlock() { m_mtx.unlock(); }
	};

	using Synchronize = std::lock_guard<AsyncData>;
}

using mt::Synchronize;

#endif // __MT_HPP__
