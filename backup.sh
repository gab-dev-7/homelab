#!/bin/bash

# ------------------------------------------------------------------------------
# SAFETY SWITCH (CRITICAL FOR MONITORING)
# ------------------------------------------------------------------------------
# set -e: Exit immediately if any command exits with a non-zero status.
# This ensures we don't send a "Success" ping to Kuma if the DB dump failed.
set -e

# ------------------------------------------------------------------------------
# CONFIGURATION
# ------------------------------------------------------------------------------
BACKUP_DIR="$HOME/backups"
PROJECT_DIR="$HOME/homelab"
DATE=$(date +%Y-%m-%d_%H-%M-%S)
RETENTION_DAYS=7

# Uptime Kuma Push URL
# We added logic to calculate the duration in seconds for the dashboard
START_TIME=$(date +%s)
KUMA_URL="https://monitor.lab.gawindlin.com/api/push/wxOtJ4T8C6?status=up&msg=OK&ping="

# Create backup directory if it doesn't exist
mkdir -p "$BACKUP_DIR"

echo "Starting backup for $DATE..."

# ------------------------------------------------------------------------------
# 1. DATABASE BACKUP (PostgreSQL)
# ------------------------------------------------------------------------------
echo "--> Dumping PostgreSQL..."
docker exec -t postgres pg_dumpall -c -U admin >"$BACKUP_DIR/postgres_dump_$DATE.sql"

# ------------------------------------------------------------------------------
# 2. VOLUME BACKUPS
# ------------------------------------------------------------------------------
echo "--> Backing up Docker Volumes..."

backup_volume() {
  VOLUME_NAME=$1
  echo "    Backing up $VOLUME_NAME..."
  docker run --rm \
    -v "$VOLUME_NAME":/volume_data \
    -v "$BACKUP_DIR":/backup \
    busybox tar -czf "/backup/${VOLUME_NAME}_$DATE.tar.gz" -C /volume_data .
}

backup_volume "homelab_adguard_conf"
backup_volume "homelab_adguard_work"
backup_volume "homelab_wallos_db"
backup_volume "homelab_grimoire_data"
backup_volume "homelab_vikunja_files"

# ------------------------------------------------------------------------------
# 3. CONFIG FILES
# ------------------------------------------------------------------------------
echo "--> Backing up Config Files..."
tar -czf "$BACKUP_DIR/config_files_$DATE.tar.gz" -C "$PROJECT_DIR" .env Caddyfile docker-compose.yml init-db.sh

# ------------------------------------------------------------------------------
# 4. COMPRESSION & CLEANUP
# ------------------------------------------------------------------------------
echo "--> Compressing final archive..."
mkdir -p "$BACKUP_DIR/$DATE"
mv "$BACKUP_DIR"/*_"$DATE"* "$BACKUP_DIR/$DATE/"

tar -czf "$BACKUP_DIR/full_backup_$DATE.tar.gz" -C "$BACKUP_DIR" "$DATE"
rm -rf "$BACKUP_DIR/$DATE"

echo "--> Cleaning up backups older than $RETENTION_DAYS days..."
find "$BACKUP_DIR" -name "full_backup_*.tar.gz" -type f -mtime +$RETENTION_DAYS -delete

# ------------------------------------------------------------------------------
# 5. NOTIFY UPTIME KUMA
# ------------------------------------------------------------------------------
END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

echo "Backup Complete! File: $BACKUP_DIR/full_backup_$DATE.tar.gz"

# Send "UP" signal to Kuma with the duration included
# -fsS: Fail silently on server error, but show curl errors
# --retry: Retry 5 times if network blips
curl -k -fsS --retry 5 "${KUMA_URL}&ping=${DURATION}"
