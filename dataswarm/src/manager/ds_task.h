/*
Copyright (C) 2022- The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file COPYING for details.
*/

#ifndef DS_TASK_H
#define DS_TASK_H

/*
This module defines the internal structure and details of a single task.
Note that these details are internal to the manager library,
and are not for public consumption.
End user may only use the API described in dataswarm.h
*/

#include "dataswarm.h"

#include "list.h"
#include "category.h"

#include <stdint.h>

struct ds_task {

        /***** Fixed properties of task at submit time. ******/

        int taskid;                  /**< A unique task id number. */
	char *command_line;          /**< The program(s) to execute, as a shell command line. */
	char *coprocess;             /**< The name of the coprocess name in the worker that executes this task. For regular tasks it is NULL. */

	struct list *input_files;    /**< The files to transfer to the worker and place in the executing directory. */
	struct list *output_files;   /**< The output files (other than the standard output stream) created by the program to be retrieved from the task. */
	struct list *env_list;       /**< Environment variables applied to the task. */
	struct list *features;       /**< User-defined features this task requires. (See ds_worker's --feature option.) */

	char *tag;                   /**< An optional user-defined logical name for the task. */
	char *category;              /**< User-provided label for the task. It is expected that all task with the same category will have similar resource usage. See @ref ds_task_specify_category. If no explicit category is given, the label "default" is used. **/

	category_allocation_t resource_request; /**< See @ref category_allocation_t */
	ds_schedule_t worker_selection_algorithm; /**< How to choose worker to run the task. */
	double priority;             /**< The priority of this task relative to others in the queue: higher number run earlier. */
	int max_retries;             /**< Number of times the task is tried to be executed on some workers until success. If less than one, the task is retried indefinitely. See try_count below.*/
	int64_t min_running_time;    /**< Minimum time (in seconds) the task needs to run. (see ds_worker --wall-time)*/

	/***** Results of task once it has reached completion. *****/
  
	ds_result_t result;          /**< The result of the task (see @ref ds_result_t */
	int exit_code;               /**< The exit code of the command line. */
	char *output;                /**< The standard output of the task. */
	char *host;                  /**< The address and port of the host on which it ran. */
	char *hostname;              /**< The name of the host on which it ran. */

	/***** Internal counters for fault tolerance. *****/
	int try_count;               /**< The number of times the task has been dispatched to a worker. If larger than max_retries, the task failes with @ref DS_RESULT_MAX_RETRIES. */
	int exhausted_attempts;      /**< Number of times the task failed given exhausted resources. */
	int fast_abort_count;        /**< Number of times this task has been terminated for running too long. */

	/***** Metrics available to the user at completion through ds_task_get_metric.  *****/
	/* All times in microseconds */
	/* A time_when_* refers to an instant in time, otherwise it refers to a length of time. */

	timestamp_t time_when_submitted;    /**< The time at which this task was added to the queue. */
	timestamp_t time_when_done;         /**< The time at which the task is mark as retrieved, after transfering output files and other final processing. */

	timestamp_t time_when_commit_start; /**< The time when the task starts to be transfered to a worker. */
	timestamp_t time_when_commit_end;   /**< The time when the task is completely transfered to a worker. */

	timestamp_t time_when_retrieval;    /**< The time when output files start to be transfered back to the manager. time_done - time_when_retrieval is the time taken to transfer output files. */

	timestamp_t time_workers_execute_last;                 /**< Duration of the last complete execution for this task. */
	timestamp_t time_workers_execute_all;                  /**< Accumulated time for executing the command on any worker, regardless of whether the task completed (i.e., this includes time running on workers that disconnected). */
	timestamp_t time_workers_execute_exhaustion;           /**< Accumulated time spent in attempts that exhausted resources. */
	timestamp_t time_workers_execute_failure;              /**< Accumulated time for runs that terminated in worker failure/disconnection. */

	int64_t bytes_received;                                /**< Number of bytes received since task has last started receiving input data. */
	int64_t bytes_sent;                                    /**< Number of bytes sent since task has last started sending input data. */
	int64_t bytes_transferred;                             /**< Number of bytes transferred since task has last started transferring input data. */

	struct rmsummary *resources_allocated;                 /**< Resources allocated to the task its latest attempt. */
	struct rmsummary *resources_measured;                  /**< When monitoring is enabled, it points to the measured resources used by the task in its latest attempt. */
	struct rmsummary *resources_requested;                 /**< Number of cores, disk, memory, time, etc. the task requires. */

	char *monitor_output_directory;			      /**< Custom output directory for the monitoring output files. If NULL, save to directory from @ref ds_enable_monitoring */
	char *monitor_snapshot_file;                          /**< Filename the monitor checks to produce snapshots. */
};

int  ds_task_update_result(struct ds_task *t, ds_result_t new_result );
void ds_task_specify_resources(struct ds_task *t, const struct rmsummary *rm);
void ds_task_clean( struct ds_task *t, int full_clean );

const char *ds_task_state_string( ds_task_state_t task_state );


#endif