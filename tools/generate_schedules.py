#!/usr/bin/env python3
"""
Generate ScheduleData.h from GTFS data for ESP32-C6 offline mode.

Downloads GTFS feed from opentransportdata.swiss and extracts schedules for:
- Tram 7 (all trips except to Wollishoferplatz)
- Bus 72 (all trips)
at Zürich, Morgental station.

Output: Human-readable ScheduleData.h file
"""

import csv
import os
import sys
import shutil
import zipfile
import urllib.request
from collections import defaultdict
from datetime import datetime

# Configuration
GTFS_URL = "https://data.opentransportdata.swiss/dataset/6cca1dfb-e53d-4da8-8d49-4797b3e768e3/resource/d5d9aac1-8251-4dda-9537-ae97cbb810c6/download/gtfs_fp2025_20251211.zip"
STATION_NAME = "Morgental"
STATION_CITY = "Zürich"
TRAM_7_ROUTE = "7"
BUS_72_ROUTE = "72"
EXCLUDE_DESTINATION = "Wollishoferplatz"
OUTPUT_FILE = "../firmware/tramli/ScheduleData.h"

def download_gtfs(url, output_path="gtfs.zip"):
    """Download GTFS feed from URL."""
    print(f"Downloading GTFS feed...")
    try:
        urllib.request.urlretrieve(url, output_path)
        print(f"Downloaded to {output_path}")
        return output_path
    except Exception as e:
        print(f"Error downloading GTFS: {e}")
        return None

def extract_gtfs(zip_path, extract_to="gtfs_data"):
    """Extract GTFS zip file."""
    print(f"Extracting GTFS data to {extract_to}...")
    os.makedirs(extract_to, exist_ok=True)
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_to)
    print("Extraction complete")
    return extract_to

def load_gtfs_data(gtfs_dir):
    """Load relevant GTFS files into memory."""
    print("Loading GTFS data...")

    # Load stops
    stops = {}
    with open(os.path.join(gtfs_dir, 'stops.txt'), 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if STATION_NAME in row['stop_name'] and STATION_CITY in row['stop_name']:
                stops[row['stop_id']] = row

    if not stops:
        print(f"Error: No stops found matching '{STATION_CITY}, {STATION_NAME}'")
        return None

    print(f"Found {len(stops)} matching stops")

    # Load routes
    routes = {}
    with open(os.path.join(gtfs_dir, 'routes.txt'), 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['route_short_name'] in [TRAM_7_ROUTE, BUS_72_ROUTE]:
                routes[row['route_id']] = row

    print(f"Found {len(routes)} matching routes")

    # Load trips
    trips = {}
    with open(os.path.join(gtfs_dir, 'trips.txt'), 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['route_id'] in routes:
                # Filter out Tram 7 to Wollishoferplatz
                if routes[row['route_id']]['route_short_name'] == TRAM_7_ROUTE:
                    if EXCLUDE_DESTINATION in row.get('trip_headsign', ''):
                        continue
                trips[row['trip_id']] = row

    print(f"Found {len(trips)} matching trips (excluding Tram 7 to {EXCLUDE_DESTINATION})")

    # Load stop_times for our station
    stop_times = defaultdict(list)
    with open(os.path.join(gtfs_dir, 'stop_times.txt'), 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['trip_id'] in trips and row['stop_id'] in stops:
                stop_times[row['trip_id']].append(row)

    print(f"Found stop times for {len(stop_times)} trips")

    # Load calendar to determine service days
    calendar = {}
    with open(os.path.join(gtfs_dir, 'calendar.txt'), 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            calendar[row['service_id']] = row

    return {
        'stops': stops,
        'routes': routes,
        'trips': trips,
        'stop_times': stop_times,
        'calendar': calendar
    }

def parse_time(time_str):
    """Parse GTFS time string (HH:MM:SS) to (hour, minute)."""
    parts = time_str.split(':')
    hours = int(parts[0])
    minutes = int(parts[1])
    # Handle times >= 24:00:00 (next day) - skip them
    if hours >= 24:
        return None
    return (hours, minutes)

def get_day_mask(service):
    """Convert GTFS calendar to day mask."""
    mask = 0
    days = ['monday', 'tuesday', 'wednesday', 'thursday', 'friday', 'saturday', 'sunday']
    for i, day in enumerate(days):
        if service.get(day, '0') == '1':
            mask |= (1 << i)
    return mask

def build_schedule(gtfs_data, route_number):
    """Build schedule data structure for a specific route."""
    print(f"\nBuilding schedule for route {route_number}...")

    # Dictionary to store unique departures: (hour, minute, day_mask) -> destination
    departures_dict = {}

    # Process each trip
    for trip_id, trip_data in gtfs_data['trips'].items():
        route = gtfs_data['routes'][trip_data['route_id']]

        if route['route_short_name'] != route_number:
            continue

        # Get service days from calendar
        service_id = trip_data['service_id']
        if service_id not in gtfs_data['calendar']:
            continue

        service = gtfs_data['calendar'][service_id]
        day_mask = get_day_mask(service)

        if day_mask == 0:
            continue

        # Get stop times for this trip
        trip_stop_times = gtfs_data['stop_times'].get(trip_id, [])
        if not trip_stop_times:
            continue

        # Sort by stop sequence
        trip_stop_times.sort(key=lambda x: int(x['stop_sequence']))

        # Find our station in this trip
        for stop_time in trip_stop_times:
            if stop_time['stop_id'] in gtfs_data['stops']:
                # Parse departure time
                time_tuple = parse_time(stop_time['departure_time'])
                if not time_tuple:
                    continue

                hour, minute = time_tuple

                # Get destination (headsign)
                destination = trip_data.get('trip_headsign', 'Unknown')
                # Remove "Zürich, " prefix if present
                if destination.startswith(f"{STATION_CITY}, "):
                    destination = destination[len(f"{STATION_CITY}, "):]

                # Create unique key
                key = (hour, minute, day_mask)
                # Keep shortest destination name for this time
                if key not in departures_dict or len(destination) < len(departures_dict[key]):
                    departures_dict[key] = destination

    # Convert to list and sort
    departures = []
    for (hour, minute, day_mask), destination in departures_dict.items():
        departures.append({
            'hour': hour,
            'minute': minute,
            'day_mask': day_mask,
            'destination': destination
        })

    # Sort by time
    departures.sort(key=lambda x: (x['hour'], x['minute']))

    print(f"Generated {len(departures)} unique departure times")

    return {
        'route_number': route_number,
        'departures': departures
    }

def format_day_mask(mask):
    """Format day mask as comment."""
    if mask == 0x1F:  # Mon-Fri
        return "0x1F"  # Weekdays
    elif mask == 0x7F:  # All days
        return "0x7F"  # All week
    elif mask == 0x60:  # Sat-Sun
        return "0x60"  # Weekend
    else:
        return f"0x{mask:02X}"

def write_header_file(tram7_schedule, bus72_schedule, output_path):
    """Write schedule to .h file."""
    print(f"\nWriting schedule to {output_path}...")

    with open(output_path, 'w', encoding='utf-8') as f:
        # Write header
        f.write("""// ScheduleData.h - Auto-generated schedule data for offline mode
// Generated from GTFS data
// DO NOT EDIT MANUALLY - Regenerate using generate_schedules.py

#ifndef SCHEDULE_DATA_H
#define SCHEDULE_DATA_H

// Schedule entry structure
struct ScheduleTime {
  unsigned char hour;      // 0-23
  unsigned char minute;    // 0-59
  unsigned char dayMask;   // Bit mask: 0=Mon, 1=Tue, 2=Wed, 3=Thu, 4=Fri, 5=Sat, 6=Sun
                          // 0x1F = weekdays (Mon-Fri)
                          // 0x7F = all days
                          // 0x60 = weekend (Sat-Sun)
  const char* destination;
};

""")

        # Write Tram 7 schedule
        f.write("// Tram 7 Schedule (to Stettbach only, excluding Wollishoferplatz)\n")
        f.write(f"// Generated from GTFS data: {len(tram7_schedule['departures'])} departure times\n")
        f.write("const ScheduleTime tram7Schedule[] = {\n")

        for dep in tram7_schedule['departures']:
            mask_str = format_day_mask(dep['day_mask'])
            dest = dep['destination'].replace('"', '\\"')  # Escape quotes
            f.write(f"  {{{dep['hour']:2d}, {dep['minute']:2d}, {mask_str}, \"{dest}\"}},\n")

        f.write("};\n\n")
        f.write(f"const int tram7ScheduleSize = {len(tram7_schedule['departures'])};\n\n")

        # Write Bus 72 schedule
        f.write("// Bus 72 Schedule (all directions from Morgental)\n")
        f.write(f"// Generated from GTFS data: {len(bus72_schedule['departures'])} departure times\n")
        f.write("const ScheduleTime bus72Schedule[] = {\n")

        for dep in bus72_schedule['departures']:
            mask_str = format_day_mask(dep['day_mask'])
            dest = dep['destination'].replace('"', '\\"')  # Escape quotes
            f.write(f"  {{{dep['hour']:2d}, {dep['minute']:2d}, {mask_str}, \"{dest}\"}},\n")

        f.write("};\n\n")
        f.write(f"const int bus72ScheduleSize = {len(bus72_schedule['departures'])};\n\n")

        # Write helper function
        f.write("""// Helper function to check if schedule applies to current day
inline bool scheduleAppliesToday(unsigned char dayMask, int dayOfWeek) {
  // dayOfWeek: 0=Monday, 6=Sunday
  return (dayMask & (1 << dayOfWeek)) != 0;
}

#endif // SCHEDULE_DATA_H
""")

    print(f"Written {output_path}")

    # Print file size
    size = os.path.getsize(output_path)
    print(f"File size: {size} bytes ({size / 1024:.1f} KB)")

def cleanup_files():
    """Clean up temporary files."""
    print("\nCleaning up temporary files...")
    try:
        if os.path.exists("gtfs.zip"):
            os.remove("gtfs.zip")
            print("Removed gtfs.zip")
        if os.path.exists("gtfs_data"):
            shutil.rmtree("gtfs_data")
            print("Removed gtfs_data/")
        print("Cleanup complete!")
    except Exception as e:
        print(f"Warning: Could not clean up some files: {e}")

def main():
    """Main function."""
    print("=" * 70)
    print("GTFS Schedule Generator for ESP32-C6 Offline Mode")
    print("Generates human-readable ScheduleData.h from GTFS feed")
    print("=" * 70)

    # Check if we should download or use existing GTFS data
    gtfs_dir = "gtfs_data"
    gtfs_zip = "gtfs.zip"

    if not os.path.exists(gtfs_dir):
        # Download GTFS
        if not os.path.exists(gtfs_zip):
            print(f"\nDownloading GTFS data from Swiss Open Transport Data...")
            gtfs_zip = download_gtfs(GTFS_URL, gtfs_zip)
            if not gtfs_zip:
                print("Error: Could not download GTFS data")
                return

        gtfs_dir = extract_gtfs(gtfs_zip)
    else:
        print(f"Using existing GTFS data from {gtfs_dir}")

    # Load GTFS data
    gtfs_data = load_gtfs_data(gtfs_dir)
    if not gtfs_data:
        return

    # Generate schedules
    tram7_schedule = build_schedule(gtfs_data, TRAM_7_ROUTE)
    bus72_schedule = build_schedule(gtfs_data, BUS_72_ROUTE)

    # Write header file
    write_header_file(tram7_schedule, bus72_schedule, OUTPUT_FILE)

    # Cleanup temporary files
    cleanup_files()

    print("\n" + "=" * 70)
    print("Schedule generation complete!")
    print(f"✓ Output file: {OUTPUT_FILE}")
    print(f"✓ Tram 7: {len(tram7_schedule['departures'])} departure times")
    print(f"✓ Bus 72: {len(bus72_schedule['departures'])} departure times")
    print("\nNext steps:")
    print("1. Review ScheduleData.h for accuracy")
    print("2. Compile and upload tramli.ino to ESP32-C6")
    print("3. Test offline mode by disconnecting WiFi")
    print("=" * 70)

if __name__ == "__main__":
    main()
