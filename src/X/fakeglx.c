/* $Id: fakeglx.c,v 1.34.4.9 2001/05/15 20:22:19 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4.2
 * 
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
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


/*
 * This is an emulation of the GLX API which allows Mesa/GLX-based programs
 * to run on X servers which do not have the real GLX extension.
 *
 * Thanks to the contributors:
 *
 * Initial version:  Philip Brown (phil@bolthole.com)
 * Better glXGetConfig() support: Armin Liebchen (liebchen@asylum.cs.utah.edu)
 * Further visual-handling refinements: Wolfram Gloger
 *    (wmglo@Dent.MED.Uni-Muenchen.DE).
 *
 * Notes:
 *   Don't be fooled, stereo isn't supported yet.
 */



#include "glxheader.h"
#include "glxapi.h"
#include "GL/xmesa.h"
#include "context.h"
#include "config.h"
#include "macros.h"
#include "mmath.h"
#include "types.h"
#include "xfonts.h"
#include "xmesaP.h"


/* This indicates the client-side GLX API and GLX encoder version. */
#define CLIENT_MAJOR_VERSION 1
#define CLIENT_MINOR_VERSION 2

/* This indicates the server-side GLX decoder version.
 * GLX 1.3 indicates OpenGL 1.2 support
 */
#define SERVER_MAJOR_VERSION 1
#define SERVER_MINOR_VERSION 3

/* This is appended onto the glXGetClient/ServerString version strings. */
#define MESA_GLX_VERSION "Mesa 3.4.2"

/* Who implemented this GLX? */
#define VENDOR "Brian Paul"



/* Silence compiler warnings */
extern void Fake_glXDummyFunc( void );
void Fake_glXDummyFunc( void )
{
   (void) kernel8;
   (void) DitherValues;
   (void) HPCR_DRGB;
   (void) kernel1;
}



#define DONT_CARE -1



#define MAX_VISUALS 100
static XMesaVisual VisualTable[MAX_VISUALS];
static int NumVisuals = 0;



/*
 * This struct and some code fragments borrowed
 * from Mark Kilgard's GLUT library.
 */
typedef struct _OverlayInfo {
  /* Avoid 64-bit portability problems by being careful to use
     longs due to the way XGetWindowProperty is specified. Note
     that these parameters are passed as CARD32s over X
     protocol. */
  unsigned long overlay_visual;
  long transparent_type;
  long value;
  long layer;
} OverlayInfo;



/* Macro to handle c_class vs class field name in XVisualInfo struct */
#if defined(__cplusplus) || defined(c_plusplus)
#define CLASS c_class
#else
#define CLASS class
#endif




/*
 * Test if the given XVisualInfo is usable for Mesa rendering.
 */
static GLboolean is_usable_visual( XVisualInfo *vinfo )
{
   switch (vinfo->CLASS) {
      case StaticGray:
      case GrayScale:
         /* Any StaticGray/GrayScale visual works in RGB or CI mode */
         return GL_TRUE;
      case StaticColor:
      case PseudoColor:
	 /* Any StaticColor/PseudoColor visual of at least 4 bits */
	 if (vinfo->depth>=4) {
	    return GL_TRUE;
	 }
	 else {
	    return GL_FALSE;
	 }
      case TrueColor:
      case DirectColor:
	 /* Any depth of TrueColor or DirectColor works in RGB mode */
	 return GL_TRUE;
      default:
	 /* This should never happen */
	 return GL_FALSE;
   }
}



/*
 * Return the level (overlay, normal, underlay) of a given XVisualInfo.
 * Input:  dpy - the X display
 *         vinfo - the XVisualInfo to test
 * Return:  level of the visual:
 *             0 = normal planes
 *            >0 = overlay planes
 *            <0 = underlay planes
 */
static int level_of_visual( Display *dpy, XVisualInfo *vinfo )
{
   Atom overlayVisualsAtom;
   OverlayInfo *overlay_info = NULL;
   int numOverlaysPerScreen;
   Status status;
   Atom actualType;
   int actualFormat;
   unsigned long sizeData, bytesLeft;
   int i;

   /*
    * The SERVER_OVERLAY_VISUALS property on the root window contains
    * a list of overlay visuals.  Get that list now.
    */
   overlayVisualsAtom = XInternAtom(dpy,"SERVER_OVERLAY_VISUALS", True);
   if (overlayVisualsAtom == None) {
      return 0;
   }

   status = XGetWindowProperty(dpy, RootWindow( dpy, vinfo->screen ),
                               overlayVisualsAtom, 0L, (long) 10000, False,
                               overlayVisualsAtom, &actualType, &actualFormat,
                               &sizeData, &bytesLeft,
                               (unsigned char **) &overlay_info );

   if (status != Success || actualType != overlayVisualsAtom ||
       actualFormat != 32 || sizeData < 4) {
      /* something went wrong */
      XFree((void *) overlay_info);
      return 0;
   }

   /* search the overlay visual list for the visual ID of interest */
   numOverlaysPerScreen = (int) (sizeData / 4);
   for (i=0;i<numOverlaysPerScreen;i++) {
      OverlayInfo *ov;
      ov = overlay_info + i;
      if (ov->overlay_visual==vinfo->visualid) {
         /* found the visual */
         if (/*ov->transparent_type==1 &&*/ ov->layer!=0) {
            int level = ov->layer;
            XFree((void *) overlay_info);
            return level;
         }
         else {
            XFree((void *) overlay_info);
            return 0;
         }
      }
   }

   /* The visual ID was not found in the overlay list. */
   XFree((void *) overlay_info);
   return 0;
}




/*
 * Given an XVisualInfo and RGB, Double, and Depth buffer flags, save the
 * configuration in our list of GLX visuals.
 */
static XMesaVisual
save_glx_visual( Display *dpy, XVisualInfo *vinfo,
                 GLboolean rgbFlag, GLboolean alphaFlag, GLboolean dbFlag,
                 GLboolean stereoFlag,
                 GLint depth_size, GLint stencil_size,
                 GLint accumRedSize, GLint accumGreenSize,
                 GLint accumBlueSize, GLint accumAlphaSize,
                 GLint level )
{
   GLboolean ximageFlag = GL_TRUE;
   XMesaVisual xmvis;
   GLint i;
   GLboolean comparePointers;

   if (dbFlag) {
      /* Check if the MESA_BACK_BUFFER env var is set */
      char *backbuffer = getenv("MESA_BACK_BUFFER");
      if (backbuffer) {
         if (backbuffer[0]=='p' || backbuffer[0]=='P') {
            ximageFlag = GL_FALSE;
         }
         else if (backbuffer[0]=='x' || backbuffer[0]=='X') {
            ximageFlag = GL_TRUE;
         }
         else {
            fprintf(stderr, "Mesa: invalid value for MESA_BACK_BUFFER ");
            fprintf(stderr, "environment variable, using an XImage.\n");
         }
      }
   }

   /* Comparing IDs uses less memory but sometimes fails. */
   /* XXX revisit this after 3.0 is finished. */
   if (getenv("MESA_GLX_VISUAL_HACK"))
      comparePointers = GL_TRUE;
   else
      comparePointers = GL_FALSE;

   /* First check if a matching visual is already in the list */
   for (i=0; i<NumVisuals; i++) {
      XMesaVisual v = VisualTable[i];
      if (v->display == dpy
          && v->level == level
          && v->ximage_flag == ximageFlag
          && v->gl_visual->RGBAflag == rgbFlag
          && v->gl_visual->DBflag == dbFlag
          && v->gl_visual->StereoFlag == stereoFlag
          && (v->gl_visual->AlphaBits > 0) == alphaFlag
          && (v->gl_visual->DepthBits >= depth_size || depth_size == 0)
          && (v->gl_visual->StencilBits >= stencil_size || stencil_size == 0)
          && (v->gl_visual->AccumRedBits >= accumRedSize || accumRedSize == 0)
          && (v->gl_visual->AccumGreenBits >= accumGreenSize || accumGreenSize == 0)
          && (v->gl_visual->AccumBlueBits >= accumBlueSize || accumBlueSize == 0)
          && (v->gl_visual->AccumAlphaBits >= accumAlphaSize || accumAlphaSize == 0)) {
         /* now either compare XVisualInfo pointers or visual IDs */
         if ((!comparePointers && v->visinfo->visualid == vinfo->visualid)
             || (comparePointers && v->vishandle == vinfo)) {
            return v;
         }
      }
   }

   /* Create a new visual and add it to the list. */

   if (NumVisuals>=MAX_VISUALS) {
      fprintf( stderr, "GLX Error: maximum number of visuals exceeded\n");
      return NULL;
   }

   xmvis = XMesaCreateVisual( dpy, vinfo, rgbFlag, alphaFlag, dbFlag,
                              stereoFlag, ximageFlag,
                              depth_size, stencil_size,
                              accumRedSize, accumBlueSize,
                              accumBlueSize, accumAlphaSize, 0, level,
                              GLX_NONE_EXT );
   if (xmvis) {
      VisualTable[NumVisuals] = xmvis;
      NumVisuals++;
   }
   return xmvis;
}



/*
 * Create a GLX visual from a regular XVisualInfo.
 * This is called when Fake GLX is given an XVisualInfo which wasn't
 * returned by glXChooseVisual.  Since this is the first time we're
 * considering this visual we'll take a guess at reasonable values
 * for depth buffer size, stencil size, accum size, etc.
 * This is the best we can do with a client-side emulation of GLX.
 */
static XMesaVisual
create_glx_visual( Display *dpy, XVisualInfo *visinfo )
{
   int vislevel;

   vislevel = level_of_visual( dpy, visinfo );
   if (vislevel) {
      /* Configure this visual as a CI, single-buffered overlay */
      return save_glx_visual( dpy, visinfo,
                              GL_FALSE,  /* rgb */
                              GL_FALSE,  /* alpha */
                              GL_FALSE,  /* double */
                              GL_FALSE,  /* stereo */
                              0,         /* depth bits */
                              0,         /* stencil bits */
                              0,0,0,0,   /* accum bits */
                              vislevel   /* level */
                            );
   }
   else if (is_usable_visual( visinfo )) {
      if (getenv("MESA_GLX_FORCE_CI")) {
         /* Configure this visual as a COLOR INDEX visual. */
         return save_glx_visual( dpy, visinfo,
                                 GL_FALSE,   /* rgb */
                                 GL_FALSE,  /* alpha */
                                 GL_TRUE,   /* double */
                                 GL_FALSE,  /* stereo */
                                 DEFAULT_SOFTWARE_DEPTH_BITS,
                                 8 * sizeof(GLstencil),
                                 0 * sizeof(GLaccum), /* r */
                                 0 * sizeof(GLaccum), /* g */
                                 0 * sizeof(GLaccum), /* b */
                                 0 * sizeof(GLaccum), /* a */
                                 0          /* level */
                               );
      }
      else {
         /* Configure this visual as RGB, double-buffered, depth-buffered. */
         /* This is surely wrong for some people's needs but what else */
         /* can be done?  They should use glXChooseVisual(). */
         return save_glx_visual( dpy, visinfo,
                                 GL_TRUE,   /* rgb */
                                 GL_FALSE,  /* alpha */
                                 GL_TRUE,   /* double */
                                 GL_FALSE,  /* stereo */
                                 DEFAULT_SOFTWARE_DEPTH_BITS,
                                 8 * sizeof(GLstencil),
                                 8 * sizeof(GLaccum), /* r */
                                 8 * sizeof(GLaccum), /* g */
                                 8 * sizeof(GLaccum), /* b */
                                 8 * sizeof(GLaccum), /* a */
                                 0          /* level */
                               );
      }
   }
   else {
      fprintf(stderr,"Mesa: error in glXCreateContext: bad visual\n");
      return NULL;
   }
}



/*
 * Find the GLX visual associated with an XVisualInfo.
 */
static XMesaVisual
find_glx_visual( Display *dpy, XVisualInfo *vinfo )
{
   int i;

   /* First try to match pointers */
   for (i=0;i<NumVisuals;i++) {
      if (VisualTable[i]->display==dpy && VisualTable[i]->vishandle==vinfo) {
         return VisualTable[i];
      }
   }
   /* try to match visual id */
   for (i=0;i<NumVisuals;i++) {
      if (VisualTable[i]->display==dpy
          && VisualTable[i]->visinfo->visualid == vinfo->visualid) {
         return VisualTable[i];
      }
   }
   return NULL;
}



/*
 * Return the transparent pixel value for a GLX visual.
 * Input:  glxvis - the glx_visual
 * Return:  a pixel value or -1 if no transparent pixel
 */
static int transparent_pixel( XMesaVisual glxvis )
{
   Display *dpy = glxvis->display;
   XVisualInfo *vinfo = glxvis->visinfo;
   Atom overlayVisualsAtom;
   OverlayInfo *overlay_info = NULL;
   int numOverlaysPerScreen;
   Status status;
   Atom actualType;
   int actualFormat;
   unsigned long sizeData, bytesLeft;
   int i;

   /*
    * The SERVER_OVERLAY_VISUALS property on the root window contains
    * a list of overlay visuals.  Get that list now.
    */
   overlayVisualsAtom = XInternAtom(dpy,"SERVER_OVERLAY_VISUALS", True);
   if (overlayVisualsAtom == None) {
      return -1;
   }

   status = XGetWindowProperty(dpy, RootWindow( dpy, vinfo->screen ),
                               overlayVisualsAtom, 0L, (long) 10000, False,
                               overlayVisualsAtom, &actualType, &actualFormat,
                               &sizeData, &bytesLeft,
                               (unsigned char **) &overlay_info );

   if (status != Success || actualType != overlayVisualsAtom ||
       actualFormat != 32 || sizeData < 4) {
      /* something went wrong */
      XFree((void *) overlay_info);
      return -1;
   }

   /* search the overlay visual list for the visual ID of interest */
   numOverlaysPerScreen = (int) (sizeData / 4);
   for (i=0;i<numOverlaysPerScreen;i++) {
      OverlayInfo *ov;
      ov = overlay_info + i;
      if (ov->overlay_visual==vinfo->visualid) {
         /* found it! */
         if (ov->transparent_type==0) {
            /* type 0 indicates no transparency */
            XFree((void *) overlay_info);
            return -1;
         }
         else {
            /* ov->value is the transparent pixel */
            XFree((void *) overlay_info);
            return ov->value;
         }
      }
   }

   /* The visual ID was not found in the overlay list. */
   XFree((void *) overlay_info);
   return -1;
}



/*
 * Try to get an X visual which matches the given arguments.
 */
static XVisualInfo *get_visual( Display *dpy, int scr,
			        unsigned int depth, int xclass )
{
   XVisualInfo temp, *vis;
   long mask;
   int n;
   unsigned int default_depth;
   int default_class;

   mask = VisualScreenMask | VisualDepthMask | VisualClassMask;
   temp.screen = scr;
   temp.depth = depth;
   temp.CLASS = xclass;

   default_depth = DefaultDepth(dpy,scr);
   default_class = DefaultVisual(dpy,scr)->CLASS;

   if (depth==default_depth && xclass==default_class) {
      /* try to get root window's visual */
      temp.visualid = DefaultVisual(dpy,scr)->visualid;
      mask |= VisualIDMask;
   }

   vis = XGetVisualInfo( dpy, mask, &temp, &n );

   /* In case bits/pixel > 24, make sure color channels are still <=8 bits.
    * An SGI Infinite Reality system, for example, can have 30bpp pixels:
    * 10 bits per color channel.  Mesa's limited to a max of 8 bits/channel.
    */
   if (vis && depth > 24 && (xclass==TrueColor || xclass==DirectColor)) {
      if (_mesa_bitcount((GLuint) vis->red_mask  ) <= 8 &&
          _mesa_bitcount((GLuint) vis->green_mask) <= 8 &&
          _mesa_bitcount((GLuint) vis->blue_mask ) <= 8) {
         return vis;
      }
      else {
         XFree((void *) vis);
         return NULL;
      }
   }

   return vis;
}



/*
 * Retrieve the value of the given environment variable and find
 * the X visual which matches it.
 * Input:  dpy - the display
 *         screen - the screen number
 *         varname - the name of the environment variable
 * Return:  an XVisualInfo pointer to NULL if error.
 */
static XVisualInfo *get_env_visual(Display *dpy, int scr, const char *varname)
{
   char value[100], type[100];
   int depth, xclass = -1;
   XVisualInfo *vis;

   if (!getenv( varname )) {
      return NULL;
   }

   strncpy( value, getenv(varname), 100 );
   value[99] = 0;

   sscanf( value, "%s %d", type, &depth );

   if (strcmp(type,"TrueColor")==0)          xclass = TrueColor;
   else if (strcmp(type,"DirectColor")==0)   xclass = DirectColor;
   else if (strcmp(type,"PseudoColor")==0)   xclass = PseudoColor;
   else if (strcmp(type,"StaticColor")==0)   xclass = StaticColor;
   else if (strcmp(type,"GrayScale")==0)     xclass = GrayScale;
   else if (strcmp(type,"StaticGray")==0)    xclass = StaticGray;

   if (xclass>-1 && depth>0) {
      vis = get_visual( dpy, scr, depth, xclass );
      if (vis) {
	 return vis;
      }
   }

   fprintf( stderr, "Mesa: GLX unable to find visual class=%s, depth=%d.\n",
	    type, depth );
   return NULL;
}



/*
 * Select an X visual which satisfies the RGBA/CI flag and minimum depth.
 * Input:  dpy, screen - X display and screen number
 *         rgba - GL_TRUE = RGBA mode, GL_FALSE = CI mode
 *         min_depth - minimum visual depth
 *         preferred_class - preferred GLX visual class or DONT_CARE
 * Return:  pointer to an XVisualInfo or NULL.
 */
static XVisualInfo *choose_x_visual( Display *dpy, int screen,
				     GLboolean rgba, int min_depth,
                                     int preferred_class )
{
   XVisualInfo *vis;
   int xclass, visclass = 0;
   int depth;

   if (rgba) {
      Atom hp_cr_maps = XInternAtom(dpy, "_HP_RGB_SMOOTH_MAP_LIST", True);
      /* First see if the MESA_RGB_VISUAL env var is defined */
      vis = get_env_visual( dpy, screen, "MESA_RGB_VISUAL" );
      if (vis) {
	 return vis;
      }
      /* Otherwise, search for a suitable visual */
      if (preferred_class==DONT_CARE) {
         for (xclass=0;xclass<6;xclass++) {
            switch (xclass) {
               case 0:  visclass = TrueColor;    break;
               case 1:  visclass = DirectColor;  break;
               case 2:  visclass = PseudoColor;  break;
               case 3:  visclass = StaticColor;  break;
               case 4:  visclass = GrayScale;    break;
               case 5:  visclass = StaticGray;   break;
            }
            if (min_depth==0) {
               /* start with shallowest */
               for (depth=0;depth<=32;depth++) {
                  if (visclass==TrueColor && depth==8 && !hp_cr_maps) {
                     /* Special case:  try to get 8-bit PseudoColor before */
                     /* 8-bit TrueColor */
                     vis = get_visual( dpy, screen, 8, PseudoColor );
                     if (vis) {
                        return vis;
                     }
                  }
                  vis = get_visual( dpy, screen, depth, visclass );
                  if (vis) {
                     return vis;
                  }
               }
            }
            else {
               /* start with deepest */
               for (depth=32;depth>=min_depth;depth--) {
                  if (visclass==TrueColor && depth==8 && !hp_cr_maps) {
                     /* Special case:  try to get 8-bit PseudoColor before */
                     /* 8-bit TrueColor */
                     vis = get_visual( dpy, screen, 8, PseudoColor );
                     if (vis) {
                        return vis;
                     }
                  }
                  vis = get_visual( dpy, screen, depth, visclass );
                  if (vis) {
                     return vis;
                  }
               }
            }
         }
      }
      else {
         /* search for a specific visual class */
         switch (preferred_class) {
            case GLX_TRUE_COLOR_EXT:    visclass = TrueColor;    break;
            case GLX_DIRECT_COLOR_EXT:  visclass = DirectColor;  break;
            case GLX_PSEUDO_COLOR_EXT:  visclass = PseudoColor;  break;
            case GLX_STATIC_COLOR_EXT:  visclass = StaticColor;  break;
            case GLX_GRAY_SCALE_EXT:    visclass = GrayScale;    break;
            case GLX_STATIC_GRAY_EXT:   visclass = StaticGray;   break;
            default:   return NULL;
         }
         if (min_depth==0) {
            /* start with shallowest */
            for (depth=0;depth<=32;depth++) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
         else {
            /* start with deepest */
            for (depth=32;depth>=min_depth;depth--) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
      }
   }
   else {
      /* First see if the MESA_CI_VISUAL env var is defined */
      vis = get_env_visual( dpy, screen, "MESA_CI_VISUAL" );
      if (vis) {
	 return vis;
      }
      /* Otherwise, search for a suitable visual, starting with shallowest */
      if (preferred_class==DONT_CARE) {
         for (xclass=0;xclass<4;xclass++) {
            switch (xclass) {
               case 0:  visclass = PseudoColor;  break;
               case 1:  visclass = StaticColor;  break;
               case 2:  visclass = GrayScale;    break;
               case 3:  visclass = StaticGray;   break;
            }
            /* try 8-bit up through 16-bit */
            for (depth=8;depth<=16;depth++) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
            /* try min_depth up to 8-bit */
            for (depth=min_depth;depth<8;depth++) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
      }
      else {
         /* search for a specific visual class */
         switch (preferred_class) {
            case GLX_TRUE_COLOR_EXT:    visclass = TrueColor;    break;
            case GLX_DIRECT_COLOR_EXT:  visclass = DirectColor;  break;
            case GLX_PSEUDO_COLOR_EXT:  visclass = PseudoColor;  break;
            case GLX_STATIC_COLOR_EXT:  visclass = StaticColor;  break;
            case GLX_GRAY_SCALE_EXT:    visclass = GrayScale;    break;
            case GLX_STATIC_GRAY_EXT:   visclass = StaticGray;   break;
            default:   return NULL;
         }
         /* try 8-bit up through 16-bit */
         for (depth=8;depth<=16;depth++) {
            vis = get_visual( dpy, screen, depth, visclass );
            if (vis) {
               return vis;
            }
         }
         /* try min_depth up to 8-bit */
         for (depth=min_depth;depth<8;depth++) {
            vis = get_visual( dpy, screen, depth, visclass );
            if (vis) {
               return vis;
            }
         }
      }
   }

   /* didn't find a visual */
   return NULL;
}



/*
 * Find the deepest X over/underlay visual of at least min_depth.
 * Input:  dpy, screen - X display and screen number
 *         level - the over/underlay level
 *         trans_type - transparent pixel type: GLX_NONE_EXT,
 *                      GLX_TRANSPARENT_RGB_EXT, GLX_TRANSPARENT_INDEX_EXT,
 *                      or DONT_CARE
 *         trans_value - transparent pixel value or DONT_CARE
 *         min_depth - minimum visual depth
 *         preferred_class - preferred GLX visual class or DONT_CARE
 * Return:  pointer to an XVisualInfo or NULL.
 */
static XVisualInfo *choose_x_overlay_visual( Display *dpy, int scr,
                                             GLboolean rgbFlag,
                                             int level, int trans_type,
                                             int trans_value,
                                             int min_depth,
                                             int preferred_class )
{
   Atom overlayVisualsAtom;
   OverlayInfo *overlay_info;
   int numOverlaysPerScreen;
   Status status;
   Atom actualType;
   int actualFormat;
   unsigned long sizeData, bytesLeft;
   int i;
   XVisualInfo *deepvis;
   int deepest;

   /*DEBUG int tt, tv; */

   switch (preferred_class) {
      case GLX_TRUE_COLOR_EXT:    preferred_class = TrueColor;    break;
      case GLX_DIRECT_COLOR_EXT:  preferred_class = DirectColor;  break;
      case GLX_PSEUDO_COLOR_EXT:  preferred_class = PseudoColor;  break;
      case GLX_STATIC_COLOR_EXT:  preferred_class = StaticColor;  break;
      case GLX_GRAY_SCALE_EXT:    preferred_class = GrayScale;    break;
      case GLX_STATIC_GRAY_EXT:   preferred_class = StaticGray;   break;
      default:                    preferred_class = DONT_CARE;
   }

   /*
    * The SERVER_OVERLAY_VISUALS property on the root window contains
    * a list of overlay visuals.  Get that list now.
    */
   overlayVisualsAtom = XInternAtom(dpy,"SERVER_OVERLAY_VISUALS", True);
   if (overlayVisualsAtom == (Atom) None) {
      return NULL;
   }

   status = XGetWindowProperty(dpy, RootWindow( dpy, scr ),
                               overlayVisualsAtom, 0L, (long) 10000, False,
                               overlayVisualsAtom, &actualType, &actualFormat,
                               &sizeData, &bytesLeft,
                               (unsigned char **) &overlay_info );

   if (status != Success || actualType != overlayVisualsAtom ||
       actualFormat != 32 || sizeData < 4) {
      /* something went wrong */
      return NULL;
   }

   /* Search for the deepest overlay which satisifies all criteria. */
   deepest = min_depth;
   deepvis = NULL;

   numOverlaysPerScreen = (int) (sizeData / 4);
   for (i=0;i<numOverlaysPerScreen;i++) {
      XVisualInfo *vislist, vistemplate;
      int count;
      OverlayInfo *ov;
      ov = overlay_info + i;

      if (ov->layer!=level) {
         /* failed overlay level criteria */
         continue;
      }
      if (!(trans_type==DONT_CARE
            || (trans_type==GLX_TRANSPARENT_INDEX_EXT
                && ov->transparent_type>0)
            || (trans_type==GLX_NONE_EXT && ov->transparent_type==0))) {
         /* failed transparent pixel type criteria */
         continue;
      }
      if (trans_value!=DONT_CARE && trans_value!=ov->value) {
         /* failed transparent pixel value criteria */
         continue;
      }

      /* get XVisualInfo and check the depth */
      vistemplate.visualid = ov->overlay_visual;
      vistemplate.screen = scr;
      vislist = XGetVisualInfo( dpy, VisualIDMask | VisualScreenMask,
                                &vistemplate, &count );

      if (count!=1) {
         /* something went wrong */
         continue;
      }
      if (preferred_class!=DONT_CARE && preferred_class!=vislist->CLASS) {
         /* wrong visual class */
         continue;
      }

      /* if RGB was requested, make sure we have True/DirectColor */
      if (rgbFlag && vislist->CLASS != TrueColor
          && vislist->CLASS != DirectColor)
         continue;

      /* if CI was requested, make sure we have a color indexed visual */
      if (!rgbFlag
          && (vislist->CLASS == TrueColor || vislist->CLASS == DirectColor))
         continue;

      if (deepvis==NULL || vislist->depth > deepest) {
         /* YES!  found a satisfactory visual */
         if (deepvis) {
            XFree( deepvis );
         }
         deepest = vislist->depth;
         deepvis = vislist;
         /* DEBUG  tt = ov->transparent_type;*/
         /* DEBUG  tv = ov->value; */
      }
   }

/*DEBUG
   if (deepvis) {
      printf("chose 0x%x:  layer=%d depth=%d trans_type=%d trans_value=%d\n",
             deepvis->visualid, level, deepvis->depth, tt, tv );
   }
*/
   return deepvis;
}



static XVisualInfo *
Fake_glXChooseVisual( Display *dpy, int screen, int *list )
{
   int *parselist;
   XVisualInfo *vis;
   int min_ci = 0;
   int min_red=0, min_green=0, min_blue=0, min_alpha=0;
   GLboolean rgb_flag = GL_FALSE;
   GLboolean alpha_flag = GL_FALSE;
   GLboolean double_flag = GL_FALSE;
   GLboolean stereo_flag = GL_FALSE;
   GLint depth_size = 0;
   GLint stencil_size = 0;
   GLint accumRedSize = 0;
   GLint accumGreenSize = 0;
   GLint accumBlueSize = 0;
   GLint accumAlphaSize = 0;
   int level = 0;
   int visual_type = DONT_CARE;
   int trans_type = DONT_CARE;
   int trans_value = DONT_CARE;
   GLint caveat = DONT_CARE;

   parselist = list;

   while (*parselist) {

      switch (*parselist) {
	 case GLX_USE_GL:
	    /* ignore */
	    parselist++;
	    break;
	 case GLX_BUFFER_SIZE:
	    parselist++;
	    min_ci = *parselist++;
	    break;
	 case GLX_LEVEL:
	    parselist++;
            level = *parselist++;
	    break;
	 case GLX_RGBA:
	    rgb_flag = GL_TRUE;
	    parselist++;
	    break;
	 case GLX_DOUBLEBUFFER:
	    double_flag = GL_TRUE;
	    parselist++;
	    break;
	 case GLX_STEREO:
            stereo_flag = GL_TRUE;
            return NULL;
	 case GLX_AUX_BUFFERS:
	    /* ignore */
	    parselist++;
	    parselist++;
	    break;
	 case GLX_RED_SIZE:
	    parselist++;
	    min_red = *parselist++;
	    break;
	 case GLX_GREEN_SIZE:
	    parselist++;
	    min_green = *parselist++;
	    break;
	 case GLX_BLUE_SIZE:
	    parselist++;
	    min_blue = *parselist++;
	    break;
	 case GLX_ALPHA_SIZE:
	    parselist++;
            min_alpha = *parselist++;
            alpha_flag = (min_alpha > 0);
	    break;
	 case GLX_DEPTH_SIZE:
	    parselist++;
	    depth_size = *parselist++;
	    break;
	 case GLX_STENCIL_SIZE:
	    parselist++;
	    stencil_size = *parselist++;
	    break;
	 case GLX_ACCUM_RED_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumRedSize = MAX2( accumRedSize, size );
            }
            break;
	 case GLX_ACCUM_GREEN_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumGreenSize = MAX2( accumGreenSize, size );
            }
            break;
	 case GLX_ACCUM_BLUE_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumBlueSize = MAX2( accumBlueSize, size );
            }
            break;
	 case GLX_ACCUM_ALPHA_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumAlphaSize = MAX2( accumAlphaSize, size );
            }
	    break;

         /*
          * GLX_EXT_visual_info extension
          */
         case GLX_X_VISUAL_TYPE_EXT:
            parselist++;
            visual_type = *parselist++;
            break;
         case GLX_TRANSPARENT_TYPE_EXT:
            parselist++;
            trans_type = *parselist++;
            break;
         case GLX_TRANSPARENT_INDEX_VALUE_EXT:
            parselist++;
            trans_value = *parselist++;
            break;
         case GLX_TRANSPARENT_RED_VALUE_EXT:
         case GLX_TRANSPARENT_GREEN_VALUE_EXT:
         case GLX_TRANSPARENT_BLUE_VALUE_EXT:
         case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
	    /* ignore */
	    parselist++;
	    parselist++;
	    break;
         
         /*
          * GLX_EXT_visual_info extension
          */
         case GLX_VISUAL_CAVEAT_EXT:
            parselist++;
            caveat = *parselist++; /* ignored for now */
            break;

	 case None:
	    break;
	 default:
	    /* undefined attribute */
	    return NULL;
      }
   }

   /* DEBUG
   printf("glXChooseVisual:\n");
   printf("  GLX_RED_SIZE = %d\n", min_red);
   printf("  GLX_GREEN_SIZE = %d\n", min_green);
   printf("  GLX_BLUE_SIZE = %d\n", min_blue);
   printf("  GLX_ALPHA_SIZE = %d\n", min_alpha);
   printf("  GLX_DEPTH_SIZE = %d\n", depth_size);
   printf("  GLX_STENCIL_SIZE = %d\n", stencil_size);
   */

   /*
    * Since we're only simulating the GLX extension this function will never
    * find any real GL visuals.  Instead, all we can do is try to find an RGB
    * or CI visual of appropriate depth.  Other requested attributes such as
    * double buffering, depth buffer, etc. will be associated with the X
    * visual and stored in the VisualTable[].
    */
   if (level==0) {
      /* normal color planes */
      if (rgb_flag) {
         /* Get an RGB visual */
         int min_rgb = min_red + min_green + min_blue;
         if (min_rgb>1 && min_rgb<8) {
            /* a special case to be sure we can get a monochrome visual */
            min_rgb = 1;
         }
         vis = choose_x_visual( dpy, screen, rgb_flag, min_rgb, visual_type );
      }
      else {
         /* Get a color index visual */
         vis = choose_x_visual( dpy, screen, rgb_flag, min_ci, visual_type );
         accumRedSize = accumGreenSize = accumBlueSize = accumAlphaSize = 0;
      }
   }
   else {
      /* over/underlay planes */
      if (rgb_flag) {
         /* rgba overlay */
         int min_rgb = min_red + min_green + min_blue;
         if (min_rgb>1 && min_rgb<8) {
            /* a special case to be sure we can get a monochrome visual */
            min_rgb = 1;
         }
         vis = choose_x_overlay_visual( dpy, screen, rgb_flag, level,
                              trans_type, trans_value, min_rgb, visual_type );
      }
      else {
         /* color index overlay */
         vis = choose_x_overlay_visual( dpy, screen, rgb_flag, level,
                              trans_type, trans_value, min_ci, visual_type );
      }
   }

   if (vis) {
      /* Note: we're not exactly obeying the glXChooseVisual rules here.
       * When GLX_DEPTH_SIZE = 1 is specified we're supposed to choose the
       * largest depth buffer size, which is 32bits/value.  Instead, we
       * return 16 to maintain performance with earlier versions of Mesa.
       */
      if (depth_size > 24)
         depth_size = 31;   /* 32 causes int overflow problems */
      else if (depth_size > 16)
         depth_size = 24;
      else if (depth_size > 0)
         depth_size = DEFAULT_SOFTWARE_DEPTH_BITS; /*16*/

      /* If using Glide, make sure we don't try to setup an impossible
       * visual.  This fixes the Q3 bug in which 24-bit Z was being reported.
       */
      {
         const char *fx = getenv("MESA_GLX_FX");
         if (fx && fx[0] != 'd')
            if (depth_size > 16 ||
                stencil_size > 0 ||
                (min_red > 1 && min_red > 5) ||
                (min_green > 1 && min_green > 6) ||
                (min_blue > 1 && min_blue > 5) ||
                alpha_flag)
               return NULL;
      }


      /* we only support one size of stencil and accum buffers. */
      if (stencil_size > 0)
         stencil_size = STENCIL_BITS;
      if (accumRedSize > 0)
         accumRedSize = ACCUM_BITS;
      if (accumGreenSize > 0)
         accumGreenSize = ACCUM_BITS;
      if (accumBlueSize > 0)
         accumBlueSize = ACCUM_BITS;
      if (accumAlphaSize > 0)
         accumAlphaSize = ACCUM_BITS;
      if (!save_glx_visual( dpy, vis, rgb_flag, alpha_flag, double_flag,
                            stereo_flag, depth_size, stencil_size,
                            accumRedSize, accumGreenSize,
                            accumBlueSize, accumAlphaSize,
                            level ))
         return NULL;
   }

   return vis;
}




static GLXContext
Fake_glXCreateContext( Display *dpy, XVisualInfo *visinfo,
                       GLXContext share_list, Bool direct )
{
   XMesaVisual glxvis;
   XMesaContext xmctx;

   /* deallocate unused windows/buffers */
   XMesaGarbageCollect();

   glxvis = find_glx_visual( dpy, visinfo );
   if (!glxvis) {
      /* This visual wasn't found with glXChooseVisual() */
      glxvis = create_glx_visual( dpy, visinfo );
      if (!glxvis) {
         /* unusable visual */
         return NULL;
      }
   }

   xmctx = XMesaCreateContext( glxvis, (XMesaContext) share_list );
   if (xmctx) {
      /* set the direct/indirect flag */
      xmctx->direct = direct;
   }
   return (GLXContext) xmctx;
}


static GLXContext MakeCurrent_PrevContext = 0;
static GLXDrawable MakeCurrent_PrevDrawable = 0;
static GLXDrawable MakeCurrent_PrevReadable = 0;
static XMesaBuffer MakeCurrent_PrevDrawBuffer = 0;
static XMesaBuffer MakeCurrent_PrevReadBuffer = 0;

/* GLX 1.3 and later */
static Bool
Fake_glXMakeContextCurrent( Display *dpy, GLXDrawable draw,
                            GLXDrawable read, GLXContext ctx )
{
   if (ctx && draw && read) {
      XMesaBuffer drawBuffer, readBuffer;
      XMesaContext xmctx = (XMesaContext) ctx;

      /* Find the XMesaBuffer which corresponds to the GLXDrawable 'draw' */
      if (ctx == MakeCurrent_PrevContext
          && draw == MakeCurrent_PrevDrawable) {
         drawBuffer = MakeCurrent_PrevDrawBuffer;
      }
      else {
         drawBuffer = XMesaFindBuffer( dpy, draw );
      }
      if (!drawBuffer) {
         /* drawable must be a new window! */
         drawBuffer = XMesaCreateWindowBuffer2( xmctx->xm_visual, draw, (XMesaContext) ctx );
         if (!drawBuffer) {
            /* Out of memory, or context/drawable depth mismatch */
            return False;
         }
      }

      /* Find the XMesaBuffer which corresponds to the GLXDrawable 'read' */
      if (ctx == MakeCurrent_PrevContext
          && read == MakeCurrent_PrevReadable) {
         readBuffer = MakeCurrent_PrevReadBuffer;
      }
      else {
         readBuffer = XMesaFindBuffer( dpy, read );
      }
      if (!readBuffer) {
         /* drawable must be a new window! */
         readBuffer = XMesaCreateWindowBuffer2( xmctx->xm_visual, read, (XMesaContext) ctx );
         if (!readBuffer) {
            /* Out of memory, or context/drawable depth mismatch */
            return False;
         }
      }

      MakeCurrent_PrevContext = ctx;
      MakeCurrent_PrevDrawable = draw;
      MakeCurrent_PrevReadable = read;
      MakeCurrent_PrevDrawBuffer = drawBuffer;
      MakeCurrent_PrevReadBuffer = readBuffer;

      /* Now make current! */
      return (Bool) XMesaMakeCurrent2((XMesaContext) ctx, drawBuffer, readBuffer);
   }
   else if (!ctx && !draw && !read) {
      /* release current context w/out assigning new one. */
      XMesaMakeCurrent( NULL, NULL );
      MakeCurrent_PrevContext = 0;
      MakeCurrent_PrevDrawable = 0;
      MakeCurrent_PrevReadable = 0;
      MakeCurrent_PrevDrawBuffer = 0;
      MakeCurrent_PrevReadBuffer = 0;
      return True;
   }
   else {
      /* The args must either all be non-zero or all zero.
       * This is an error.
       */
      return False;
   }
}



static Bool
Fake_glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
   return Fake_glXMakeContextCurrent( dpy, drawable, drawable, ctx );
}



static GLXPixmap
Fake_glXCreateGLXPixmap( Display *dpy, XVisualInfo *visinfo, Pixmap pixmap )
{
   XMesaVisual v;
   XMesaBuffer b;

   v = find_glx_visual( dpy, visinfo );
   if (!v) {
      v = create_glx_visual( dpy, visinfo );
      if (!v) {
         /* unusable visual */
         return 0;
      }
   }

   b = XMesaCreatePixmapBuffer( v, pixmap, 0 );
   if (!b) {
      return 0;
   }
   return b->frontbuffer;
}


#ifdef GLX_MESA_pixmap_colormap
static GLXPixmap
Fake_glXCreateGLXPixmapMESA( Display *dpy, XVisualInfo *visinfo,
                             Pixmap pixmap, Colormap cmap )
{
   XMesaVisual v;
   XMesaBuffer b;

   v = find_glx_visual( dpy, visinfo );
   if (!v) {
      v = create_glx_visual( dpy, visinfo );
      if (!v) {
         /* unusable visual */
         return 0;
      }
   }

   b = XMesaCreatePixmapBuffer( v, pixmap, cmap );
   if (!b) {
      return 0;
   }
   return b->frontbuffer;
}
#endif


static void
Fake_glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, pixmap);
   if (b) {
      XMesaDestroyBuffer(b);
   }
   else if (getenv("MESA_DEBUG")) {
      fprintf( stderr, "Mesa: glXDestroyGLXPixmap: invalid pixmap\n");
   }
}



static void
Fake_glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
                     unsigned long mask )
{
   XMesaContext xm_src = (XMesaContext) src;
   XMesaContext xm_dst = (XMesaContext) dst;
   (void) dpy;
   gl_copy_context( xm_src->gl_ctx, xm_dst->gl_ctx, (GLuint) mask );
}



static Bool
Fake_glXQueryExtension( Display *dpy, int *errorb, int *event )
{
   /* Mesa's GLX isn't really an X extension but we try to act like one. */
   (void) dpy;
   (void) errorb;
   (void) event;
   return True;
}


extern void _kw_ungrab_all( Display *dpy );
void _kw_ungrab_all( Display *dpy )
{
   XUngrabPointer( dpy, CurrentTime );
   XUngrabKeyboard( dpy, CurrentTime );
}


static void
Fake_glXDestroyContext( Display *dpy, GLXContext ctx )
{
   (void) dpy;
   MakeCurrent_PrevContext = 0;
   MakeCurrent_PrevDrawable = 0;
   MakeCurrent_PrevReadable = 0;
   MakeCurrent_PrevDrawBuffer = 0;
   MakeCurrent_PrevReadBuffer = 0;
   XMesaDestroyContext( (XMesaContext) ctx );
   XMesaGarbageCollect();
}



static Bool
Fake_glXIsDirect( Display *dpy, GLXContext ctx )
{
   (void) dpy;
   return ((XMesaContext) ctx)->direct;
}



static void
Fake_glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
   XMesaBuffer buffer = XMesaFindBuffer( dpy, drawable );

   if (buffer) {
      XMesaSwapBuffers(buffer);
   }
   else if (getenv("MESA_DEBUG")) {
      fprintf(stderr, "Mesa Warning: glXSwapBuffers: invalid drawable\n");
   }
}



#ifdef GLX_MESA_copy_sub_buffer
static void
Fake_glXCopySubBufferMESA( Display *dpy, GLXDrawable drawable,
                           int x, int y, int width, int height )
{
   XMesaBuffer buffer = XMesaFindBuffer( dpy, drawable );
   if (buffer) {
      XMesaCopySubBuffer(buffer, x, y, width, height);
   }
   else if (getenv("MESA_DEBUG")) {
      fprintf(stderr, "Mesa Warning: glXCopySubBufferMESA: invalid drawable\n");
   }
}
#endif



static Bool
Fake_glXQueryVersion( Display *dpy, int *maj, int *min )
{
   (void) dpy;
   /* Return GLX version, not Mesa version */
   assert(CLIENT_MAJOR_VERSION == SERVER_MAJOR_VERSION);
   *maj = CLIENT_MAJOR_VERSION;
   *min = MIN2( CLIENT_MINOR_VERSION, SERVER_MINOR_VERSION );
   return True;
}



/*
 * Query the GLX attributes of the given XVisualInfo.
 */
static int
Fake_glXGetConfig( Display *dpy, XVisualInfo *visinfo,
                   int attrib, int *value )
{
   XMesaVisual glxvis;

   glxvis = find_glx_visual( dpy, visinfo );
   if (!glxvis) {
      /* this visual wasn't obtained with glXChooseVisual */
      glxvis = create_glx_visual( dpy, visinfo );
      if (!glxvis) {
	 /* this visual can't be used for GL rendering */
	 if (attrib==GLX_USE_GL) {
	    *value = (int) False;
	    return 0;
	 }
	 else {
	    /*fprintf( stderr, "Mesa: Error in glXGetConfig: bad visual\n");*/
	    return GLX_BAD_VISUAL;
	 }
      }
   }

   switch(attrib) {
      case GLX_USE_GL:
         *value = (int) True;
	 return 0;
      case GLX_BUFFER_SIZE:
	 *value = visinfo->depth;
	 return 0;
      case GLX_LEVEL:
	 *value = glxvis->level;
	 return 0;
      case GLX_RGBA:
	 if (glxvis->gl_visual->RGBAflag) {
	    *value = True;
	 }
	 else {
	    *value = False;
	 }
	 return 0;
      case GLX_DOUBLEBUFFER:
	 *value = (int) glxvis->gl_visual->DBflag;
	 return 0;
      case GLX_STEREO:
	 *value = (int) glxvis->gl_visual->StereoFlag;
	 return 0;
      case GLX_AUX_BUFFERS:
	 *value = (int) False;
	 return 0;
      case GLX_RED_SIZE:
         *value = glxvis->gl_visual->RedBits;
	 return 0;
      case GLX_GREEN_SIZE:
         *value = glxvis->gl_visual->GreenBits;
	 return 0;
      case GLX_BLUE_SIZE:
         *value = glxvis->gl_visual->BlueBits;
	 return 0;
      case GLX_ALPHA_SIZE:
         *value = glxvis->gl_visual->AlphaBits;
	 return 0;
      case GLX_DEPTH_SIZE:
         *value = glxvis->gl_visual->DepthBits;
	 return 0;
      case GLX_STENCIL_SIZE:
	 *value = glxvis->gl_visual->StencilBits;
	 return 0;
      case GLX_ACCUM_RED_SIZE:
	 *value = glxvis->gl_visual->AccumRedBits;
	 return 0;
      case GLX_ACCUM_GREEN_SIZE:
	 *value = glxvis->gl_visual->AccumGreenBits;
	 return 0;
      case GLX_ACCUM_BLUE_SIZE:
	 *value = glxvis->gl_visual->AccumBlueBits;
	 return 0;
      case GLX_ACCUM_ALPHA_SIZE:
         *value = glxvis->gl_visual->AccumAlphaBits;
	 return 0;

      /*
       * GLX_EXT_visual_info extension
       */
      case GLX_X_VISUAL_TYPE_EXT:
         switch (glxvis->visinfo->CLASS) {
            case StaticGray:   *value = GLX_STATIC_GRAY_EXT;   return 0;
            case GrayScale:    *value = GLX_GRAY_SCALE_EXT;    return 0;
            case StaticColor:  *value = GLX_STATIC_GRAY_EXT;   return 0;
            case PseudoColor:  *value = GLX_PSEUDO_COLOR_EXT;  return 0;
            case TrueColor:    *value = GLX_TRUE_COLOR_EXT;    return 0;
            case DirectColor:  *value = GLX_DIRECT_COLOR_EXT;  return 0;
         }
         return 0;
      case GLX_TRANSPARENT_TYPE_EXT:
         if (glxvis->level==0) {
            /* normal planes */
            *value = GLX_NONE_EXT;
         }
         else if (glxvis->level>0) {
            /* overlay */
            if (glxvis->gl_visual->RGBAflag) {
               *value = GLX_TRANSPARENT_RGB_EXT;
            }
            else {
               *value = GLX_TRANSPARENT_INDEX_EXT;
            }
         }
         else if (glxvis->level<0) {
            /* underlay */
            *value = GLX_NONE_EXT;
         }
         return 0;
      case GLX_TRANSPARENT_INDEX_VALUE_EXT:
         {
            int pixel = transparent_pixel( glxvis );
            if (pixel>=0) {
               *value = pixel;
            }
            /* else undefined */
         }
         return 0;
      case GLX_TRANSPARENT_RED_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_GREEN_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_BLUE_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
         /* undefined */
         return 0;

      /*
       * GLX_EXT_visual_info extension
       */
      case GLX_VISUAL_CAVEAT_EXT:
         /* test for zero, just in case */
         if (glxvis->VisualCaveat > 0)
            *value = glxvis->VisualCaveat;
         else
            *value = GLX_NONE_EXT;
         return 0;

      /*
       * Extensions
       */
      default:
	 return GLX_BAD_ATTRIBUTE;
   }
}



static void
Fake_glXWaitGL( void )
{
   XMesaContext xmesa = XMesaGetCurrentContext();
   XMesaFlush( xmesa );
}



static void
Fake_glXWaitX( void )
{
   XMesaContext xmesa = XMesaGetCurrentContext();
   XMesaFlush( xmesa );
}


/*
 * Return the extensions string, which is 3Dfx-dependant.
 */
static const char *get_extensions( void )
{
#ifdef FX
   const char *fx = getenv("MESA_GLX_FX");
   if (fx && fx[0] != 'd') {
      return "GLX_MESA_pixmap_colormap GLX_EXT_visual_info GLX_EXT_visual_rating GLX_MESA_release_buffers GLX_MESA_copy_sub_buffer GLX_SGI_video_sync GLX_MESA_set_3dfx_mode GLX_ARB_get_proc_address";
   }
#endif
   return "GLX_MESA_pixmap_colormap GLX_EXT_visual_info GLX_EXT_visual_rating GLX_MESA_release_buffers GLX_MESA_copy_sub_buffer GLX_SGI_video_sync GLX_ARB_get_proc_address";
}



/* GLX 1.1 and later */
static const char *
Fake_glXQueryExtensionsString( Display *dpy, int screen )
{
   (void) dpy;
   (void) screen;
   return get_extensions();
}



/* GLX 1.1 and later */
static const char *
Fake_glXQueryServerString( Display *dpy, int screen, int name )
{
   static char version[1000];
   sprintf(version, "%d.%d %s", SERVER_MAJOR_VERSION, SERVER_MINOR_VERSION,
           MESA_GLX_VERSION);

   (void) dpy;
   (void) screen;

   switch (name) {
      case GLX_EXTENSIONS:
         return get_extensions();
      case GLX_VENDOR:
	 return VENDOR;
      case GLX_VERSION:
	 return version;
      default:
         return NULL;
   }
}



/* GLX 1.1 and later */
static const char *
Fake_glXGetClientString( Display *dpy, int name )
{
   static char version[1000];
   sprintf(version, "%d.%d %s", CLIENT_MAJOR_VERSION, CLIENT_MINOR_VERSION,
           MESA_GLX_VERSION);

   (void) dpy;

   switch (name) {
      case GLX_EXTENSIONS:
         return get_extensions();
      case GLX_VENDOR:
	 return VENDOR;
      case GLX_VERSION:
	 return version;
      default:
         return NULL;
   }
}



/*
 * GLX 1.3 and later
 */

static GLXFBConfig *
Fake_glXChooseFBConfig( Display *dpy, int screen,
                        const int *attribList, int *nitems )
{
   (void) dpy;
   (void) screen;
   (void) attribList;
   (void) nitems;
   return 0;
}


static int
Fake_glXGetFBConfigAttrib( Display *dpy, GLXFBConfig config,
                           int attribute, int *value )
{
   (void) dpy;
   (void) config;
   (void) attribute;
   (void) value;
   return 0;
}


static GLXFBConfig *
Fake_glXGetFBConfigs( Display *dpy, int screen, int *nelements )
{
   (void) dpy;
   (void) screen;
   (void) nelements;
   return 0;
}


static XVisualInfo *
Fake_glXGetVisualFromFBConfig( Display *dpy, GLXFBConfig config )
{
   (void) dpy;
   (void) config;
   return 0;
}


static GLXWindow
Fake_glXCreateWindow( Display *dpy, GLXFBConfig config, Window win,
                      const int *attribList )
{
   (void) dpy;
   (void) config;
   (void) win;
   (void) attribList;
   return 0;
}


static void
Fake_glXDestroyWindow( Display *dpy, GLXWindow window )
{
   (void) dpy;
   (void) window;
   return;
}


static GLXPixmap
Fake_glXCreatePixmap( Display *dpy, GLXFBConfig config, Pixmap pixmap,
                      const int *attribList )
{
   (void) dpy;
   (void) config;
   (void) pixmap;
   (void) attribList;
   return 0;
}


static void
Fake_glXDestroyPixmap( Display *dpy, GLXPixmap pixmap )
{
   (void) dpy;
   (void) pixmap;
   return;
}


static GLXPbuffer
Fake_glXCreatePbuffer( Display *dpy, GLXFBConfig config,
                       const int *attribList )
{
   (void) dpy;
   (void) config;
   (void) attribList;
   return 0;
}


static void
Fake_glXDestroyPbuffer( Display *dpy, GLXPbuffer pbuf )
{
   (void) dpy;
   (void) pbuf;
}


static void
Fake_glXQueryDrawable( Display *dpy, GLXDrawable draw, int attribute,
                       unsigned int *value )
{
   (void) dpy;
   (void) draw;
   (void) attribute;
   (void) value;
}


static GLXContext
Fake_glXCreateNewContext( Display *dpy, GLXFBConfig config,
                          int renderType, GLXContext shareList, Bool direct )
{
   (void) dpy;
   (void) config;
   (void) renderType;
   (void) shareList;
   (void) direct;
   return 0;
}


static int
Fake_glXQueryContext( Display *dpy, GLXContext ctx, int attribute, int *value )
{
   (void) dpy;
   (void) ctx;
   (void) attribute;
   (void) value;
   return 0;
}


static void
Fake_glXSelectEvent( Display *dpy, GLXDrawable drawable, unsigned long mask )
{
   (void) dpy;
   (void) drawable;
   (void) mask;
}


static void
Fake_glXGetSelectedEvent( Display *dpy, GLXDrawable drawable,
                          unsigned long *mask )
{
   (void) dpy;
   (void) drawable;
   (void) mask;
}



#ifdef GLX_MESA_release_buffers
/*
 * Release the depth, stencil, accum buffers attached to a GLXDrawable
 * (a window or pixmap) prior to destroying the GLXDrawable.
 */
static Bool
Fake_glXReleaseBuffersMESA( Display *dpy, GLXDrawable d )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, d);
   if (b) {
      XMesaDestroyBuffer(b);
      return True;
   }
   return False;
}
#endif



#ifdef GLX_MESA_set_3dfx_mode
static GLboolean
Fake_glXSet3DfxModeMESA( GLint mode )
{
   return XMesaSetFXmode( mode );
}
#endif



/*
 * GLX_SGI_video_sync
 */

#ifdef GLX_SGI_video_sync

static int
Fake_glXGetVideoSyncSGI(unsigned int *count)
{
   return 0;
}


static int
Fake_glXWaitVideoSyncSGI(int divisor, int remainder, unsigned int *count)
{
   return 0;
}

#endif



extern struct _glxapi_table *_mesa_GetGLXDispatchTable(void);
struct _glxapi_table *_mesa_GetGLXDispatchTable(void)
{
   static struct _glxapi_table glx;

   /* be sure our dispatch table size <= libGL's table */
   {
      int size = sizeof(struct _glxapi_table) / sizeof(void *);
      (void) size;
      assert((GLint) _glxapi_get_dispatch_table_size() >= size);
   }

   /* initialize the whole table to no-ops */
   _glxapi_set_no_op_table(&glx);

   /* now initialize the table with the functions I implement */
   glx.ChooseVisual = Fake_glXChooseVisual;
   glx.CopyContext = Fake_glXCopyContext;
   glx.CreateContext = Fake_glXCreateContext;
   glx.CreateGLXPixmap = Fake_glXCreateGLXPixmap;
   glx.DestroyContext = Fake_glXDestroyContext;
   glx.DestroyGLXPixmap = Fake_glXDestroyGLXPixmap;
   glx.GetConfig = Fake_glXGetConfig;
   /*glx.GetCurrentContext = Fake_glXGetCurrentContext;*/
   /*glx.GetCurrentDrawable = Fake_glXGetCurrentDrawable;*/
   glx.IsDirect = Fake_glXIsDirect;
   glx.MakeCurrent = Fake_glXMakeCurrent;
   glx.QueryExtension = Fake_glXQueryExtension;
   glx.QueryVersion = Fake_glXQueryVersion;
   glx.SwapBuffers = Fake_glXSwapBuffers;
   glx.UseXFont = Fake_glXUseXFont;
   glx.WaitGL = Fake_glXWaitGL;
   glx.WaitX = Fake_glXWaitX;

#ifdef GLX_VERSION_1_1
   glx.GetClientString = Fake_glXGetClientString;
   glx.QueryExtensionsString = Fake_glXQueryExtensionsString;
   glx.QueryServerString = Fake_glXQueryServerString;
#endif

#ifdef GLX_VERSION_1_2
   /*glx.GetCurrentDisplay = Fake_glXGetCurrentDisplay;*/
#endif

#ifdef GLX_VERSION_1_3
   glx.ChooseFBConfig = Fake_glXChooseFBConfig;
   glx.CreateNewContext = Fake_glXCreateNewContext;
   glx.CreatePbuffer = Fake_glXCreatePbuffer;
   glx.CreatePixmap = Fake_glXCreatePixmap;
   glx.CreateWindow = Fake_glXCreateWindow;
   glx.DestroyPbuffer = Fake_glXDestroyPbuffer;
   glx.DestroyPixmap = Fake_glXDestroyPixmap;
   glx.DestroyWindow = Fake_glXDestroyWindow;
   /*glx.GetCurrentReadDrawable = Fake_glXGetCurrentReadDrawable;*/
   glx.GetFBConfigAttrib = Fake_glXGetFBConfigAttrib;
   glx.GetFBConfigs = Fake_glXGetFBConfigs;
   glx.GetSelectedEvent = Fake_glXGetSelectedEvent;
   glx.GetVisualFromFBConfig = Fake_glXGetVisualFromFBConfig;
   glx.MakeContextCurrent = Fake_glXMakeContextCurrent;
   glx.QueryContext = Fake_glXQueryContext;
   glx.QueryDrawable = Fake_glXQueryDrawable;
   glx.SelectEvent = Fake_glXSelectEvent;
#endif

#ifdef GLX_SGI_video_sync
   glx.GetVideoSyncSGI = Fake_glXGetVideoSyncSGI;
   glx.WaitVideoSyncSGI = Fake_glXWaitVideoSyncSGI;
#endif

#ifdef GLX_MESA_copy_sub_buffer
   glx.CopySubBufferMESA = Fake_glXCopySubBufferMESA;
#endif

#ifdef GLX_MESA_release_buffers
   glx.ReleaseBuffersMESA = Fake_glXReleaseBuffersMESA;
#endif

#ifdef GLX_MESA_pixmap_colormap
   glx.CreateGLXPixmapMESA = Fake_glXCreateGLXPixmapMESA;
#endif

#ifdef GLX_MESA_set_3dfx_mode
   glx.Set3DfxModeMESA = Fake_glXSet3DfxModeMESA;
#endif

   return &glx;
}
