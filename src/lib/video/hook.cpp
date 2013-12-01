#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "video.hpp"

extern "C" {

typedef void (*kruto_glXSwapBuffers)(Display *dpy, GLXDrawable drawable);
static kruto_glXSwapBuffers __kruto_glXSwapBuffers = reinterpret_cast<kruto_glXSwapBuffers>(dlsym(RTLD_NEXT, "glXSwapBuffers"));

void glXSwapBuffers(Display *dpy, GLXDrawable drawable) 
{
	krutoc::Video::instance()->update();
	__kruto_glXSwapBuffers(dpy, drawable);
}

}
