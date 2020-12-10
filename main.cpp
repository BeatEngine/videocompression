#include <stdio.h>
#include <string>
#include <time.h>
#include "videoCompression.h"

#include <X11/Xlib.h> //-lX11
#include <X11/Xutil.h>
#include <X11/Xmd.h> 
#include <X11/Xatom.h>

#include <unistd.h>


#define FPS 60;
//lX11 -lXext -Ofast
int main(int args, char** argv)
{

    VideoCompressor vid;

    int x = 30; // 1/2 min
    int i = 0;
    clock_t start;
    int delay = 1/FPS;
    while(i < x)
    {
        start = clock();
        Display *dis=XOpenDisplay((char *)0);
        Screen *scr = XDefaultScreenOfDisplay(dis);
        Drawable drawable = XDefaultRootWindow(dis);
        if(i == 0)
        {
            vid = VideoCompressor(scr->width, scr->height);
        }
        XImage *image = XGetImage(dis, drawable, 0, 0, scr->width, scr->height, AllPlanes, ZPixmap);

        if(scr->width*scr->height > 0)
        {
            unsigned char* rgb = (unsigned char*)malloc(scr->width*scr->height*3);
            int c = 0;
            for(int i = 0; i < scr->width*scr->height*4; i+=4)
            {
                rgb[c] = image->data[i];
                rgb[c+1] = image->data[i+1];
                rgb[c+2] = image->data[i+2];
                c+=3;
            }
            vid.loadNextFrame(rgb, true);
            free(rgb);
        }
        XDestroyImage(image);
        XCloseDisplay(dis);
        i++;
        while ((clock()-start)/CLOCKS_PER_SEC < delay)
        {
            usleep(10);
        }
    }
    i = 0;
    vid.setVideoPosition(0);
    while (i < x)
    {
        vid.getNext(true);
        i++;
    }
    


    return 0;
}




