/* $Id: svgamesa24.c,v 1.7 2000/06/14 21:59:07 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 * Copyright (C) 1995-2000  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * SVGA driver for Mesa.
 * Original author:  Brian Paul
 * Additional authors:  Slawomir Szczyrba <steev@hot.pl>  (Mesa 3.2)
 */

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#ifdef SVGA

#include "svgapix.h"
#include "svgamesa24.h"


#if 0
/* this doesn't compile with GCC on RedHat 6.1 */
static inline int RGB2BGR24(int c)
{
	asm("rorw  $8, %0\n"	 
	    "rorl $16, %0\n"	 
	    "rorw  $8, %0\n"	 
	    "shrl  $8, %0\n"	 
      : "=q"(c):"0"(c));
    return c;
}
#else
static unsigned long RGB2BGR24(unsigned long color)
{
   return (color & 0xff00)|(color>>16)|((color & 0xff)<<16);
}
#endif

static void __svga_drawpixel24(int x, int y, GLubyte r, GLubyte g, GLubyte b)
{
    unsigned long offset;

    _RGB *rgbBuffer=(void *)SVGABuffer.DrawBuffer;
    y = SVGAInfo->height-y-1;
    offset = y * SVGAInfo->width + x;

    rgbBuffer[offset].r=r;
    rgbBuffer[offset].g=g;
    rgbBuffer[offset].b=b;
}

static unsigned long __svga_getpixel24(int x, int y)
{
    unsigned long offset;

    _RGB *rgbBuffer=(void *)SVGABuffer.ReadBuffer;
    y = SVGAInfo->height-y-1;
    offset = y * SVGAInfo->width + x;
    return rgbBuffer[offset].r<<16 | rgbBuffer[offset].g<<8 | rgbBuffer[offset].b;
}

void __set_color24( GLcontext *ctx,
                    GLubyte red, GLubyte green,
                    GLubyte blue, GLubyte alpha )
{
   SVGAMesa->red = red;
   SVGAMesa->green = green;
   SVGAMesa->blue = blue;
/*   SVGAMesa->truecolor = red<<16 | green<<8 | blue; */
}

void __clear_color24( GLcontext *ctx,
                      GLubyte red, GLubyte green,
                      GLubyte blue, GLubyte alpha )
{
   SVGAMesa->clear_red = red;
   SVGAMesa->clear_green = green;
   SVGAMesa->clear_blue = blue;
/*   SVGAMesa->clear_truecolor = red<<16 | green<<8 | blue; */
}

GLbitfield __clear24( GLcontext *ctx, GLbitfield mask, GLboolean all,
                      GLint x, GLint y, GLint width, GLint height )
{
   int i,j;
   
   if (mask & DD_FRONT_LEFT_BIT) {
      if (all) {
         _RGB *rgbBuffer=(void *)SVGABuffer.FrontBuffer;
         for (i=0;i<SVGABuffer.BufferSize / 3;i++) {
            rgbBuffer[i].r=SVGAMesa->clear_red;
            rgbBuffer[i].g=SVGAMesa->clear_green;
            rgbBuffer[i].b=SVGAMesa->clear_blue;
         } 
      }
      else {
         GLubyte *tmp = SVGABuffer.DrawBuffer;
         SVGABuffer.DrawBuffer = SVGABuffer.FrontBuffer;
         for (i=x;i<width;i++)    
            for (j=y;j<height;j++)    
               __svga_drawpixel24( i, j, SVGAMesa->clear_red,
                                   SVGAMesa->clear_green,
                                   SVGAMesa->clear_blue);
         SVGABuffer.DrawBuffer = tmp;
      }	
   }
   if (mask & DD_BACK_LEFT_BIT) {
      if (all) {
         _RGB *rgbBuffer=(void *)SVGABuffer.BackBuffer;
         for (i=0;i<SVGABuffer.BufferSize / 3;i++) {
            rgbBuffer[i].r=SVGAMesa->clear_red;
            rgbBuffer[i].g=SVGAMesa->clear_green;
            rgbBuffer[i].b=SVGAMesa->clear_blue;
         } 
      }
      else {
         GLubyte *tmp = SVGABuffer.DrawBuffer;
         SVGABuffer.DrawBuffer = SVGABuffer.BackBuffer;
         for (i=x;i<width;i++)    
            for (j=y;j<height;j++)    
               __svga_drawpixel24( i, j, SVGAMesa->clear_red,
                                   SVGAMesa->clear_green,
                                   SVGAMesa->clear_blue);
         SVGABuffer.DrawBuffer = tmp;
      }	
   }
   return mask & (~(DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT));
}

void __write_rgba_span24( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                          const GLubyte rgba[][4], const GLubyte mask[] )
{
   int i;
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++, x++) {
         if (mask[i]) {
         __svga_drawpixel24( x, y, rgba[i][RCOMP],
	                           rgba[i][GCOMP],
				   rgba[i][BCOMP]);
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++, x++) {
         __svga_drawpixel24( x, y, rgba[i][RCOMP],
	                           rgba[i][GCOMP],
				   rgba[i][BCOMP]);
      }
   }
}

void __write_mono_rgba_span24( const GLcontext *ctx,
                               GLuint n, GLint x, GLint y,
                               const GLubyte mask[])
{
   int i;
   for (i=0; i<n; i++, x++) {
      if (mask[i]) {
         __svga_drawpixel24( x, y, SVGAMesa->red,
                                   SVGAMesa->green,
				   SVGAMesa->blue);
      }
   }
}

void __read_rgba_span24( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                         GLubyte rgba[][4] )
{
   int i;
   for (i=0; i<n; i++, x++) {
    *((GLint*)rgba[i]) = RGB2BGR24(__svga_getpixel24( x, y));
   }
}

void __write_rgba_pixels24( const GLcontext *ctx,
                            GLuint n, const GLint x[], const GLint y[],
                            const GLubyte rgba[][4], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         __svga_drawpixel24( x[i], y[i], rgba[i][RCOMP],
	                                 rgba[i][GCOMP],
				         rgba[i][BCOMP]);
      }
   }
}

void __write_mono_rgba_pixels24( const GLcontext *ctx,
                                 GLuint n,
                                 const GLint x[], const GLint y[],
                                 const GLubyte mask[] )
{
   int i;
   /* use current rgb color */
   for (i=0; i<n; i++) {
      if (mask[i]) {
         __svga_drawpixel24( x[i], y[i], SVGAMesa->red,
                                         SVGAMesa->green,
				         SVGAMesa->blue);
      }
   }
}

void __read_rgba_pixels24( const GLcontext *ctx,
                           GLuint n, const GLint x[], const GLint y[],
                           GLubyte rgba[][4], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++,x++) {
    *((GLint*)rgba[i]) = RGB2BGR24(__svga_getpixel24( x[i], y[i]));    
   }
}

#else


/* silence compiler warning */
extern void _mesa_svga24_dummy_function(void);
void _mesa_svga24_dummy_function(void)
{
}


#endif
