/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdint>
#include <chrono>
#include <string.h>
#include <Arduino.h>
#include "id_open.h"

class droneIDEU
{
public:
    /**
     * Constructor
     */
    droneIDEU() {}

    void setup(char uas_operator[24], char uas_id[24], uint8_t uas_type, uint8_t EU_category, uint8_t EU_class);

    String getID();

    /**
     * Setter for double GPS coordinates in centidegrees
     * @param lat
     * @param lon
     */
    void set_lat_lon(double lat, double lon);
    /**
     * Setter for altitude in MSL (Mean Sea Level) in meters
     * @param alt
     */
    void set_altitude(float alt);
    /**
     * Setter for height above ground in relation to take-off point in m
     * @param height
     */
    void set_heigth(float height);
    /**
     * Setter for double GPS-Home coordinates in centidegrees
     * @param lat
     * @param lon
     */
    void set_home_lat_lon(double lat, double lon, float height);
    /**
     * Set time
     * @param second
     * @param minute
     * @param hour
     * @param day
     * @param month
     * @param year
     * //TODO set timezone
     */
    void set_time(u_int8_t second, u_int8_t minute, u_int8_t hour, u_int8_t day, u_int8_t month, u_int8_t year);
    /**
     * Setter for number of satellites
     * @param lat
     * @param lon
     */
    void set_num_satellites(int num_satellites);
    /**
     * Set ground speed in knots
     * @param ground_speed
     */
    void set_ground_speed(int ground_speed);
    /**
     * Drone heading in degrees from north
     * @param heading
     */
    void set_heading(int heading);

    /**
     * Setter for the UAS operator
     * @param id_value
     */
    void set_drone_id(const char *id_value);

    /**
    * Saves the last time a frame was sent to respect the timing of one frame every 3s
     */
    void set_last_send();

    /**
     * Notifies when 3s have passed to send a new frame.
     * @return true if elapse time is > 1s
     */
    bool has_pass_time() const;
    /**
     * Notifies you when the drone has moved more than 30m in less than 1s.
 * @return true if distance travelled > 10m
     */
    bool has_pass_distance() const;
    /**
     * Notifies you if the distance or time condition has passed to send a new frame.
     */
    bool time_to_send() const;

    void send_beacon_frame();

    void close();


private:
    ID_OpenDrone squitter;
    UTM_Utilities utm_utils;

    struct UTM_parameters utm_parameters;
    struct UTM_data utm_data;

    /**
     * Time limit for transmition
     */
    static constexpr uint8_t FRAME_TIME_LIMIT = 1; // in s
    /**
     * Distance limit for transmition
     */
    static constexpr double FRAME_DISTANCE_LIMIT = 10.0; // in m
    ///////////////////////////////////////////////////////

    // conversion constants and params
    const double angle_rad2deg = 180.0 / M_PI;
    const double speed_ms2kn = 1.9438452;
    const double max_accel = 10 * 1000;     // in mms-1
    const double max_speed = 25;            // in ms-2
    const double max_climbrate = 10 * 1000; // in mms-1
    const double max_height = 200;          // in m over takeoff

    uint8_t _droneID[30 + 1]; // +1 for null termination
    std::chrono::system_clock::time_point _last_send = std::chrono::system_clock::now();
    // for travelled distance calculation
    double _old_latitude;
    double _old_longitude;
    double _travelled_distance;

    // Taken from TinyGPS++
    /**
     * Calculate distance from two coordinates
     * @param lat1
     * @param long1
     * @param lat2
     * @param long2
     * @return distance en m
     */
    static double distanceBetween(double lat1, double long1, double lat2, double long2);
};
