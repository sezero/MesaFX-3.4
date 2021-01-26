/* $Id: svgamesa15.c,v 1.6 2000/06/14 21:59:07 brianp Exp $ */

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
#include "svgamesa15.h"

static void __svga_drawpixel15(int x, int y, unsigned long c)
{
    unsigned long offset;
    GLshort *shortBuffer=(void *)SVGABuffer.DrawBuffer;
    y = SVGAInfo->height-y-1;
    offset = y * SVGAInfo->width + x;
    shortBuffer[offset]=c;
}

static unsigned long __svga_getpixel15(int x, int y)
{
    unsigned long offset;
    GLshort *shortBuffer=(void *)SVGABuffer.ReadBuffer;
    y = SVGAInfo->height-y-1;
    offset = y * SVGAInfo->width + x;
    return shortBuffer[offset];
}

void __set_color15( GLcontext *ctx,
                    GLubyte red, GLubyte green,
                    GLubyte blue, GLubyte alpha )
{
   SVGAMesa->hicolor=(red>>3)<<10 | (green>>3)<<5 | (blue>>3); 
/*   SVGAMesa->hicolor=(red)<<10 | (green)<<5 | (blue); */
}   

void __clear_color15( GLcontext *ctx,
                      GLubyte red, GLubyte green,
                      GLubyte blue, GLubyte alpha )
{
   SVGAMesa->clear_hicolor=(red>>3)<<10 | (green>>3)<<5 | (blue>>3);  
/*   SVGAMesa->clear_hicolor=(red)<<10 | (green)<<5 | (blue);*/
}   

GLbitfield __clear15( GLcontext *ctx, GLbitfield mask, GLboolean all,
                      GLint x, GLint y, GLint width, GLint height )
{
   int i, j;

   if (mask & DD_FRONT_LEFT_BIT) {
      GLshort *shortBuffer=(void *)SVGABuffer.FrontBuffer;
      if (all) {
         for (i=0;i<SVGABuffer.BufferSize / 2;i++)
            shortBuffer[i]=SVGAMesa->clear_hicolor;
      }
      else {
         GLubyte *tmp = SVGABuffer.DrawBuffer;
         SVGABuffer.DrawBuffer = SVGABuffer.FrontBuffer;
         for (i=x;i<width;i++)
            for (j=y;j<height;j++)
               __svga_drawpixel15(i,j,SVGAMesa->clear_hicolor);
         SVGABuffer.DrawBuffer = tmp;
      }
   }
   if (mask & DD_BACK_LEFT_BIT) {
      GLshort *shortBuffer=(void *)SVGABuffer.BackBuffer;
      if (all) {
         for (i=0;i<SVGABuffer.BufferSize / 2;i++)
            shortBuffer[i]=SVGAMesa->clear_hicolor;
      }
      else {
         GLubyte *tmp = SVGABuffer.DrawBuffer;
         SVGABuffer.DrawBuffer = SVGABuffer.BackBuffer;
         for (i=x;i<width;i++)
            for (j=y;j<height;j++)
               __svga_drawpixel15(i,j,SVGAMesa->clear_hicolor);
         SVGABuffer.DrawBuffer = tmp;
      }
   }
   return mask & (~(DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT));
}

void __write_rgba_span15( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                          const GLubyte rgba[][4], const GLubyte mask[] )
{
   int i;
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++, x++) {
         if (mask[i]) {
         __svga_drawpixel15( x, y, (rgba[i][RCOMP]>>3)<<10 | \
			           (rgba[i][GCOMP]>>3)<<5 |  \
			           (rgba[i][BCOMP]>>3));
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++, x++) {
         __svga_drawpixel15( x, y, (rgba[i][RCOMP]>>3)<<10 | \
			           (rgba[i][GCOMP]>>3)<<5  | \
			           (rgba[i][BCOMP]>>3));
      }
   }
}

void __write_mono_rgba_span15( const GLcontext *ctx,
                               GLuint n, GLint x, GLint y,
                               const GLubyte mask[])
{
   int i;
   for (i=0; i<n; i++, x++) {
      if (mask[i]) {
         __svga_drawpixel15( x, y, SVGAMesa->hicolor);
      }
   }
}

void __read_rgba_span15( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                         GLubyte rgba[][4] )
{
   int i,pix;
   for (i=0; i<n; i++, x++) {
    pix = __svga_getpixel15( x, y);
    rgba[i][RCOMP] = ((pix>>10)<<3) & 0xff;
    rgba[i][GCOMP] = ((pix>> 5)<<3) & 0xff;
    rgba[i][BCOMP] = ((pix    )<<3) & 0xff;
   }
}

void __write_rgba_pixels15( const GLcontext *ctx,
                            GLuint n, const GLint x[], const GLint y[],
                            const GLubyte rgba[][4], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         __svga_drawpixel15( x[i], y[i], (rgba[i][RCOMP]>>3)<<10 | \
			                 (rgba[i][GCOMP]>>3)<<5  | \
			                 (rgba[i][BCOMP]>>3));
      }
   }
}


void __write_mono_rgba_pixels15( const GLcontext *ctx,
                                 GLuint n,
                                 const GLint x[], const GLint y[],
                                 const GLubyte mask[] )
{
   int i;
   /* use current rgb color */
   for (i=0; i<n; i++) {
      if (mask[i]) {
         __svga_drawpixel15( x[i], y[i], SVGAMesa->hicolor );
      }
   }
}

void __read_rgba_pixels15( const GLcontext *ctx,
                           GLuint n, const GLint x[], const GLint y[],
                           GLubyte rgba[][4], const GLubyte mask[] )
{
   int i,pix;
   for (i=0; i<n; i++,x++) {
    pix = __svga_getpixel15( x[i], y[i] );
    rgba[i][RCOMP] = ((pix>>10)<<3) & 0xff;
    rgba[i][GCOMP] = ((pix>> 5)<<3) & 0xff;
    rgba[i][BCOMP] = ((pix    )<<3) & 0xff;
   }
}

#else


/* silence compiler warning */
extern void _mesa_svga15_dummy_function(void);
void _mesa_svga15_dummy_function(void)
{
}


#endif
