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

#include <TinyGPS++.h>
#include "boards.h"
#include <Ticker.h>
#include "droneID.h"
#include "BluetoothSerial.h"

#include <EEPROM.h>

extern "C" {
#include "Button2.h"
}

droneIDEU drone_ideu;
TinyGPSPlus gps;
BluetoothSerial SerialBT;
char ssid[14];

struct droneInfo
{
    char UAS_operator[24];
    char UAV_id[24];
    u_int32_t UA_type;
    u_int32_t min_sattelites;
    float min_accuracy;
    u_int32_t EU_category;
    u_int32_t EU_class;
};

droneInfo drone;
char incomingChar;
String message = "";


uint8_t program = 0;
bool print_debug = true;
const uint8_t min_sattelites = 4;
const float min_accuracy = 8.;

uint64_t dispMap = 0;
String dispInfo;
char buff[5][256];

uint64_t gpsSec = 0;
#define BUTTONS_MAP {BUTTON_PIN}

Ticker btnTick;
Button2 *pBtns = nullptr;
uint8_t g_btns[] =  BUTTONS_MAP;
#define ARRARY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

bool is_home_set = false;

void displayInfo();

void setupBle();

void setupData();

void printDrone();