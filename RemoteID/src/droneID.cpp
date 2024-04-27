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

#include "droneIDEU.h"
#include "string.h"

void droneIDEU::setup(char uas_operator[24], char uas_id[24], uint8_t uas_type, uint8_t EU_category, uint8_t EU_class)
{
    // utm things
    memset(&utm_parameters, 0, sizeof(utm_parameters));
    String tmp = String(uas_operator).substring(0,24);
    tmp.trim();
    strncpy(utm_parameters.UAS_operator, tmp.c_str(), 24);
    Serial.println(uas_operator);
    utm_parameters.UA_type = uas_type;
    
    utm_parameters.ID_type = 1;
    utm_parameters.region = 1;
    utm_parameters.EU_category = EU_category;
    utm_parameters.EU_class = EU_class; 
    tmp = String(uas_id).substring(0,24);
    tmp.trim();
    strcpy(utm_parameters.UAV_id, tmp.c_str());
    
    squitter.init(&utm_parameters);

    memset(&utm_data, 0, sizeof(utm_data));
}
/**
 * Setter for double GPS coordinates in centidegrees
 * @param lat
 * @param lon
 */
void droneIDEU::set_lat_lon(double lat, double lon)
{
    _old_latitude = utm_data.latitude_d;
    _old_longitude = utm_data.longitude_d;
    utm_data.latitude_d = lat;
    utm_data.longitude_d = lon;
    _travelled_distance += distanceBetween(lat, lon, _old_latitude, _old_longitude);
}
/**
 * Setter for altitude in MSL (Mean Sea Level) in meters
 * @param alt
 */
void droneIDEU::set_altitude(float alt)
{
    utm_data.alt_msl_m = alt;
}
/**
 * Setter for height above ground in relation to take-off point in m
 * @param height
 */
void droneIDEU::set_heigth(float height)
{
    utm_data.alt_msl_m = height;
    utm_data.alt_agl_m = utm_data.base_alt_m + height;
}
/**
 * Setter for double GPS-Home coordinates in centidegrees
 * @param lat
 * @param lon
 */
void droneIDEU::set_home_lat_lon(double lat, double lon, float height)
{
    utm_data.base_latitude = lat;
    utm_data.base_longitude = lon;
    utm_data.base_alt_m = height;
    utm_data.latitude_d = lat;
    utm_data.longitude_d = lon;
    set_heigth(height);
    utm_data.base_valid = 1;
}
/**
 * Set time
 * @param second
 * @param minute
 * @param hour
 * @param day
 * @param month
 * @param year
 */
void droneIDEU::set_time(u_int8_t second, u_int8_t minute, u_int8_t hour, u_int8_t day, u_int8_t month, u_int8_t year)
{
    time_t time_2;
    struct tm clock_tm;
    struct timeval tv = {0, 0};
    struct timezone utc = {0, 0};
    clock_tm.tm_sec = second;
    clock_tm.tm_min = minute;
    clock_tm.tm_hour = hour;
    clock_tm.tm_mday = day;
    clock_tm.tm_mon = month;
    clock_tm.tm_year = year;
    tv.tv_sec =
        time_2 = mktime(&clock_tm);

    settimeofday(&tv, NULL);
}
/**
 * Setter for number of satellites
 * @param lat
 * @param lon
 */
void droneIDEU::set_num_satellites(int num_satellites)
{
    utm_data.satellites = num_satellites;
}
/**
 * Set ground speed in knots
 * @param ground_speed
 */
void droneIDEU::set_ground_speed(int ground_speed)
{
    utm_data.speed_kn = ground_speed;
}
/**
 * Drone heading in degrees from north
 * @param heading
 */
void droneIDEU::set_heading(int heading)
{
    utm_data.heading = heading;
}

/**
 * Setter for the UAS operator
 * @param id_value
 */
void droneIDEU::set_drone_id(const char *id_value)
{
    strcpy(utm_parameters.UAS_operator, id_value);
}

/**
 * Saves the last time a frame was sent to respect the timing of one frame every 3s
 */
void droneIDEU::set_last_send()
{
    _last_send = std::chrono::high_resolution_clock::now();
    _travelled_distance = 0.0;
}

/**
 * Notifies when 3s have passed to send a new frame.
 * @return true if elapse time is > 1s
 */
bool droneIDEU::has_pass_time() const
{
    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - _last_send;
    return elapsed.count() >= FRAME_TIME_LIMIT;
}
/**
 * Notifies you when the drone has moved more than 30m in less than 1s.
 * @return true if distance travelled > 10m
 */
bool droneIDEU::has_pass_distance() const
{
    return _travelled_distance >= FRAME_DISTANCE_LIMIT;
}
/**
 * Notifies you if the distance or time condition has passed to send a new frame.
 * @return
 */
bool droneIDEU::time_to_send() const
{
    return has_pass_time() || has_pass_distance();
}
/**
 * Transmits WiFi beacon
*/
void droneIDEU::send_beacon_frame()
{
    time_t time_2;
    struct tm *gmt;
    struct tm timeDetails;
    time(&time_2);
    localtime_r(&time_2, &timeDetails);
    gmt = gmtime(&time_2);
    utm_data.seconds = gmt->tm_sec;
    utm_data.minutes = gmt->tm_min;
    utm_data.hours = gmt->tm_hour;

    Serial.printf("lon: %f \t lat : %f \n", utm_data.longitude_d, utm_data.latitude_d);

    squitter.transmit(&utm_data);
}

void droneIDEU::close(){
    
}

// Taken from TinyGPS++
/**
 * Calculates an approximation of the distance between two WS84 coordinates (GPS)
 * @param lat1
 * @param long1
 * @param lat2
 * @param long2
 * @return distance en m
 */
double droneIDEU::distanceBetween(double lat1, double long1, double lat2, double long2)
{
    // returns distance in meters between two positions, both specified
    // as signed decimal-degrees latitude and longitude. Uses great-circle
    // distance computation for hypothetical sphere of radius 6372795 meters.
    // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
    // Courtesy of Maarten Lamers
    double delta = radians(long1 - long2);
    const double sdlong = sin(delta);
    const double cdlong = cos(delta);
    lat1 = radians(lat1);
    lat2 = radians(lat2);
    const double slat1 = sin(lat1);
    const double clat1 = cos(lat1);
    const double slat2 = sin(lat2);
    const double clat2 = cos(lat2);
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = sq(delta);
    delta += sq(clat2 * sdlong);
    delta = sqrt(delta);
    const double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);
    return abs(delta * 6372795);
}
