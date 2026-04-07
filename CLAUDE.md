# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A C chat server implementing an epoll-based event loop, intended to demonstrate single-threaded event-driven I/O as an alternative to the thread-per-client model. The goal is to build a working server, then benchmark it against a threaded equivalent to make the concurrency tradeoff visible.

## Build and Run

```bash
# Build the server
make

# Or compile directly
gcc -o chat_server chat_server.c

# Run the server (default port 8080)
./chat_server

# Build and run the load-testing client
gcc -o load_client load_client.c
./load_client <host> <port> <num_connections>
```

## Architecture

### Core event loop pattern

```
create listening socket (non-blocking)
epoll_create1() → epoll fd
epoll_ctl() → add listening socket with EPOLLIN
loop:
    epoll_wait() → blocks until fd(s) ready
    for each event:
        if listening socket → accept(), add new client fd to epoll
        if client fd → recv(), broadcast to all other fds
        if recv() == 0 → client disconnected, epoll_ctl(EPOLL_CTL_DEL), close()
```

### Key Linux syscalls
- `epoll_create1(0)` — create the epoll instance
- `epoll_ctl(epfd, EPOLL_CTL_ADD/MOD/DEL, fd, &event)` — register/modify/remove a fd
- `epoll_wait(epfd, events, MAX_EVENTS, -1)` — block until ready; returns count of ready fds

### Level vs edge triggered
Start with level-triggered (default, `EPOLLIN`). Edge-triggered (`EPOLLIN | EPOLLET`) requires draining the entire buffer in a loop on each event or data will be missed.

### Client tracking
All connected client fds need to be stored (e.g. a flat array or fd-indexed array) so the broadcast loop can iterate them. The listening socket fd must be excluded from broadcasts.

## Benchmarking

Compare against a thread-per-client server by measuring at N=10, N=100, N=1000 simultaneous connections:
- **Latency** — time from send to receive
- **Throughput** — messages/second
- **Memory** — `cat /proc/<pid>/status | grep VmRSS` under load

To make the difference visible at lower connection counts, add a `usleep()` in the handler to simulate I/O wait and exaggerate context-switching cost.
