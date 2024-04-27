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

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue

data class DroneInfo(val txt: String) {
    var UAS_operator by mutableStateOf("UAS_operator")
    var UAV_id by mutableStateOf("UAV_id")
    var ua_type by mutableStateOf(0)
    var EU_category by mutableStateOf(2)
    var EU_class by mutableStateOf(1)
    var min_sattelites by mutableStateOf(4)
    var min_accuracy by mutableStateOf(2.0.toFloat())

    fun setValue(key: String, value: Float) {
        when (key) {
            "ua_type" -> this.ua_type = value.toInt()
            "EU_category" -> this.EU_category = value.toInt()
            "EU_class" -> this.EU_class = value.toInt()
            "min_sattelites" -> this.min_sattelites = value.toInt()
            "min_accuracy" -> this.min_accuracy = value
        }
    }

    fun getValue(key: String): Any {
        when (key) {
            "ua_type" -> return this.ua_type.toFloat()
            "EU_category" -> return this.EU_category.toFloat()
            "EU_class" -> return this.EU_class.toFloat()
            "min_sattelites" -> return this.min_sattelites.toFloat()
            "min_accuracy" -> return this.min_accuracy
        }
        return -1
    }

    override fun toString(): String {
        return "$UAS_operator - $UAV_id - $ua_type - $EU_category - $EU_class - $min_sattelites - $min_accuracy"
    }
}