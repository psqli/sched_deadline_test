/*
 * SCHED_DEADLINE test
 * Copyright (C) 2018  Ricardo Biehl Pasquali
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * 01/06/2018
 *
 * SCHED_DEADLINE exists since Linux kernel 3.14
 */

#define _GNU_SOURCE
#include <stdio.h>       /* printf() */
#include <time.h>        /* clock_gettime() */
#include <stdint.h>      /* int*_t */
#include <sys/syscall.h> /* SYS_* */
#include <unistd.h>      /* syscall() */

#define gettid()      syscall(SYS_gettid)
#define sched_yield() syscall(SYS_sched_yield)

#define SCHED_DEADLINE 6

struct sched_attr {
	uint32_t size;

	uint32_t sched_policy;
	uint64_t sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	int32_t sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	uint32_t sched_priority;

	/* SCHED_DEADLINE (nsec) */
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

int
sched_setattr(pid_t pid, const struct sched_attr *attr,
              unsigned int flags)
{
	return syscall(SYS_sched_setattr, pid, attr, flags);
}

int
sched_getattr(pid_t pid, struct sched_attr *attr,
              unsigned int size, unsigned int flags)
{
	return syscall(SYS_sched_getattr, pid, attr, size, flags);
}

static void
time_diff(struct timespec *diff,
          struct timespec *stop,
          struct timespec *start)
{
	if (stop->tv_nsec < start->tv_nsec) {
		/* here we assume (stop->tv_sec - start->tv_sec) is not zero */
		diff->tv_sec = stop->tv_sec - start->tv_sec - 1;
		diff->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		diff->tv_sec = stop->tv_sec - start->tv_sec;
		diff->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
}

int
main(void)
{
	/* scheduler attributes */
	struct sched_attr attr;
	int runtime_ms, deadline_ms, period_ms;
	unsigned int flags = 0;
	/* time measurements */
	struct timespec last, now, diff;
	int ret;

	printf("Ctrl C to stop\n");
	printf("thread id: %ld\n", gettid());

	runtime_ms = 4;
	deadline_ms = 10;
	period_ms = 1000;

	/* set up sched_attr structure */
	attr.size = sizeof(attr);
	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_flags = 0;
	attr.sched_nice = 0;
	attr.sched_priority = 0;
	attr.sched_runtime = runtime_ms * 1000000;
	attr.sched_deadline = deadline_ms * 1000000;
	attr.sched_period = attr.sched_deadline = period_ms * 1000000;

	/* set sched deadline */
	ret = sched_setattr(0, &attr, flags);
	if (ret < 0) {
		perror("sched_setattr");
		return 1;
	}

	clock_gettime(CLOCK_MONOTONIC, &last);

	while (1) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		time_diff(&diff, &now, &last);
		printf("%ld.%06ld ms since last wake up\n",
		       diff.tv_sec * 1000 + diff.tv_nsec / 1000000,
		       diff.tv_nsec % 1000000);
		last = now;
		sched_yield();
	}

	return 0;
}
