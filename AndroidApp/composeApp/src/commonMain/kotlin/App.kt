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

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.absoluteOffset
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material.AlertDialog
import androidx.compose.material.TextButton
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.RadioButton
import androidx.compose.material3.TextField
import androidx.compose.material3.TriStateCheckbox
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.state.ToggleableState
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.CoroutineExceptionHandler
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.CoroutineStart
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import theme.DarkColorScheme

lateinit var bluetoothManager: AndroidBluetoothManager
var blDevice: String = ""
var errorMessage = mutableStateOf("")

@Composable
fun App() {
    val drone by remember { mutableStateOf(DroneInfo("")) }
    bluetoothManager = AndroidBluetoothManager()
    bluetoothManager.initialize(drone)
    val topLevelScope = CoroutineScope(Job() + coroutineExceptionHandler)

    MaterialTheme(colorScheme = DarkColorScheme) {
        Column(
            verticalArrangement = Arrangement.spacedBy(2.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier
                .background(MaterialTheme.colorScheme.background)
                .fillMaxHeight()
                .padding(top = 16.dp)
        )
        {
            makeMenu(bluetoothManager)

            MinimalDialog()

            dataFields(drone)
            Row {
                Button(
                    onClick = { topLevelScope.launch(context=coroutineExceptionHandler, start=CoroutineStart.DEFAULT) {
                        if(blDevice != "")
                            bluetoothManager.transferData(blDevice)
                        else
                            errorMessage.value = "Error, no device selected!"
                    } })
                {
                    Text("Upload")
                }
                Button(
                    onClick = { topLevelScope.launch(context=coroutineExceptionHandler, start=CoroutineStart.DEFAULT) {
                        if(blDevice != "")
                            bluetoothManager.receiveData(blDevice)
                        else
                            errorMessage.value = "Error, no device selected!"
                    } })
                {
                    Text("Download")
                }
            }
        }
    }

}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun makeMenu(bluetoothManager: AndroidBluetoothManager, ){
    var expanded by remember { mutableStateOf(false) }
    val deviceList: List<String> = bluetoothManager.getDevices()
    var selectedText by remember { if(deviceList.isNotEmpty()) mutableStateOf(deviceList[0]) else  mutableStateOf("")}
    blDevice = selectedText
    println(deviceList)
    ExposedDropdownMenuBox (
        expanded = expanded,
        onExpandedChange = {expanded = !expanded} ,
        modifier = Modifier
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 20.dp, vertical = 20.dp)
            .absoluteOffset(x= (-20).dp)
    ) {
        TextField(
            value = selectedText,
            onValueChange = {},
            readOnly = true,
            trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expanded = expanded) },
            modifier = Modifier.menuAnchor()
        )
        ExposedDropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            for (device in deviceList) {
                DropdownMenuItem(
                    text = { Text(device) },
                    onClick = {
                        blDevice = device
                        selectedText = device
                        expanded = false
                    }
                )
            }
        }
    }
}

@Composable
fun MinimalDialog() {
    if(errorMessage.value != "") {
        AlertDialog(
            onDismissRequest = { errorMessage.value = "" },
            title = { Text(text = "Error") },
            text = { Text(text = errorMessage.value) },
            confirmButton = {
                TextButton(onClick = { errorMessage.value = "" }) {
                    Text(text = "OK")
                }
            })
    }
}

val coroutineExceptionHandler = CoroutineExceptionHandler { coroutineContext, exception ->
    bluetoothManager.reset()
    println("${exception.printStackTrace()}\n in CoroutineExceptionHandler")
    errorMessage.value = exception.message.toString()
}

@Composable
fun dataFields(drone: DroneInfo){
    Column (
        verticalArrangement = Arrangement.spacedBy(2.dp),
        horizontalAlignment = Alignment.Start,
        modifier = Modifier
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 20.dp)
    ) {
        OutlinedTextField(
            value = drone.UAS_operator,
            onValueChange = { drone.UAS_operator = it },
            label = { Text("UAS_operator") },
            maxLines = 1,
        )
        OutlinedTextField(
            value = drone.UAV_id,
            onValueChange = { drone.UAV_id = it },
            label = { Text("UAV_id") },
            maxLines = 1,
        )

        // UA_Type
        val lst_uatype : Array<String> = arrayOf("None", "Aeroplane", "Heli or Multi", "Gyroplane", "HybridLift", "Ornithopter", "Glider", "Kite", "FreeBalloon", "CaptiveBalloon", "Airship", "Parachute", "Rocket", "TetheredPowered", "GroundObstacle")
        ListOfCheckboxes("ua_type", lst_uatype, drone)

        // EU_category
        val lst_eucategory : Array<String> = arrayOf("Undeclared", "Open", "Specific", "Certified")
        ListOfCheckboxes("EU_category", lst_eucategory, drone)

        // EU_class
        val lst_euclass : Array<String> = arrayOf("0", "1", "2", "3", "4", "5", "6")
        ListOfCheckboxes("EU_class", lst_euclass, drone)

        // min_sattelites
        makeSlider("min_sattelites", drone, 8, 2f .. 10f)

        // min_accuracy
        makeSlider("min_accuracy", drone, 0, 0f .. 10f)
    }
}


@Composable
fun ListOfCheckboxes(typeOfInput: String, listOfStrings: Array<String>, drone: DroneInfo){
    val n = listOfStrings.size
    var checkedState by remember { mutableStateOf(false) }
    Row(verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier.padding(horizontal = 8.dp)
    ) {
        TriStateCheckbox(
            state = if (checkedState) ToggleableState.Indeterminate else ToggleableState.On,
            onClick = { checkedState = !checkedState },
        )
        Text(text=typeOfInput + ": " + listOfStrings[(drone.getValue(typeOfInput) as Float).toInt()], color = MaterialTheme.colorScheme.onSurfaceVariant)
    }
    if(checkedState) {
        Card (
            modifier = Modifier.padding(horizontal = 32.dp)
        ){
            Column(
                verticalArrangement = Arrangement.spacedBy(2.dp),
                horizontalAlignment = Alignment.Start,
                modifier = Modifier.padding(horizontal = 20.dp)
            ) {
                for(i in 0..<n){
                    Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(horizontal = 8.dp)) {
                        RadioButton(
                            selected = drone.getValue(typeOfInput) == i,
                            onClick = { drone.setValue(typeOfInput, i.toFloat()); drone.setValue(typeOfInput, i.toFloat()) }
                        )
                        Text(text=listOfStrings[i], color = MaterialTheme.colorScheme.onSurfaceVariant)
                    }
                }
            }
        }
    }
}

@Composable
fun makeSlider(typeOfInput: String, drone: DroneInfo, steps: Int, range:  ClosedFloatingPointRange<Float>){
    Slider(
        value = drone.getValue(typeOfInput) as Float,
        onValueChange = { drone.setValue(typeOfInput, it) },
        colors = SliderDefaults.colors(
            thumbColor = MaterialTheme.colorScheme.secondary,
            activeTrackColor = MaterialTheme.colorScheme.secondary,
            inactiveTrackColor = MaterialTheme.colorScheme.secondaryContainer,
        ),
        steps = steps,
        valueRange = range
    )
    Text(text = typeOfInput + ": " + drone.getValue(typeOfInput), color = MaterialTheme.colorScheme.onSurfaceVariant)
}