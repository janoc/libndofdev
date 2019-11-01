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
 * Basic sanity test that the library works and can actually read device data.
 */

#include <stdio.h>
#include <unistd.h>

#include "ndofdev_external.h"

int main(int argc, char** argv)
{
    int i;
    NDOF_Device *dev = NULL;

    ndof_libinit(NULL, NULL, NULL);

    dev = ndof_create();
    if(ndof_init_first(dev, NULL) < 0)
    {
        perror("Unable to open any NDOF devices!");
        return(-1);
    }

    puts("Detected:");
    ndof_dump(stdout, dev);

    puts("\nPress RETURN to start polling ...");
    getchar();

    // try to poll the device and get data
    while(1)
    {
        ndof_update(dev);

        printf("\nAxes: ");
        for(i = 0; i < dev->axes_count; ++i)
            printf("%ld ", dev->axes[i]);

        printf("\nButtons: ");
        for(i = 0; i < dev->btn_count; ++i)
            printf("%ld ", dev->buttons[i]);

        usleep(100000);
    }
}
