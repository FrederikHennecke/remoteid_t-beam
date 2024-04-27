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

#include "RemoteID.h"

/**
 * setup for the board: power, buttons, data
*/
void setup()
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    button_init();

    delay(10000);

    setupData();
}

void loop()
{
    static uint64_t gpsMap = 0;

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        Serial.println(F("No GPS detected: check wiring."));
    }

    switch (program)
    {
    case 0:
        program1(&gpsMap);
        break;
    default:
        program2();
        break;
    }
}

void program1(uint64_t *gpsMap)
{
    // Here we read the data arriving from the GPS and pass them to the TinyGPS++ library for processing.
    while (Serial1.available() > 0)
        gps.encode(Serial1.read());

    if (print_debug)
        displayInfo();

    // We deal with the case where the GPS has a problem
    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        snprintf(buff[0], sizeof(buff[0]), "T-Beam GPS");
        snprintf(buff[1], sizeof(buff[1]), "No GPS detected");
        if (!print_debug)
        {
            Serial.println(buff[1]);
        }
        return;
    }
    // In case of invalid GPS position
    if (!gps.location.isValid())
    {
        if (millis() - *gpsMap > 1000)
        {
            snprintf(buff[0], sizeof(buff[0]), "T-Beam GPS");
            snprintf(buff[1], sizeof(buff[1]), "Positioning(%llu)", gpsSec++);
            if (!print_debug)
            {
                Serial.println(buff[1]);
            }
            *gpsMap = millis();
        }
    }
    else
    {
        // The case is processed if the GPS position is valid.
        // The starting point is entered when the accuracy is satisfactory.
        if (!is_home_set)
        {
            if (gps.satellites.value() >= min_sattelites && gps.hdop.hdop() < min_accuracy)
            {
                Serial.println("Setting Home Position");
                drone_ideu.set_home_lat_lon(gps.location.lat(), gps.location.lng(), (float)gps.altitude.meters());
                drone_ideu.set_home_lat_lon(gps.location.lat(), gps.location.lng(), (float)gps.altitude.meters());
                drone_ideu.set_time(gps.time.second(), gps.time.minute(), gps.time.hour(), gps.date.day(), gps.date.month(), gps.date.year());
                is_home_set = true;
            }
            else
                return;
        }

        // Set gps data and send beacon
        drone_ideu.set_lat_lon(gps.location.lat(), gps.location.lng());
        drone_ideu.set_heigth((float)gps.altitude.meters());
        drone_ideu.set_heading((int)gps.course.deg());
        drone_ideu.set_ground_speed((int)gps.speed.knots());
        drone_ideu.set_num_satellites(gps.satellites.value());

        if (drone_ideu.time_to_send())
        {
            Serial.println("Send beacon");

            drone_ideu.send_beacon_frame();
            drone_ideu.set_last_send();
        }
    }
}

void program2()
{
    // get message
    if (SerialBT.available())
    {
        incomingChar = SerialBT.read();
        message += String(incomingChar);

        if (incomingChar != '\n')
        {
            return;
        }

        if (message.charAt(0) == 'g')
        {
            Serial.println("send BL data!");
            char *my_s_bytes = reinterpret_cast<char *>(&drone);
            u_int8_t *my_s_bytes2 = (u_int8_t*)malloc(sizeof(drone)+1);

            // set size of packet
            u_int8_t size = 72;
            memcpy(my_s_bytes2, &size, sizeof(u_int8_t));
            memcpy(&my_s_bytes2[1], my_s_bytes, sizeof(drone));
        
            delay(100);
            Serial.println(sizeof(drone));
            for(int i=0;i<73;i++){
                SerialBT.write(my_s_bytes2[i]);
            }
            message = "";
        }
        if (message.charAt(0) == 'p')
        {
            Serial.println(message.c_str());
            uint8_t *rxdata;
            char *rxchars = strdup(message.c_str());
            memcpy(rxdata, message.c_str(), 72);
            Serial.println(rxchars);

            int index = 1;
            snprintf(drone.UAS_operator, 24, "%24s", &rxchars[index]); index += 24;
            snprintf(drone.UAV_id, 24, "%24s", &rxchars[index]); index += 24;
            
            memcpy(&rxdata[index], &drone.UA_type, 4); index += 4;
            memcpy(&rxdata[index], &drone.min_sattelites, 4); index += 4;
            memcpy(&rxdata[index], &drone.min_accuracy, 4); index += 4;
            memcpy(&rxdata[index], &drone.EU_category, 4); index += 4;
            memcpy(&rxdata[index], &drone.EU_class, 4); index += 4;
            EEPROM.begin(4+68);
            EEPROM.writeInt(0, MAGIC_NUMBER);
            EEPROM.put(4, drone);
            EEPROM.commit();
            message = "";
            ESP.restart();
        }
    }
}


char * char_repeat(const char* str, int numWhitespace) {
    int len = strlen(str);
    numWhitespace++;
    // Allocate memory for the new string including whitespace
    char* newStr = (char*)malloc(len + numWhitespace);
    
    for (int i = 0; i < numWhitespace; i++) {
        newStr[i] = ' ';
    }
    // Copy the original string to the new string after the whitespace
    strcpy(newStr + numWhitespace-1, str);
    return newStr;
}

void setupData()
{
    EEPROM.begin(4+68);
    const uint32_t magicInt = EEPROM.readInt(0);

    // if data exists
    if(magicInt == MAGIC_NUMBER){
        Serial.println("Data found in EEPROM!");
        EEPROM.get(4, drone);
        drone_ideu.setup(drone.UAS_operator, drone.UAV_id, drone.UA_type, drone.EU_category, drone.EU_class);
        return;
    }
    // else put random data in there
    Serial.println("No Data found in EEPROM!");
    String s = char_repeat(DEFAULT_UAS_OPERATOR, 24-strlen(DEFAULT_UAS_OPERATOR));
    s.concat(DEFAULT_UAS_OPERATOR);
    strncpy(drone.UAS_operator, s.c_str(), 24);
    String s2 = char_repeat(DEFAULT_UAS_ID, 24-strlen(DEFAULT_UAS_ID));
    s2.concat(DEFAULT_UAS_ID);
    strncpy(drone.UAV_id, s2.c_str(), 24);
    drone.UA_type = DEFAULT_UA_TYPE;
    drone.min_sattelites = DEFAULT_MIN_SATELLITES;
    drone.min_accuracy = DEFAULTMIN_ACCURACY;
    drone.EU_category = DEFAULT_EU_CATEGORY;
    drone.EU_class = DEFAULT_EU_CLASS;
    drone_ideu.setup(drone.UAS_operator, drone.UAV_id, drone.UA_type, drone.EU_category, drone.EU_class);

    EEPROM.writeInt(0, MAGIC_NUMBER);
    EEPROM.put(4, drone);
    EEPROM.commit();
}

void setupBle()
{
    Serial.println("Starting Bluetooth...");
    String s = "RemoteID-" + String(ESP.getEfuseMac());
    SerialBT.begin(s); // Bluetooth device name
    Serial.println("The device started, now you can pair it with bluetooth!");
}


void displayInfo()
{
    Serial.print(F("Location: "));
    if (gps.location.isValid())
    {
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 6);
    }
    else
    {
        Serial.print(F("INVALID"));
    }

    Serial.print(F("  Date/Time: "));
    if (gps.date.isValid())
    {
        Serial.print(gps.date.month());
        Serial.print(F("/"));
        Serial.print(gps.date.day());
        Serial.print(F("/"));
        Serial.print(gps.date.year());
    }
    else
    {
        Serial.print(F("INVALID"));
    }

    Serial.print(F(" "));
    if (gps.time.isValid())
    {
        if (gps.time.hour() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        if (gps.time.minute() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        if (gps.time.second() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.second());
        Serial.print(F("."));
        if (gps.time.centisecond() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.centisecond());
    }
    else
    {
        Serial.print(F("INVALID"));
    }
    Serial.print("\t");
    Serial.print(gps.satellites.value());

    Serial.print("\t");
    Serial.print(gps.hdop.hdop());

    Serial.println();
}

/************************************
 *      BUTTON
 * *********************************/
void button_callback(Button2 &b)
{
    for (int i = 0; i < ARRARY_SIZE(g_btns); ++i)
    {
        if (pBtns[i] == b || true)
        {
            if(program){
                program = 0;
                ESP.restart();
            } else{
                setupBle();
                program = 1;
            }
        }
    }
}

void button_loop()
{
    for (int i = 0; i < ARRARY_SIZE(g_btns); ++i)
    {
        pBtns[i].loop();
    }
}

void button_init()
{
    uint8_t args = ARRARY_SIZE(g_btns);
    pBtns = new Button2[args];
    for (int i = 0; i < args; ++i)
    {
        pBtns[i] = Button2(g_btns[i]);
        pBtns[i].setPressedHandler(button_callback);
    }
    
    btnTick.attach_ms(20, button_loop);
        
}