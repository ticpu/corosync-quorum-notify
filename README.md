# Corosync Quorum Notify

A lightweight bridge between Corosync cluster quorum states and Keepalived, designed to replace the traditional Pacemaker stack for simpler high-availability setups.

## Background

Traditional Corosync/Pacemaker stacks can be overly complex for simple high-availability needs. This project was created to:

- Provide a lighter alternative to Pacemaker
- Enable direct integration with Keepalived
- Offer simple quorum state monitoring
- Reduce complexity in HA setups

## Features

- Monitors Corosync quorum state changes in real-time
- Notifies external programs (like Keepalived) of quorum changes
- Provides systemd integration with watchdog support
- Maintains a simple and focused approach to cluster state management

## Building

To compile the program, you'll need the following dependencies:
- libquorum (corosync)
- libsystemd
- libcpg (corosync)

```bash
gcc -Wall -o corosync-quorum-notify corosync-quorum-notify.c -lquorum -lsystemd -lcpg
```

To create a Debian package, use `./build.sh`, this will build the package using podman from a clean Debian root.

## Installation

1. Build and install the application:
   ```bash
   ./build.sh
   sudo dpkg -i corosync-quorum-notify_*.deb
   ```
2. Create configuration file:
   ```bash
   echo 'NOTIFY_APP=/usr/local/sbin/keepalived-quorum.sh' | sudo tee /etc/default/corosync-quorum-notify
   ```
3. Integrate with keepalived:
   ```
   track_file corosync {
       file "/run/corosync-quorum-notify/corosync-quorum-status.txt"
       weight 0
   }
   ```
4. Enable and start the service:
   ```bash
   sudo systemctl enable corosync-quorum-notify
   sudo systemctl start corosync-quorum-notify
   ```

## How It Works

The program monitors Corosync quorum state changes and executes a specified external program with the following arguments:

```bash
external_program quorum_state current_node node_list
```
Where:
- `quorum_state`: Either "quorate" or "not_quorate"
- `current_node`: The ID of the current node
- `node_list`: A comma-separated list of node IDs in the Corosync cluster

The included `keepalived-quorum.sh` script processes these arguments and creates a status file that can be monitored by keepalived's track_file feature.
This will cause keepalived to transition to `FAULT` state when quorum is lost and back to `MASTER`/`BACKUP` when restored.
