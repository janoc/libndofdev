/*
*
* Copyright (c) 2008, Jan Ciger (jan.ciger (at) gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of its contributors may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Jan Ciger ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Jan Ciger BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
    * Drop-in replacement for the 3D Connexion SDK library not available on Linux
    *
    * Uses native Linux input (evdev) device for SpaceNavigator, otherwise SDL for
    * regular joysticks.
    *
    * Evdev could be used for joysticks as well, but higher level logic would have to be
    * re-implemented (calibration, filtering, etc.) - SDL includes it already.
    *
*/

#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL/SDL.h>

#include "ndofdev_external.h"

// Hack - we need to store the descriptor
// of the SpaceNavigator in order to be able to 
// turn its LED off in the ndof_cleanup() function :(
static int spacenav_fd = -1;

const int SPACE_NAVIG_THRESHOLD = 20; // minimum change threshold

typedef struct
{
    // native HID interface or SDL?
    int USE_SDL;

    // SpaceNavigator
    int fd;             // file descriptor of the device
    long int *axes;     // last state
    long int *buttons;

    // SDL joysticks
    SDL_Joystick *j;

} LinJoystickPrivate;

NDOF_Device *ndof_create()
{
    NDOF_Device *d = (NDOF_Device *) calloc(1, sizeof(NDOF_Device));
    return d;
}

int ndof_init_first(NDOF_Device *in_out_dev, void *param)
{
    // try to find 3DConnexion SpaceNavigator first
    int i = 0;
    int fd = -1;
    char *fname = NULL;
    struct input_id ID;

    fname = (char *)malloc(1000 * sizeof(char));
    while (i < 32)
    {
        sprintf(fname, "/dev/input/event%d", i++);
        fd = open(fname, O_RDWR | O_NONBLOCK);
        if (fd > 0)
        {
            ioctl(fd, EVIOCGID, &ID);        // get device ID
            if ((ID.vendor == 0x046d) &&
                    ((ID.product == 0xc626) || // SpaceNavigators
                     (ID.product == 0xc621) || // SpaceBall 5000 
                     (ID.product == 0xc627) || // SpaceExplorer (untested)
                     (ID.product == 0xc623) || // SpaceTraveler (untested)
                     (ID.product == 0xc603)))  // SpaceMouse (untested)
            {
                // printf("Using device: %s\n", fname);
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }

    if(fd > 0)
    {
        // We have SpaceNavigator, use it
        spacenav_fd = fd;

        int N_AXES = 6; // FIXME: shouldn't be hardwired!
        int N_BUTTONS = 12;

        in_out_dev->axes_count = N_AXES;
        in_out_dev->btn_count  = N_BUTTONS;
        in_out_dev->absolute   = 0;
        in_out_dev->valid      = 1;
        in_out_dev->axes_max   = 512;
        in_out_dev->axes_min   = -512;
        ioctl(fd, EVIOCGNAME(255), &in_out_dev->product); // copy name of the device

        // private data
        LinJoystickPrivate *priv = (LinJoystickPrivate *) malloc (sizeof(LinJoystickPrivate));
        priv->fd = fd;
        priv->axes = (long int *) calloc(N_AXES, sizeof(long int));
        priv->buttons = (long int *) calloc(N_BUTTONS, sizeof(long int));
        priv->USE_SDL = 0;
        priv->j = NULL;
        in_out_dev->private_data = priv;

        // turn the LED on
        struct input_event led_ev;

        led_ev.type = EV_LED;
        led_ev.code = LED_MISC;
        led_ev.value = 1;
        write(spacenav_fd, &led_ev, sizeof(struct input_event));

        return 0;

    } else {
        // SpaceNavigator not found, use SDL Joystick
        SDL_Joystick *j = SDL_JoystickOpen(0);
        if(j)
        {
            in_out_dev->axes_count = SDL_JoystickNumAxes(j);
            in_out_dev->btn_count = SDL_JoystickNumButtons(j);
            in_out_dev->absolute = 0; // always relative on Linux
            in_out_dev->valid = 1;
            in_out_dev->axes_max = 32767;
            in_out_dev->axes_min = -32767;
            strncpy(in_out_dev->product, SDL_JoystickName(0), 255);

            // private data
            LinJoystickPrivate *priv = (LinJoystickPrivate *) malloc (sizeof(LinJoystickPrivate));
            priv->j = j;
            priv->fd = -1;
            priv->USE_SDL = 1;
            in_out_dev->private_data = priv;

            return 0;
        } else {

            // no joysticks found
            return -1;
        }
    }
}

void ndof_libcleanup()
{
    if(spacenav_fd > 0)
    {
        // turn the LED off
        struct input_event led_ev;

        led_ev.type = EV_LED;
        led_ev.code = LED_MISC;
        led_ev.value = 0;
        write(spacenav_fd, &led_ev, sizeof(struct input_event));
    }

    // FIXME: needs to cleanup the memory
}

int ndof_libinit(NDOF_DeviceAddCallback in_add_cb,
                        NDOF_DeviceRemovalCallback in_removal_cb,
                        void *platform_specific)
{
    // Initialize the joystick subsystem
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    return 0;
}

void ndof_update(NDOF_Device *in_dev)
{
    int i;

    LinJoystickPrivate *priv = (LinJoystickPrivate *) in_dev->private_data;
    assert(priv != NULL);

    if(priv->USE_SDL)
    {
        SDL_JoystickUpdate();
        SDL_Joystick *j = priv->j;

        for(i = 0; i < in_dev->axes_count; i++)
        {
            in_dev->axes[i] = (int) (SDL_JoystickGetAxis(j, i));
        }

        for(i = 0; i < in_dev->btn_count; i++)
        {
            in_dev->buttons[i] = SDL_JoystickGetButton(j, i);
        }
    } else {
        // update SpaceNavigator

        struct input_event ev;

        while(read(priv->fd, &ev, sizeof(struct input_event)) > 0)
        {
            switch (ev.type)
            {
                case EV_KEY:
                    // printf("Key %d pressed %d.\n", ev.code, ev.value);
                    priv->buttons[ev.code & 0xff] = ev.value;
                    break;

                case EV_REL:
                case EV_ABS: // 2.6.35 and up, maybe earlier kernels too send EV_ABS instead of EV_REL
                    // printf("%d %g\n", ev.code, ev.value);

                    // clean up small values
                    priv->axes[ev.code] = abs(ev.value) > SPACE_NAVIG_THRESHOLD ? ev.value : 0;
                    break;

                default:
                    break;
            }
        }

        memcpy(in_dev->axes, priv->axes, in_dev->axes_count * sizeof(long int));
        memcpy(in_dev->buttons, priv->buttons, in_dev->btn_count * sizeof(long int));
    }
}

void ndof_dump(NDOF_Device *dev)
{

}

void ndof_dump_list()
{

}
