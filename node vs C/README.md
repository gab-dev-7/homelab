# Benchmark: C vs Node.js

An experiment to test the limits of raw C against the V8 JavaScript engine. Read the full story on [gawindlin.com](https://gawindlin.com/blog/fith-post/).

## The Contenders

1. **`server.js`**: Standard Node.js HTTP server (Event Loop).
2. **`server_blocking.c`**: Naive implementation using blocking I/O.
3. **`server_epoll.c`**: Single-threaded implementation using Linux `epoll` and HTTP Keep-Alive.
4. **`server_mt.c`**: Multi-threaded implementation using `pthread`, `epoll`, and `SO_REUSEPORT`.

## The Results

Benchmarks ran using `wrk` (12 threads, 400 connections) over a VPN.

| Implementation          | Req/Sec    | Notes                       |
| :---------------------- | :--------- | :-------------------------- |
| **Node.js**             | 15,859     | The Gold Standard           |
| **Blocking C**          | 6,529      | 100% Fail rate              |
| **Epoll C (1 Thread)**  | 13,578     | Faster, but not fast enough |
| **Epoll C (4 Threads)** | **17,059** | **Victory**                 |

## How to Run

Compile the multi-threaded server:

```bash
gcc -O3 -pthread server_mt.c -o server_mt
./server_mt
```
