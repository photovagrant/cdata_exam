/* this driver comes from 
https://android.googlesource.com/device/samsung/tuna/+/android-4.3_r2.2/libsensors/
*/


static struct hw_module_methods_t sensors_module_methods = { //it's a class in C++
        open: open_sensors
};


struct sensors_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: SENSORS_HARDWARE_MODULE_ID,
                name: "Samsung Sensor module",
                author: "Samsung Electronic Company",
                methods: &sensors_module_methods, //this is a object
                dso: 0,
                reserved: {},
        },
        get_sensors_list: sensors__get_sensors_list,
};
