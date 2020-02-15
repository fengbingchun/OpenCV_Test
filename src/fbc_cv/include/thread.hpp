// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_THREAD_HPP_
#define FBC_CV_THREAD_HPP_

// reference: ffmpeg 4.2
//            libavutil/thread.h

#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <errno.h>

namespace fbc {

typedef struct pthread_t {
	void *handle;
	void *(*func)(void* arg);
	void *arg;
	void *ret;
} pthread_t;

/* use light weight mutex/condition variable API for Windows Vista and later */
typedef SRWLOCK pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;

#define PTHREAD_MUTEX_INITIALIZER SRWLOCK_INIT
#define PTHREAD_COND_INITIALIZER CONDITION_VARIABLE_INIT

#define InitializeCriticalSection(x) InitializeCriticalSectionEx(x, 0, 0)
#define WaitForSingleObject(a, b) WaitForSingleObjectEx(a, b, FALSE)

static unsigned __stdcall win32thread_worker(void *arg)
{
	pthread_t *h = (pthread_t*)arg;
	h->ret = h->func(h->arg);
	return 0;
}

static int pthread_create(pthread_t *thread, const void *unused_attr, void *(*start_routine)(void*), void *arg)
{
	thread->func = start_routine;
	thread->arg = arg;
#if HAVE_WINRT
	thread->handle = (void*)CreateThread(NULL, 0, win32thread_worker, thread,
		0, NULL);
#else
	thread->handle = (void*)_beginthreadex(NULL, 0, win32thread_worker, thread,
		0, NULL);
#endif
	return !thread->handle;
}

static int pthread_join(pthread_t thread, void **value_ptr)
{
	DWORD ret = WaitForSingleObject(thread.handle, INFINITE);
	if (ret != WAIT_OBJECT_0) {
		if (ret == WAIT_ABANDONED)
			return EINVAL;
		else
			return EDEADLK;
	}
	if (value_ptr)
		*value_ptr = thread.ret;
	CloseHandle(thread.handle);
	return 0;
}

static inline int pthread_mutex_init(pthread_mutex_t *m, void* attr)
{
	InitializeSRWLock(m);
	return 0;
}
static inline int pthread_mutex_destroy(pthread_mutex_t *m)
{
	/* Unlocked SWR locks use no resources */
	return 0;
}
static inline int pthread_mutex_lock(pthread_mutex_t *m)
{
	AcquireSRWLockExclusive(m);
	return 0;
}
static inline int pthread_mutex_unlock(pthread_mutex_t *m)
{
	ReleaseSRWLockExclusive(m);
	return 0;
}

typedef INIT_ONCE pthread_once_t;
#define PTHREAD_ONCE_INIT INIT_ONCE_STATIC_INIT

static int pthread_once(pthread_once_t *once_control, void(*init_routine)(void))
{
	BOOL pending = FALSE;
	InitOnceBeginInitialize(once_control, 0, &pending, NULL);
	if (pending)
		init_routine();
	InitOnceComplete(once_control, 0, NULL);
	return 0;
}

static inline int pthread_cond_init(pthread_cond_t *cond, const void *unused_attr)
{
	InitializeConditionVariable(cond);
	return 0;
}

/* native condition variables do not destroy */
static inline int pthread_cond_destroy(pthread_cond_t *cond)
{
	return 0;
}

static inline int pthread_cond_broadcast(pthread_cond_t *cond)
{
	WakeAllConditionVariable(cond);
	return 0;
}

static inline int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	SleepConditionVariableSRW(cond, mutex, INFINITE, 0);
	return 0;
}

static inline int pthread_cond_signal(pthread_cond_t *cond)
{
	WakeConditionVariable(cond);
	return 0;
}

#define AVMutex pthread_mutex_t
#define AV_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

#define ff_mutex_init    pthread_mutex_init
#define ff_mutex_lock    pthread_mutex_lock
#define ff_mutex_unlock  pthread_mutex_unlock
#define ff_mutex_destroy pthread_mutex_destroy

#define AVOnce pthread_once_t
#define AV_ONCE_INIT PTHREAD_ONCE_INIT

#define ff_thread_once(control, routine) pthread_once(control, routine)

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_THREAD_HPP_
