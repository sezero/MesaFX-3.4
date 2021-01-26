/* $Id: glxheader.h,v 1.1.4.1 2001/02/09 08:25:18 joukj Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 * 
 * Copyright (C) 1999  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef GLX_HEADER_H
#define GLX_HEADER_H

#ifdef VMS
#include <vms_x_fix.h>
#endif


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif


#include "glheader.h"


#ifdef XFree86Server

# include "resource.h"
# include "windowstr.h"
# include "gcstruct.h"
# include "GL/xf86glx.h"
# include "xf86glx_util.h"

#else

# ifdef GLX_DIRECT_RENDERING
#  include "dri_mesaint.h"
# endif
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# ifdef USE_XSHM  /* was SHM */
#  include <sys/ipc.h>
#  include <sys/shm.h>
#  include <X11/extensions/XShm.h>
# endif

#endif



/* this silences a compiler warning on several systems */
struct timespec;
struct itimerspec;



#endif /*GLX_HEADER*/
