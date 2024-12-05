# Distributed Systems

A fault-tolerant key-value store implementation inspired by Amazon’s Dynamo, developed for the Distributed Systems course at FCUL (Faculty of Sciences - University of Lisbon).

This system uses a binary search tree for storage and ZooKeeper for coordination, featuring passive replication with a fixed primary server model.

## Architecture

- **Primary Server:** Handles write operations from clients, replicates to backup
- **Backup Server**: Receives replicated writes from primary, handles read operations
- **ZooKeeper**: Manages server coordination and fault detection
- **Clients**: Connect through ZooKeeper, send writes to primary and reads to backup

## Features

- Primary-Backup replication model
- Automatic failover handling
- Write operation propagation
- ZooKeeper-based server coordination
- Operation verification system
- Fault detection and recovery
- Thread-safe operations

## Operations

- Write operations (Primary server):

  - Put: Insert/update key-value pairs
  - Delete: Remove entries

- Read operations (Backup server):

  - Get: Retrieve values
  - Size: Get tree size
  - Height: Get tree height
  - GetKeys: List all keys
  - Verify: Check operation status

## Usage

### Prerequisites

- Protocol Buffers (protoc-c)
- ZooKeeper
- GCC or compatible compiler

### Build

Run make in the project directory:

```bash
make
```

### Run

1. Start ZooKeeper: Ensure ZooKeeper is running.
2. Run the Server: Start the primary or backup server.

```bash
./tree-server <port> <zookeeper_ip>:<zookeeper_port>
```

3. Run the Client: Connect to the server.

```bash
./tree-client <zookeeper_ip>:<zookeeper_port>
```

## Contributors

- Guilherme Lopes
- [Ricardo Banon](https://www.linkedin.com/in/ricardobanon/)
- [António Pereira](https://www.linkedin.com/in/antonio-fernando-pereira/)

## Known Issues

Client may hang when attempting to start after backup server transforms into primary
