/*
 * quorum-notify: A utility to call an external program upon quorum status changes in a Corosync cluster.
 *
 * BSD 3-Clause License
 *
 * Copyright (c) 2023, Jérôme Poulin <jeromepoulin@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Build: gcc -Wall -o corosync-quorum-notify corosync-quorum-notify -lquorum -lsystemd -lcpg
 *
 * Usage: ./corosync-quorum-notify external_program_to_call
 *
 * Upon quorum state change in a Corosync cluster, this program will call the specified external program
 * with arguments indicating the new quorum state and a list of current nodes. The call format will be:
 *
 * external_program quorum_state current_node node_list
 *
 * where:
 *  - quorum_state: "quorate" or "not_quorate" depending on the new quorum state.
 *  - current_node: The id of the current node.
 *  - node_list: A comma-separated list of node ids in the Corosync cluster.
 *
 * This program is meant to be run as a long-lived process in the background.
 */

#include <stdio.h>
#include <corosync/corotypes.h>
#include <corosync/cpg.h>
#include <corosync/quorum.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/poll.h>
#include <systemd/sd-daemon.h>

static char *ext_program = NULL;
static uint32_t node_id = 0;

static void quorum_notification(quorum_handle_t handle,
								uint32_t quorate,
								uint64_t ring_seq,
								uint32_t view_list_entries,
								uint32_t * view_list) {

	char command[512];
	char node_list[256] = "";
	char * quorate_text = quorate ? "quorate" : "not_quorate";
	int ret;

	for (int i = 0; i < view_list_entries; i++) {
		if (i != 0) {
			strcat(node_list, ",");
		}
		char node[32];
		snprintf(node, sizeof(node), "%u", view_list[i]);
		strcat(node_list, node);
	}

	snprintf(command, sizeof(command), "%s %s %u %s", ext_program, quorate_text, node_id, node_list);
	syslog(LOG_INFO, "quorum state change: %s, calling %s\n", quorate_text, command);
	ret = system(command);
	syslog(LOG_INFO, "command returned %d\n", ret);
}

static quorum_callbacks_t callbacks = {
		.quorum_notify_fn = quorum_notification,
};

int main(int argc, char *argv[]) {
	int err;
	quorum_handle_t handle;
	cpg_handle_t cpg_handle;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <program_to_run_on_quorum_change>\n", argv[0]);
		return 1;
	}

	openlog("corosync-quorum-notify", LOG_PERROR, LOG_DAEMON);
	ext_program = argv[1];

	err = quorum_initialize(&handle, &callbacks, NULL);
	if (err != CS_OK) {
		syslog(LOG_ERR, "Failed to initialize quorum service %d\n", err);
		return 1;
	}

	err = quorum_trackstart(handle, CS_TRACK_CHANGES);
	if (err != CS_OK) {
		syslog(LOG_ERR, "Failed to start tracking quorum changes, error %d\n", err);
		return err;
	}

	int fd;
	err = quorum_fd_get(handle, &fd); // Correct usage
	if (err != CS_OK) {
		syslog(LOG_ERR, "Failed to get quorum fd, error %d\n", err);
		return err;
	}
	struct pollfd fds = { .fd = fd, .events = POLLIN };

	err = cpg_initialize(&cpg_handle, NULL);
	if (err != CS_OK) {
		syslog(LOG_ERR, "Failed to initialize cpg service %d\n", err);
		return 1;
	}

	err = cpg_local_get(cpg_handle, &node_id);
	if (err != CS_OK) {
		syslog(LOG_ERR, "Failed to get local node id %d\n", err);
		return 1;
	}

	sd_notify(0, "READY=1");

	while(1) {
		err = poll(&fds, 1, 500);
		if (err == -1) {
			syslog(LOG_ERR, "Failed to poll, error %d\n", err);
			return err;
		}

		if (fds.revents & POLLIN) {
			err = quorum_dispatch(handle, CS_DISPATCH_ONE);
			if (err != CS_OK) {
				syslog(LOG_ERR, "Failed to dispatch quorum event, error %d\n", err);
				return err;
			}
		}

		sd_notify(0, "WATCHDOG=1");
	}
}
