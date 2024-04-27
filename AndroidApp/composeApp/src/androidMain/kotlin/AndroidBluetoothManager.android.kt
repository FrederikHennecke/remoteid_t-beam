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

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothServerSocket
import android.bluetooth.BluetoothSocket
import android.content.Intent
import android.content.Intent.FLAG_ACTIVITY_NEW_TASK
import android.util.Log
import android.widget.Toast
import androidx.core.content.ContextCompat.getSystemService
import androidx.core.content.ContextCompat.startActivity
import java.io.IOException
import java.io.InputStream
import java.nio.ByteBuffer
import java.util.UUID


actual class AndroidBluetoothManager{

    private lateinit var bluetoothManager: BluetoothManager
    private lateinit var bluetoothAdapter: BluetoothAdapter
    private lateinit var bluetoothSocket: BluetoothSocket
    private lateinit var drone: DroneInfo
    var m_myUUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")



    actual fun initialize(drone: DroneInfo) {
        this.drone = drone
        bluetoothManager = getSystemService(applicationContext, BluetoothManager::class.java) as BluetoothManager
        bluetoothAdapter = bluetoothManager.adapter
        if (bluetoothAdapter == null) {
            Toast.makeText(applicationContext, "Error connecting to bluetooth", Toast.LENGTH_LONG).show()
        }

        if (!bluetoothAdapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            enableBtIntent.addFlags(FLAG_ACTIVITY_NEW_TASK)
            startActivity(applicationContext, enableBtIntent, null)
        }
        BluetoothServerController().start()
    }

    @SuppressLint("MissingPermission")
    actual fun getDevices() : List<String> {
        val pairedDevices: Set<BluetoothDevice>? = bluetoothAdapter.bondedDevices
        val listOfDevices: MutableList<String> = ArrayList<String>()
        pairedDevices?.forEach { device ->
            if(device.name.startsWith("RemoteID-")){
                listOfDevices.add(device.name)
            }
        }
        return listOfDevices.toList()
    }

    @SuppressLint("MissingPermission")
    actual fun connectDevice(deviceName: String){
        val pairedDevices: Set<BluetoothDevice>? = bluetoothAdapter.bondedDevices
        pairedDevices?.forEach { device ->
            if(device.name.equals(deviceName)){
                bluetoothSocket = device.createInsecureRfcommSocketToServiceRecord(m_myUUID)
                bluetoothSocket.connect()
                println("connected to $deviceName")
            }
        }
    }

    actual suspend fun receiveData(deviceName: String){
        connectDevice(deviceName)
        bluetoothSocket.outputStream.write("g\n".toByteArray())
        //delay(500)
        val mmInStream: InputStream = bluetoothSocket.inputStream

        val sizeOFPacket: Int = mmInStream.read()
        println(sizeOFPacket)
        val arr : MutableList<Byte> = mutableListOf()
        for(i in 1 .. sizeOFPacket){
            val dataToInt: Int = mmInStream.read() // why tf cant I input mmBuffer???
            arr.add(dataToInt.toByte())
            println(i)
            if(dataToInt == -1 || mmInStream.available()==0) {
                break
            }
        }
        if(arr.size<76)
            throw java.lang.Exception("not enough input found!")
        if(arr.size>76)
            throw java.lang.Exception("too much input found!")

        println("finished reading")
        decodeBytes(arr.toByteArray())
        bluetoothSocket.close()
    }




    private fun decodeBytes(bytes: ByteArray) {
        if (bytes.size!=72) {
            println("error, could not decode byte array")
            return
        }
        val str = bytes.decodeToString()

        drone.UAS_operator = str.substring(0, 24)
        drone.UAV_id = str.substring(24, 48)

        // reverse the byte order each time (both devices should be littleEndian but the encode and
        // decode functions work different on android and esp32)
        drone.ua_type = ByteBuffer.wrap(bytes.sliceArray(48.. 51).reversed().toByteArray()).getInt()
        drone.min_sattelites = ByteBuffer.wrap(bytes.sliceArray(52.. 55).reversed().toByteArray()).getInt()
        drone.min_accuracy = ByteBuffer.wrap(bytes.sliceArray(56.. 59).reversed().toByteArray()).getFloat()
        drone.EU_category = ByteBuffer.wrap(bytes.sliceArray(60.. 63).reversed().toByteArray()).getInt()
        drone.EU_class = ByteBuffer.wrap(bytes.sliceArray(64.. 67).reversed().toByteArray()).getInt()

        println(drone)
    }


    actual suspend fun transferData(deviceName: String) {
        connectDevice(deviceName)
        println("transferData")

        var bytes = ByteArray(68) // 24*2+5*4
        val buffer = ByteBuffer.wrap(bytes)
        putCharArray(buffer, drone.UAS_operator)
        putCharArray(buffer, drone.UAV_id)
        buffer.putInt(drone.ua_type)
        buffer.putInt(drone.min_sattelites)
        buffer.putFloat(drone.min_accuracy)
        buffer.putInt(drone.EU_category)
        buffer.putInt(drone.EU_class)
        bytes = "p".toByteArray() + buffer.array() + "\n".toByteArray()
        println(bytes.decodeToString())

        bluetoothSocket.outputStream.write(bytes)
        bluetoothSocket.close()
    }

    private fun putCharArray(buffer: ByteBuffer, charArray: String){
        if(charArray.length<24){
            val s: String = " ".repeat(24 - charArray.length)
            buffer.put(s.toByteArray())
        }
        for(c:Char in charArray){
            buffer.put(c.code.toByte());
        }
    }

    actual fun reset() {
        bluetoothSocket.inputStream.close()
        bluetoothSocket.outputStream.close()
        bluetoothSocket.close()
        BluetoothServerController().cancel()

        initialize(drone)
    }

}

// https://github.com/ikolomiyets/bluetooth-test/tree/master
@SuppressLint("MissingPermission")
class BluetoothServerController : Thread() {
    private var cancelled: Boolean
    private val serverSocket: BluetoothServerSocket?

    init {
        val bluetoothManager = getSystemService(applicationContext, BluetoothManager::class.java) as BluetoothManager
        val btAdapter = bluetoothManager.adapter
        if (btAdapter != null) {
            this.serverSocket = btAdapter.listenUsingRfcommWithServiceRecord("test", UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"))
            this.cancelled = false
        } else {
            this.serverSocket = null
            this.cancelled = true
        }

    }

    override fun run() {
        var socket: BluetoothSocket

        while(true) {
            if (this.cancelled) {
                println("cancelled")
                break
            }

            try {
                socket = serverSocket!!.accept()
            } catch(e: IOException) {
                println(e.printStackTrace())
                break
            }

            if (!this.cancelled && socket != null) {
                Log.i("server", "Connecting")
                BluetoothServer(socket).destroy()
                BluetoothServer(socket).start()
            }
        }
    }

    fun cancel() {
        this.cancelled = true
        this.serverSocket!!.close()
    }
}

class BluetoothServer(private val socket: BluetoothSocket): Thread() {
    private val inputStream = this.socket.inputStream
    private val outputStream = this.socket.outputStream

    override fun run() {
        try {
            println("Reading Bluetooth")
            val available = inputStream.available()
            val bytes = ByteArray(available)
            Log.i("server", "Reading")
            inputStream.read(bytes, 0, available)
            val text = String(bytes)
            Log.i("server", "Message received")
            Log.i("server", text)
        } catch (e: Exception) {
            Log.e("client", "Cannot read data", e)
        } finally {
            inputStream.close()
            outputStream.close()
            socket.close()
        }
    }
}