/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * Based on: blaze sensors lib
 */

#define LOG_TAG "Sensors"

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <cutils/log.h>
#include <cutils/native_handle.h>

/*****************************************************************************/

#define MAX_NUM_SENSORS 1

#define SUPPORTED_SENSORS  ((1<<MAX_NUM_SENSORS)-1)

#define ID_A  (0)
#define ID_O  (1)

#define SENSORS_ACCELERATION   (1<<ID_A)
#define SENSORS_ORIENTATION    (1<<ID_O)

#define MAX_NUM_DRIVERS 1

#define BASE_LOCATION   0
#define ENABLE_LOCATION  1
#define DELAY_LOCATION   2
#define MAX_LOCATIONS   DELAY_LOCATION + 1

#define ID_CMA  (0)
#define ID_SFH  (1)
#define ID_BMP  (2)
#define ID_HMC  (3)

struct driver_t {
    char *name;		/* name reported to input module */
    char *loc;		/* driver sys location */
    char *enable;	/* driver enable location */
    char *delay;	/* driver delay location */
    uint32_t mask;
};

static struct driver_t dDriverList[] = {
    {"kxtf9_accel",
         "/sys/bus/i2c/drivers/kxtf9/1-000f/",
         "/sys/bus/i2c/drivers/kxtf9/1-000f/enable",
         "/sys/bus/i2c/drivers/kxtf9/1-000f/delay",
	 (SENSORS_ACCELERATION) },
//         (SENSORS_ACCELERATION | SENSORS_ORIENTATION) },
};

/*****************************************************************************/

struct sensors_control_context_t {
    struct sensors_control_device_t device;
    int dev_fd[MAX_NUM_DRIVERS][MAX_LOCATIONS];
    sensors_data_t filter_sensors[MAX_NUM_SENSORS];
    uint32_t active_sensors;
    uint32_t active_drivers;
    int uinput;
    pthread_t poll_thread;
};

struct sensors_data_context_t {
    struct sensors_data_device_t device;
    int events_fd;
    sensors_data_t sensors[MAX_NUM_SENSORS];
    uint32_t pendingSensors;
};

/*
 * The SENSORS Module
 */
static const struct sensor_t sSensorList[] = {

        { "KXTF9 3 axis accelerometer",
                "kxtf9_accel",
                1, SENSORS_HANDLE_BASE+ID_A,
                SENSOR_TYPE_ACCELEROMETER, 4.0f*9.81f, 9.81f/1000.0f, 0.25f, { } },
/*	// KXTF9 TILT
	{ "KXTF9 Orientation sensor",
		"kxtf9_orientation",
		1, SENSORS_HANDLE_BASE+ID_O,
		SENSOR_TYPE_ORIENTATION, 360.0f, 1.0f, 9.7f, { } },
*/
};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static uint32_t sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    *list = sSensorList;
    return sizeof(sSensorList)/sizeof(sSensorList[0]);
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

const struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "nook color Sensors Module",
        .author = "",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};

/*****************************************************************************/

// sensor IDs must be a power of two and
// must match values in SensorManager.java
#define EVENT_TYPE_ACCEL_X          ABS_Y
#define EVENT_TYPE_ACCEL_Y          ABS_X
#define EVENT_TYPE_ACCEL_Z          ABS_Z

// 1000 LSG = 1G
#define LSG                         (1000.0f)

// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A                   (GRAVITY_EARTH / LSG)
#define CONVERT_A_X                 (CONVERT_A)
#define CONVERT_A_Y                 (CONVERT_A)
#define CONVERT_A_Z                 (CONVERT_A)


#define SENSOR_STATE_MASK           (0x7FFF)

/*****************************************************************************/

static int open_input(char *dev_name, int mode)
{
    /* scan all input drivers and look for "dev_name" */
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, mode);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, dev_name)) {
                LOGD("using %s (name=%s)", dev_name, name);
                break;
            }
            close(fd);
            fd = -1;
        }
    }
    closedir(dir);

    if (fd < 0) {
        LOGE("Couldn't find or open '%s' driver (%s)", dev_name, strerror(errno));
    }
    return fd;
}


static int open_dev(struct sensors_control_context_t *dev, int dev_num, int type)
{
    LOGD("Opening device number %i, and type %i", dev_num, type);
    if (dev->dev_fd[dev_num][type] < 0) {
        if (type == BASE_LOCATION) {
	        dev->dev_fd[dev_num][type] = open(dDriverList[dev_num].loc, O_RDWR);
                LOGE_IF(dev->dev_fd[dev_num][type] < 0, "Couldn't open %s (%s)",
                   dDriverList[dev_num].loc, strerror(errno));
        } else if(type == ENABLE_LOCATION) {
               dev->dev_fd[dev_num][type] = open(dDriverList[dev_num].enable, O_RDWR);
                LOGE_IF(dev->dev_fd[dev_num][type] < 0, "Couldn't open %s (%s)",
                   dDriverList[dev_num].enable, strerror(errno));
        } else if(type == DELAY_LOCATION) {
               dev->dev_fd[dev_num][type] = open(dDriverList[dev_num].delay, O_RDWR);
               LOGE_IF(dev->dev_fd[dev_num][type] < 0, "Couldn't open %s (%s)",
                dDriverList[dev_num].delay, strerror(errno));
        }

    }
    return dev->dev_fd[dev_num][type];
}

static void close_dev(struct sensors_control_context_t *dev, int dev_num, uint32_t enabled, int type)
{
    if ((dev->dev_fd[dev_num][type] >= 0) && ((enabled & dDriverList[dev_num].mask) == 0)) {
        close(dev->dev_fd[dev_num][type]);
        dev->dev_fd[dev_num][type] = -1;
    }
}

static int send_event(int fd, uint16_t type, uint16_t code, int32_t value)
{
    struct input_event event;

    memset(&event, 0, sizeof(event));

    event.type      = type;
    event.code      = code;
    event.value     = value;

    return write(fd, &event, sizeof(event));
}

static int uinput_create(char *name)
{
    struct uinput_user_dev udev;
    int i = 0;

    int fd = open_input(name, O_RDWR);
    if (fd >= 0) {
        return fd;
    }
    int uinputfd = open("/dev/uinput", O_RDWR);
    if(uinputfd < 0) {
        LOGE("Can't open uinput device (%s)", strerror(errno));
        return -errno;
    }

    memset(&udev, 0, sizeof(udev));
    strncpy(udev.name, name, UINPUT_MAX_NAME_SIZE);

    ioctl(uinputfd, UI_SET_EVBIT, EV_SYN);
    ioctl(uinputfd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinputfd, UI_SET_EVBIT, EV_REL);

    ioctl(uinputfd, UI_SET_ABSBIT, EVENT_TYPE_ACCEL_X);
    ioctl(uinputfd, UI_SET_ABSBIT, EVENT_TYPE_ACCEL_Y);
    ioctl(uinputfd, UI_SET_ABSBIT, EVENT_TYPE_ACCEL_Z);

    /* no need to filter since drivers already do */
    for (i = 0; i < ABS_MAX; i++) {
        udev.absmax[i] = 8000;
        udev.absmin[i] = -8000;
    }

    if (write(uinputfd, &udev, sizeof(udev)) < 0) {
        LOGE("Can't write uinput device information (%s)", strerror(errno));
        close(uinputfd);
        return -errno;
    }

    if (ioctl(uinputfd, UI_DEV_CREATE))
    {
        LOGE("Can't create uinput device (%s)", strerror(errno));
        close(uinputfd);
        return -errno;
    }

    return uinputfd;
}

static void *poll_thread(void *arg)
{
    struct sensors_control_context_t *dev= (struct sensors_control_context_t *)arg;
    struct pollfd event_fd[MAX_NUM_DRIVERS];

    int err = 0;
    int i = 0;
    int j = 0;

    for (i = 0; i < MAX_NUM_DRIVERS; i++) {
        int fd = open_input(dDriverList[i].name, O_RDONLY);
	 LOGD("Opening the input %s\n", dDriverList[i].name);
        if (fd < 0) {
            LOGE("invalid file descriptor, fd=%d", fd);
        }

        event_fd[i].fd = fd;
        event_fd[i].events = POLLIN;
    }

    uint32_t new_sensors = 0;
    LOGD("starting poll loop for %d drivers", MAX_NUM_DRIVERS);
    while(1) {
	//LOGD("sensor poll");
        int pollres = poll(event_fd, MAX_NUM_DRIVERS, -1);
        if (pollres <= 0) {
            if (errno != EINTR) {
                LOGW("select failed (errno=%d)\n", errno);
                usleep(100000);
            }
            continue;
        }
        for (i = 0; i < MAX_NUM_DRIVERS; i++) {
            if (event_fd[i].revents) {
                if (event_fd[i].revents & POLLIN) {
                    struct input_event event;
                    int nread = read(event_fd[i].fd, &event, sizeof(event));
                    if (nread == sizeof(event)) {
                        uint32_t active_sensors = dev->active_sensors;
                        uint32_t write_event = 0;
	    		//LOGD("event - sensor %d, val %f, type %d, code %d, active sensors %d", i, event.value, event.type, event.code,  active_sensors);
                        if (dev->uinput >= 0) {
                            if (event.type == EV_REL) {
                                switch (event.code) {
                                    case EVENT_TYPE_ACCEL_X:
					// LOGD("EVENT_TYPE_ACCEL_X");
                                        new_sensors |= SENSORS_ACCELERATION;
                                        dev->filter_sensors[ID_A].acceleration.x = event.value;
                                        break;
                                    case EVENT_TYPE_ACCEL_Y:
					// LOGD("EVENT_TYPE_ACCEL_Y");
                                        new_sensors |= SENSORS_ACCELERATION;
                                        dev->filter_sensors[ID_A].acceleration.y = event.value;
                                        break;
                                    case EVENT_TYPE_ACCEL_Z:
					// LOGD("EVENT_TYPE_ACCEL_Z");
                                        new_sensors |= SENSORS_ACCELERATION;
                                        dev->filter_sensors[ID_A].acceleration.z = event.value;
                                        break;
                                }
                            } else if (event.type == EV_SYN) {
                                if (event.code == SYN_CONFIG) {
                                    // we use SYN_CONFIG to signal that we need to exit the
                                    // main loop.
                                    LOGD("got empty message: value=%d", event.value);
                                    if (event.value == 0) {
                                        if(!write(dev->uinput, &event, sizeof(event)))
                                            LOGE("%s: failed to write event (%s)", __PRETTY_FUNCTION__,
                                                    strerror(errno));
                                    }
                                }
                                while(new_sensors) {
                                    uint32_t i = 31 - __builtin_clz(new_sensors);
                                    new_sensors &= ~(1<<i);
                                    switch (1 << i) {
                                        case SENSORS_ACCELERATION:
                                            if(active_sensors & SENSORS_ACCELERATION) {
                                                send_event(dev->uinput, EV_ABS, EVENT_TYPE_ACCEL_X,
                                                    dev->filter_sensors[ID_A].acceleration.x);
                                                send_event(dev->uinput, EV_ABS, EVENT_TYPE_ACCEL_Y,
                                                    dev->filter_sensors[ID_A].acceleration.y);
                                                send_event(dev->uinput, EV_ABS, EVENT_TYPE_ACCEL_Z,
                                                    dev->filter_sensors[ID_A].acceleration.z);
                                                send_event(dev->uinput, EV_SYN, 0, 0);
                                            }
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ((void*)0);
}


/*****************************************************************************/

static native_handle_t* control__open_data_source(struct sensors_control_context_t *dev)
{
    native_handle_t* handle;

    if (dev->uinput < 0) {
        int ufd = uinput_create("sensors");
        if (ufd < 0) {
            LOGE("%s: uinput_create failed to create sensors", __PRETTY_FUNCTION__);
            return NULL;
        }
        dev->uinput = ufd;
    }
    /* need to spawn a thread to listen and handle data */
    if (dev->poll_thread == 0) {
	LOGD("Spawning poll thread");
        pthread_create(&dev->poll_thread, NULL, poll_thread, (void*)dev);
    }

    int fd = open_input("sensors", O_RDONLY);
    if (fd < 0) {
        LOGE("%s: open_input failed to find sensors", __PRETTY_FUNCTION__);
        return NULL;
    }

    handle = native_handle_create(1, 0);
    handle->data[0] = fd;
    return handle;
}

static int control__close_data_source(struct sensors_control_context_t *dev)
{
    return 0;
}

static int control__activate(struct sensors_control_context_t *dev,
        int handle, int enabled)
{
    unsigned int flags;
    int err = 0;

    if ((handle<SENSORS_HANDLE_BASE) ||
            (handle>=SENSORS_HANDLE_BASE+MAX_NUM_SENSORS)) {
	    LOGD("Max num of sensors is: %d and handle is %d",MAX_NUM_SENSORS, handle);
        return -1;
    }

    uint32_t handle_mask = (1<<handle);
    uint32_t enabled_mask = enabled ? handle_mask : 0;

    uint32_t current_active = dev->active_sensors;
    uint32_t new_active = (current_active & ~handle_mask) | (enabled_mask & handle_mask);

    uint32_t current_enabled = dev->active_drivers;
    uint32_t new_enabled = new_active;

    uint32_t changed_enabled = current_enabled ^ new_enabled;

    if (changed_enabled) {
        if (changed_enabled & SENSORS_ACCELERATION) {
	    LOGD("Enabling accelerometer");
            int fd = open_dev(dev, ID_CMA, ENABLE_LOCATION);
            if (fd >= 0) {
                flags = (new_enabled & SENSORS_ACCELERATION) ? 1 : 0;
	        char buffer[20];
	        int bytes = sprintf(buffer, "%d\n", flags);
                LOGD("KXTF9 SET ENABLE: flag = %d", flags);
                if (write(fd, buffer, bytes) < 0) {
                    LOGE("KXTF9 SET ENABLE failed(%s)", strerror(errno));
                    err = -errno;
                }
                close_dev(dev, ID_CMA, new_enabled, ENABLE_LOCATION);
            } else {
                LOGE("ID_CMA open error");
                err = fd;
            }
	err = 0;
        }

        if (err < 0)
            return err;

    }

    dev->active_sensors = current_active = new_active;
    dev->active_drivers = current_enabled = new_enabled;

    return 0;
}

static int control__set_delay(struct sensors_control_context_t *dev, int32_t ms)
{
    int delay = ms;
    short sdelay = ms;
    int err = 0;
    int fd = 0;

     if (dev->active_sensors & SENSORS_ACCELERATION) {
	     fd = open_dev(dev, ID_CMA, DELAY_LOCATION);
	     if (fd >= 0) {
		  char buffer[20];
		  int bytes = sprintf(buffer, "%d\n", ms);
		  LOGD("KXTF9 set delay is: delay = %d", ms);
		  if (write(fd, buffer, bytes) < 0) {
		      LOGE("KXTF9 set delay failed (%s)", strerror(errno));
		      err = -errno;
		  }
		  close_dev(dev, ID_CMA, SENSORS_ACCELERATION, DELAY_LOCATION);
	    } else {
		  LOGE("ID_CMA open set delay open error");
		  err = fd;
	    }
    }

    return err;
}

static int control__wake(struct sensors_control_context_t *dev)
{
    int err = 0;
    int fd = open_input(dDriverList[0].name, O_WRONLY);
    if (fd >= 0) {
        err = send_event(fd, EV_SYN, SYN_CONFIG, 0);
        LOGD_IF(err < 0, "control__wake, err=%d (%s)", errno, strerror(errno));
        close(fd);
    }
    return err;
}

/*****************************************************************************/

static int data__data_open(struct sensors_data_context_t *dev, native_handle_t* handle)
{
    int i;
    memset(&dev->sensors, 0, sizeof(dev->sensors));

    for (i=0 ; i < MAX_NUM_SENSORS ; i++) {
        dev->sensors[i].vector.status = SENSOR_STATUS_ACCURACY_HIGH;
    }

    dev->pendingSensors = 0;
    dev->events_fd = dup(handle->data[0]);

    native_handle_close(handle);
    native_handle_delete(handle);
    return 0;
}

static int data__data_close(struct sensors_data_context_t *dev)
{
    if (dev->events_fd >= 0) {
        close(dev->events_fd);
        dev->events_fd = -1;
    }
    return 0;
}

static int pick_sensor(struct sensors_data_context_t *dev,
        sensors_data_t* values)
{
    uint32_t mask = SUPPORTED_SENSORS;
    while (mask) {
        uint32_t i = 31 - __builtin_clz(mask);
        mask &= ~(1<<i);
        if (dev->pendingSensors & (1<<i)) {
            dev->pendingSensors &= ~(1<<i);
            *values = dev->sensors[i];
            values->sensor = (1<<i);
            LOGD_IF(0, "%d [%f, %f, %f]", (1<<i),
                    values->vector.x,
                    values->vector.y,
                    values->vector.z);
            return i;
        }
    }
    LOGE("No sensor to return!!! pendingSensors=%08x", dev->pendingSensors);
    // we may end-up in a busy loop, slow things down, just in case.
    usleep(100000);
    return -1;
}

static int data__poll(struct sensors_data_context_t *dev, sensors_data_t* values)
{
    int fd = dev->events_fd;
    if (fd < 0) {
        LOGE("data__poll:Invalid file descriptor, fd=%d", fd);
        return -1;
    }

    // there are pending sensors, returns them now...
    if (dev->pendingSensors) {
	LOGE("Pending sensors");
        return pick_sensor(dev, values);
    }
    // wait until we get a complete event for an enabled sensor
    uint32_t new_sensors = 0;
    while (1) {
        /* read the next event */
        struct input_event event;
        uint32_t v;
        int nread = read(fd, &event, sizeof(event));
        if (nread != sizeof(event)) {
            LOGE("Incorrect event size size=%i", nread);
            return -1;
        }
           // LOGD("type: %d code: %d value: %-5d time: %ds",
                // event.type, event.code, event.value,
                // (int)event.time.tv_sec);
        if (event.type == EV_ABS) {
            switch (event.code) {
                case EVENT_TYPE_ACCEL_X:
                    new_sensors |= SENSORS_ACCELERATION;
                    dev->sensors[ID_A].acceleration.y = event.value * CONVERT_A_Y;
                    break;
                case EVENT_TYPE_ACCEL_Y:
                    new_sensors |= SENSORS_ACCELERATION;
                    dev->sensors[ID_A].acceleration.x = event.value * CONVERT_A_X;
                    break;
                case EVENT_TYPE_ACCEL_Z:
                    new_sensors |= SENSORS_ACCELERATION;
                    dev->sensors[ID_A].acceleration.z = event.value * CONVERT_A_Z;
                    break;
		}
        } else if (event.type == EV_SYN) {
            if (event.code == SYN_CONFIG) {
                // we use SYN_CONFIG to signal that we need to exit the
                // main loop.
                LOGD("got empty message in data poll: value=%d", event.value);
                return 0x7FFFFFFF;
            }
            if (new_sensors) {
                dev->pendingSensors = new_sensors;
                int64_t t = event.time.tv_sec*1000000000LL +
                        event.time.tv_usec*1000;
                while (new_sensors) {
                    uint32_t i = 31 - __builtin_clz(new_sensors);
                    new_sensors &= ~(1<<i);
                    dev->sensors[i].time = t;
                }
                return pick_sensor(dev, values);
            }
        }
    }
}

/*****************************************************************************/

static int control__close(struct hw_device_t *dev)
{
    int i = 0;
    int j = 0;
    struct sensors_control_context_t* ctx = (struct sensors_control_context_t*)dev;
    if (ctx) {
        control__close_data_source(ctx);
        for (i = 0; i < MAX_NUM_DRIVERS; i++) {
            for (j = 0; j < MAX_LOCATIONS; j++) {
                if (ctx->dev_fd[i][j] >= 0)
                    close(ctx->dev_fd[i][j]);
            }
        }
        free(ctx);
    }

    return 0;
}

static int data__close(struct hw_device_t *dev)
{
    int i = 0;
    struct sensors_data_context_t* ctx = (struct sensors_data_context_t*)dev;
    if (ctx) {
        if (ctx->events_fd >= 0) {
            //LOGD("(device close) about to close fd=%d", ctx->events_fd);
            close(ctx->events_fd);
        }
        free(ctx);
    }
    return 0;
}

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int i = 0;
    int j = 0;
    int status = -EINVAL;
    if (!strcmp(name, SENSORS_HARDWARE_CONTROL)) {
        struct sensors_control_context_t *dev;
        dev = malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
        for (i = 0; i < MAX_NUM_DRIVERS; i++) {
	        for (j = 0; j < MAX_LOCATIONS; j++)
	            dev->dev_fd[i][j] = -1;
	}
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = module;
        dev->device.common.close = control__close;
        dev->device.open_data_source = control__open_data_source;
        dev->device.close_data_source = control__close_data_source;
        dev->device.activate = control__activate;
        dev->device.set_delay = control__set_delay;
        dev->device.wake = control__wake;
        dev->active_sensors = 0;
        dev->active_drivers= 0;
        dev->uinput = -1;
       *device = &dev->device.common;
        status = 0;
    } else if (!strcmp(name, SENSORS_HARDWARE_DATA)) {
        struct sensors_data_context_t *dev;
        dev = malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
        dev->events_fd = -1;
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = module;
        dev->device.common.close = data__close;
        dev->device.data_open = data__data_open;
        dev->device.data_close = data__data_close;
        dev->device.poll = data__poll;
        *device = &dev->device.common;
        status = 0;
    }
    return status;
}
