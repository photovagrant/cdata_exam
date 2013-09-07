/* this driver comes from 
https://android.googlesource.com/device/samsung/tuna/+/android-4.3_r2.2/libsensors/
*/


static struct hw_module_methods_t sensors_module_methods = { //it's a class in C++
        open: open_sensors
};


struct sensors_module_t HAL_MODULE_INFO_SYM = {  //CLASS !!!
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

tatic int open_sensors(const struct hw_module_t* module, const char* id,
                        struct hw_device_t** device)
{
    FUNC_LOG;
    int status = -EINVAL;
    sensors_poll_context_t *dev = new sensors_poll_context_t();

    memset(&dev->device, 0, sizeof(sensors_poll_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = 0;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;

    *device = &dev->device.common;
    status = 0;

    return status;
}

