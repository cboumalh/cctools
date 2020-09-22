
#include "dataswarm_manager.h"
#include "dataswarm_worker_rep.h"
#include "dataswarm_blob_rep.h"
#include "dataswarm_task_rep.h"
#include "dataswarm_rpc.h"

#include "debug.h"
#include "stringtools.h"
#include "itable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int wait_for_rpcs(struct dataswarm_manager *m, struct dataswarm_worker_rep *r) {
	int done   = 0;
	int all_ok = 1;
	uint64_t key;

	while(1) {
		done = 1;

		/* check blob responses */
		struct dataswarm_blob_rep *b;
		itable_firstkey(r->blob_of_rpc);
		while((itable_nextkey(r->blob_of_rpc, &key, (void **) &b))) {
			if(b->result == DS_RESULT_PENDING) {
				done = 0;
			} else if (b->result != DS_RESULT_SUCCESS) {
				debug(D_DATASWARM, "rpc for blob %s failed with: %d", b->blobid, b->result);
				all_ok = 0;
			}
		}

		struct dataswarm_task_rep *t;
		itable_firstkey(r->task_of_rpc);
		while((itable_nextkey(r->task_of_rpc, &key, (void **) &t))) {
			if(t->result == DS_RESULT_PENDING) {
				done = 0;
			} else if (b->result != DS_RESULT_SUCCESS) {
				debug(D_DATASWARM, "rpc for task %s failed with: %d", t->taskid, t->result);
				all_ok = 0;
			}
		}

		if(done) break;
		dataswarm_rpc_get_response(m,r);
		//sleep(1);
	}

	return all_ok;
}

void dataswarm_test_script( struct dataswarm_manager *m, struct dataswarm_worker_rep *r )
{
	const char *bloba = "abc123";
	const char *blobb = "xyz456";

	dataswarm_manager_add_blob_to_worker(m, r, bloba),
		dataswarm_manager_add_blob_to_worker(m, r, blobb),

		dataswarm_rpc_blob_delete(m,r,bloba);
	dataswarm_rpc_blob_delete(m,r,blobb);
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error with rpc delete. But that may be ok.");
		return;
	}

	dataswarm_rpc_blob_create(m,r,bloba,100000,NULL);
	dataswarm_rpc_blob_create(m,r,blobb,100000,NULL);
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error with an rpc. Cannot continue.");
		return;
	}

	dataswarm_rpc_blob_put(m,r,bloba,"/usr/share/dict/words");
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error with an rpc. Cannot continue.");
		return;
	}

	dataswarm_rpc_blob_commit(m,r,bloba);
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error with an rpc. Cannot continue.");
		return;
	}

	/* Create a simple task that reads from bloba mounted as myinput and writes to blob mounted as stdout. */
	struct jx *taskinfo = jx_objectv("command",   jx_string("wc -l myinput"),
			"namespace", jx_objectv(bloba, jx_objectv("type", jx_string("path"),
					"path", jx_string("myinput"),
					"mode", jx_string("R"),
					NULL),
				blobb, jx_objectv("type", jx_string("stdout"),
					NULL),
				NULL),
			NULL);


	/* submit task to manager */
	char *taskid = dataswarm_manager_submit_task(m, taskinfo);

	/* declare task in worker */
	dataswarm_manager_add_task_to_worker(m,r,taskid);

	/* send task to worker */
	dataswarm_rpc_task_submit(m,r,taskid);
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error sending task to worker.");
	}

	/* sleep while task is executed */
	sleep(5);

	dataswarm_rpc_blob_get(m,r,blobb,"/dev/stdout");
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error with an the get rpc. Cleanup continues...");
	}

	dataswarm_rpc_task_remove(m,r,taskid);
	free(taskid);

	dataswarm_rpc_blob_delete(m,r,bloba);
	//dataswarm_rpc_blob_delete(m,r,blobb);
	if(!wait_for_rpcs(m, r)) {
		debug(D_DATASWARM, "There was an error with an rpc. Cannot continue.");
		return;
	}


	debug(D_DATASWARM, "Done testing this worker.");
}

void dataswarm_test_script_old_sync( struct dataswarm_manager *m, struct dataswarm_worker_rep *r )
{
	const char *bloba = "abc123";
	const char *blobb = "xyz456";

	dataswarm_rpc_blob_delete(m,r,bloba);
	dataswarm_rpc_blob_delete(m,r,blobb);

	sleep(1);

	dataswarm_rpc_blob_create(m,r,bloba,100000,NULL);
	dataswarm_rpc_blob_put(m,r,bloba,"/usr/share/dict/words");
	dataswarm_rpc_blob_commit(m,r,bloba);


	dataswarm_rpc_blob_create(m,r,blobb,100000,NULL);

	sleep(1);

	/* Create a simple task that reads from bloba mounted as myinput and writes to blob mounted as stdout. */
	char *taskinfo = string_format("{ \"task-id\": \"%s\",\"command\" : \"wc -l myinput\", \"namespace\" : { \"%s\" : {\"type\" : \"path\", \"path\" : \"myinput\", \"mode\" : \"R\" }, \"%s\" : {\"type\" : \"stdout\" } } }","t93",bloba,blobb);
	dataswarm_rpc_task_submit(m,r,taskinfo);
	free(taskinfo);

	sleep(5);

	// need to wait for task to complete
	dataswarm_rpc_blob_get(m,r,blobb,"/dev/stdout");

	sleep(1);

	dataswarm_rpc_task_remove(m,r,"t93");

	sleep(1);


	dataswarm_rpc_blob_delete(m,r,bloba);
	dataswarm_rpc_blob_delete(m,r,blobb);

}

/* vim: set noexpandtab tabstop=4: */
