#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <thread>
#include <vector>

/**
 * Get the number of cores, or if set the integer value of the environment
 * variable 'NSLOTS' (set by default by OGE/SGE qsub)
 */
inline unsigned get_thread_count() {
  unsigned thread_count = std::thread::hardware_concurrency();

  char const *nslots = std::getenv("NSLOTS");
  if (nslots != nullptr) {
    std::stringstream(nslots) >> thread_count;
  }

  // sanity check
  if (thread_count <= 0 || thread_count > 64) {
    std::cerr << "Thread count " << thread_count << " is not valid!" << std::endl;
    std::abort();
  }

  std::cout << "Using " << thread_count << " threads" << std::endl;
  return thread_count;
}

template <typename T>
using Iterator = typename T::iterator;
template <typename T>
using Chunk = typename std::pair<Iterator<std::vector<T>>, Iterator<std::vector<T>>>;
template <typename T>
using ChunkVec = typename std::vector<Chunk<T>>;

/*
 * Chunk a container into equal sized pieces (except for the last one),
 * returns a vector of begin and end iterators
 */
template <typename T>
ChunkVec<T> make_chunks(std::vector<T> &container, const std::ptrdiff_t n) {
  assert(n > 0);

  using diff_t = std::ptrdiff_t;
  using It     = Iterator<std::vector<T>>;

  const It range_from = std::begin(container);
  const It range_to   = std::end(container);

  const diff_t total   = std::distance(range_from, range_to);
  const diff_t portion = total / n;

  ChunkVec<T> chunks(n);

  It portion_end = range_from;

  std::generate(begin(chunks), end(chunks), [&portion_end, portion]() {
    It portion_start = portion_end;

    portion_end += portion;
    return std::make_pair(portion_start, portion_end);
  });

  chunks.back().second = range_to;

  return chunks;
}

/*
 * Spinning thread barrier to synchronize threads
 */
class ThreadBarrier {
  const unsigned count;
  std::atomic<unsigned> spaces;
  std::atomic<std::size_t> generation;

 public:
  explicit ThreadBarrier(unsigned count_) : count(count_), spaces(count), generation(0) {}

  void arrive_and_wait() {
    const std::size_t my_generation = generation;
    if (!--spaces) {
      spaces = count;
      ++generation;
    } else {
      while (generation == my_generation);
    }
  }
};


// Code in below is an adaptation of Jeff Preshing's
// portable + lightweight semaphore implementations, originally from
// https://github.com/preshing/cpp11-on-multicore/blob/master/common/sema.h
// LICENSE:
// Copyright (c) 2015 Jeff Preshing
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//	claim that you wrote the original software. If you use this software
//	in a product, an acknowledgement in the product documentation would be
//	appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//	misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#if defined(_WIN32)
//---------------------------------------------------------
// Semaphore (Windows)
//---------------------------------------------------------

#include <windows.h>
#undef min
#undef max

class Semaphore
{
private:
    HANDLE m_hSema;

    Semaphore(const Semaphore& other) = delete;
    Semaphore& operator=(const Semaphore& other) = delete;

public:
    Semaphore(int initialCount = 0)
    {
        assert(initialCount >= 0);
        m_hSema = CreateSemaphore(NULL, initialCount, MAXLONG, NULL);
    }

    ~Semaphore()
    {
        CloseHandle(m_hSema);
    }

    void wait()
    {
        WaitForSingleObject(m_hSema, INFINITE);
    }

    void signal(int count = 1)
    {
        ReleaseSemaphore(m_hSema, count, NULL);
    }
};


#elif defined(__MACH__)
//---------------------------------------------------------
// Semaphore (Apple iOS and OSX)
// Can't use POSIX semaphores due to http://lists.apple.com/archives/darwin-kernel/2009/Apr/msg00010.html
//---------------------------------------------------------

#include <mach/mach.h>

class Semaphore
{
private:
    semaphore_t m_sema;

    Semaphore(const Semaphore& other) = delete;
    Semaphore& operator=(const Semaphore& other) = delete;

public:
    Semaphore(int initialCount = 0)
    {
        assert(initialCount >= 0);
        semaphore_create(mach_task_self(), &m_sema, SYNC_POLICY_FIFO, initialCount);
    }

    ~Semaphore()
    {
        semaphore_destroy(mach_task_self(), m_sema);
    }

    void wait()
    {
        semaphore_wait(m_sema);
    }

    void signal()
    {
        semaphore_signal(m_sema);
    }

    void signal(int count)
    {
        while (count-- > 0)
        {
            semaphore_signal(m_sema);
        }
    }
};


#elif defined(__unix__)
//---------------------------------------------------------
// Semaphore (POSIX, Linux)
//---------------------------------------------------------

#include <semaphore.h>

class Semaphore
{
 private:
  sem_t m_sema;

  Semaphore(const Semaphore& other) = delete;
  Semaphore& operator=(const Semaphore& other) = delete;

 public:
  Semaphore(int initialCount = 0)
  {
    assert(initialCount >= 0);
    sem_init(&m_sema, 0, initialCount);
  }

  ~Semaphore()
  {
    sem_destroy(&m_sema);
  }

  void wait()
  {
    // http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
    int rc;
    do
    {
      rc = sem_wait(&m_sema);
    }
    while (rc == -1 && errno == EINTR);
  }

  void signal()
  {
    sem_post(&m_sema);
  }

  void signal(int count)
  {
    while (count-- > 0)
    {
      sem_post(&m_sema);
    }
  }
};


#else

#error Unsupported platform!

#endif


//---------------------------------------------------------
// LightweightSemaphore
//---------------------------------------------------------
class LightweightSemaphore
{
 private:
  std::atomic<int> m_count;
  Semaphore m_sema;

  void waitWithPartialSpinning()
  {
    int oldCount;
    // Is there a better way to set the initial spin count?
    // If we lower it to 1000, testBenaphore becomes 15x slower on my Core i7-5930K Windows PC,
    // as threads start hitting the kernel semaphore.
    int spin = 10000;
    while (spin--)
    {
      oldCount = m_count.load(std::memory_order_relaxed);
      if ((oldCount > 0) && m_count.compare_exchange_strong(oldCount, oldCount - 1, std::memory_order_acquire))
        return;
      std::atomic_signal_fence(std::memory_order_acquire);     // Prevent the compiler from collapsing the loop.
    }
    oldCount = m_count.fetch_sub(1, std::memory_order_acquire);
    if (oldCount <= 0)
    {
      m_sema.wait();
    }
  }

 public:
  LightweightSemaphore(int initialCount = 0) : m_count(initialCount)
  {
    assert(initialCount >= 0);
  }

  bool tryWait()
  {
    int oldCount = m_count.load(std::memory_order_relaxed);
    return (oldCount > 0 && m_count.compare_exchange_strong(oldCount, oldCount - 1, std::memory_order_acquire));
  }

  void wait()
  {
    if (!tryWait())
      waitWithPartialSpinning();
  }

  void signal(int count = 1)
  {
    int oldCount = m_count.fetch_add(count, std::memory_order_release);
    int toRelease = -oldCount < count ? -oldCount : count;
    if (toRelease > 0)
    {
      m_sema.signal(toRelease);
    }
  }
};


typedef LightweightSemaphore DefaultSemaphoreType;

//---------------------------------------------------------
// AutoResetEvent
//---------------------------------------------------------
class AutoResetEvent
{
 private:
  // m_status == 1: Event object is signaled.
  // m_status == 0: Event object is reset and no threads are waiting.
  // m_status == -N: Event object is reset and N threads are waiting.
  std::atomic<int> m_status;
  DefaultSemaphoreType m_sema;

 public:
  AutoResetEvent(int initialStatus = 0) : m_status(initialStatus)
  {
    assert(initialStatus >= 0 && initialStatus <= 1);
  }

  void signal()
  {
    int oldStatus = m_status.load(std::memory_order_relaxed);
    for (;;)    // Increment m_status atomically via CAS loop.
    {
      assert(oldStatus <= 1);
      int newStatus = oldStatus < 1 ? oldStatus + 1 : 1;
      if (m_status.compare_exchange_weak(oldStatus, newStatus, std::memory_order_release, std::memory_order_relaxed))
        break;
      // The compare-exchange failed, likely because another thread changed m_status.
      // oldStatus has been updated. Retry the CAS loop.
    }
    if (oldStatus < 0)
      m_sema.signal();    // Release one waiting thread.
  }

  void wait()
  {
    int oldStatus = m_status.fetch_sub(1, std::memory_order_acquire);
    assert(oldStatus <= 1);
    if (oldStatus < 1)
    {
      m_sema.wait();
    }
  }
};
