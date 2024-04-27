# RemoteID T-Beam

This project aims to provide an open-source implementation of Drone RemoteID for the T-Beam board. It consists of two main components: a C++ implementation for the T-Beam board and a Kotlin Android app for changing settings via Bluetooth.
## Features
- Drone RemoteID Implementation: Provides functionality to broadcast RemoteID information from the T-Beam board.
- Bluetooth Configuration: Allows users to configure settings of the T-Beam board using a Kotlin Android app via Bluetooth connection.

## Getting Started
### Prerequisites
- T-Beam board
- VSCode + PlatformIO plugin
- Android Studio for compiling and installing the Android app on your device

### Installation
1. Clone the repository.
2. Compile and upload the C++ code to the T-Beam board using Arduino IDE.
3. (Optional) Compile and install the Android app on your device using Android Studio.
A more in-depth explanation can be found in the sub-folders.

### Usage
- Power on the T-Beam board and ensure it's broadcasting RemoteID information.
- Launch the Android app on your device and establish a Bluetooth connection with the T-Beam board.
- Use the app to configure settings and interact with the T-Beam board.

## License

This project is licensed under the GNU General Public License.

## Acknowledgements
- utilities.h, boards.h from https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/tree/master
- opendroneid, odid_wifi, wifi from https://github.com/opendroneid/opendroneid-core-c
- id_open, utm from https://github.com/sxjack/uav_electronic_ids
- some inspiration from https://github.com/khancyr/droneID_FR/tree/master