#!/bin/ba

# Get battery percentage
CAPACITY=$(cat /sys/class/power_supply/BAT0/capacity)
# Get status (Charging, Discharging, Full)
STATUS=$(cat /sys/class/power_supply/BAT0/status)

# Optional: Get Temperature
TEMP_RAW=$(cat /sys/class/thermal/thermal_zone0/temp)
TEMP=$((TEMP_RAW / 1000))

# Log to console for debugging
echo "Battery: $CAPACITY% | Status: $STATUS | Temp: ${TEMP}°C"

# Push to Uptime Kuma
curl -fsS --retry 3 "https://monitor.lab.gawindlin.com/api/push/qO6BqEWD1r?status=up&msg=OK&ping=&msg=Status:${STATUS},Temp:${TEMP}C&ping=${CAPACITY}"
