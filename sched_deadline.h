/*
 * 16/07/2018
 *
 * sched deadline support
 *
 * Deadline scheduling for Linux
 *   https://lwn.net/Articles/356576/
 */

#ifndef SCHED_DEADLINE_H
#define SCHED_DEADLINE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>      /* int*_t */
#include <sys/syscall.h> /* SYS_* */
#include <unistd.h>      /* syscall() */

struct sched_attr {
	uint32_t size;

	uint32_t sched_policy;
	uint64_t sched_flags;

	/* used by: SCHED_NORMAL, SCHED_BATCH */
	int32_t sched_nice;

	/* used by: SCHED_FIFO, SCHED_RR */
	uint32_t sched_priority;

	/* used by: SCHED_DEADLINE (nsec) */
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

/* sched_policy */
#define SCHED_DEADLINE 6

/* helpers */
#define gettid()      syscall(SYS_gettid)
#define sched_yield() syscall(SYS_sched_yield)

static inline int
sched_setattr(pid_t pid, const struct sched_attr *attr,
              unsigned int flags)
{
	return syscall(SYS_sched_setattr, pid, attr, flags);
}

static inline int
sched_getattr(pid_t pid, struct sched_attr *attr,
              unsigned int size, unsigned int flags)
{
	return syscall(SYS_sched_getattr, pid, attr, size, flags);
}

#endif /* SCHED_DEADLINE_H */
