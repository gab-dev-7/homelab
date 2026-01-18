# Surface Pro 9 Homelab & Experiments

Personal documentation for infrastructure running on a Surface Pro 9 (Ubuntu Server) and various low-level engineering benchmarks.

## Hardware

- **Model:** Microsoft Surface Pro 9
- **Storage:** 256GB SSD
- **RAM:** 8GB
- **OS:** Ubuntu Server

## Infrastructure

The services run via Docker Compose, utilizing Caddy as a reverse proxy and CrowdSec for intrusion prevention.

### Core Stack

- **Proxy:** Caddy (managing certificates and routing)
- **Database:** PostgreSQL 16 (shared instance)
- **Cache:** Redis 7
- **Security:** CrowdSec (log monitoring)

### Services

- **AdGuard Home:** DNS ad-blocking and network security
- **OwnCloud:** File hosting
- **Vikunja:** Task management
- **Wallos:** Subscription and finance tracking
- **Linkwarden:** Bookmark management
- **Uptime Kuma:** Service monitoring

## Benchmarks

*Work in Progress:* I am currently testing the overhead of the Node.js event loop vs. raw C implementations under high concurrent load on this specific mobile CPU architecture.

## Usage

This repository is for documentation purposes. Environment variables have been stripped.

1. Review `docker-compose.yml` for service definitions.
2. Review `Caddyfile` for routing configurations.
3. `.env` file is required for `POSTGRES_USER`, `POSTGRES_PASSWORD`, and domain configurations.

---
