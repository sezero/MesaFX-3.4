/* $Id: linetemp.h,v 1.6.4.1 2000/11/05 21:24:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.1
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


/*
 * Line Rasterizer Template
 *
 * This file is #include'd to generate custom line rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated along the line:
 *    INTERP_Z      - if defined, interpolate Z values
 *    INTERP_RGB    - if defined, interpolate RGB values
 *    INTERP_SPEC   - if defined, interpolate specular RGB values
 *    INTERP_ALPHA  - if defined, interpolate Alpha values
 *    INTERP_INDEX  - if defined, interpolate color index values
 *    INTERP_ST     - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_STUV0   - if defined, interpolate unit 0 STU texcoords with
 *                         perspective correction
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_STUV1   - if defined, interpolate unit 1 STU texcoords
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to directly compute pixel addresses during
 * rasterization (see pixelPtr):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code
 *    SETUP_CODE    - code which is to be executed once per line
 *
 * To enable line stippling define STIPPLE = 1
 * To enable wide lines define WIDE = 1
 * 
 * To actually "plot" each pixel either the PLOT macro or
 * (XMAJOR_PLOT and YMAJOR_PLOT macros) must be defined...
 *    PLOT(X,Y) - code to plot a pixel.  Example:
 *                if (Z < *zPtr) {
 *                   *zPtr = Z;
 *                   color = pack_rgb( FixedToInt(r0), FixedToInt(g0),
 *                                     FixedToInt(b0) );
 *                   put_pixel( X, Y, color );
 *                }
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 */


/*void line( GLcontext *ctx, GLuint vert0, GLuint vert1, GLuint pvert )*/
{
   const struct vertex_buffer *VB = ctx->VB;
   GLint x0 = (GLint) VB->Win.data[vert0][0];
   GLint x1 = (GLint) VB->Win.data[vert1][0];
   GLint y0 = (GLint) VB->Win.data[vert0][1];
   GLint y1 = (GLint) VB->Win.data[vert1][1];
   GLint dx, dy;
#ifdef INTERP_XY
   GLint xstep, ystep;
#endif
#ifdef INTERP_Z
   GLint z0, z1, dz;
   const GLint depthBits = ctx->Visual->DepthBits;
   const GLint fixedToDepthShift = depthBits <= 16 ? FIXED_SHIFT : 0;
#  define FixedToDepth(F)  ((F) >> fixedToDepthShift)
#  ifdef DEPTH_TYPE
     GLint zPtrXstep, zPtrYstep;
     DEPTH_TYPE *zPtr;
#  endif
#endif
#ifdef INTERP_RGB
   GLfixed r0 = IntToFixed(VB->ColorPtr->data[vert0][0]);
   GLfixed dr = IntToFixed(VB->ColorPtr->data[vert1][0]) - r0;
   GLfixed g0 = IntToFixed(VB->ColorPtr->data[vert0][1]);
   GLfixed dg = IntToFixed(VB->ColorPtr->data[vert1][1]) - g0;
   GLfixed b0 = IntToFixed(VB->ColorPtr->data[vert0][2]);
   GLfixed db = IntToFixed(VB->ColorPtr->data[vert1][2]) - b0;
#endif
#ifdef INTERP_SPEC
   GLfixed sr0 = VB->Specular ? IntToFixed(VB->Specular[vert0][0]) : 0;
   GLfixed dsr = VB->Specular ? IntToFixed(VB->Specular[vert1][0]) - sr0 : 0;
   GLfixed sg0 = VB->Specular ? IntToFixed(VB->Specular[vert0][1]) : 0;
   GLfixed dsg = VB->Specular ? IntToFixed(VB->Specular[vert1][1]) - sg0 : 0;
   GLfixed sb0 = VB->Specular ? IntToFixed(VB->Specular[vert0][2]) : 0;
   GLfixed dsb = VB->Specular ? IntToFixed(VB->Specular[vert1][2]) - sb0 : 0;
#endif
#ifdef INTERP_ALPHA
   GLfixed a0 = IntToFixed(VB->ColorPtr->data[vert0][3]);
   GLfixed da = IntToFixed(VB->ColorPtr->data[vert1][3]) - a0;
#endif
#ifdef INTERP_INDEX
   GLint i0 = VB->IndexPtr->data[vert0] << 8;
   GLint di = (GLint) (VB->IndexPtr->data[vert1] << 8) - i0;
#endif
#ifdef INTERP_ST
   GLfixed s0 = FloatToFixed(VB->TexCoord[vert0][0] * S_SCALE);
   GLfixed ds = FloatToFixed(VB->TexCoord[vert1][0] * S_SCALE) - s0;
   GLfixed t0 = FloatToFixed(VB->TexCoord[vert0][1] * T_SCALE);
   GLfixed dt = FloatToFixed(VB->TexCoord[vert1][1] * T_SCALE) - t0;
#endif
#if defined(INTERP_STUV0) || defined(INTERP_STUV1)
   GLfloat invw0 = VB->Win.data[vert0][3];
   GLfloat invw1 = VB->Win.data[vert1][3];
#endif
#ifdef INTERP_STUV0
   /* h denotes hyperbolic */
   GLfloat hs0 = invw0 * VB->TexCoordPtr[0]->data[vert0][0];
   GLfloat dhs = invw1 * VB->TexCoordPtr[0]->data[vert1][0] - hs0;
   GLfloat ht0 = invw0 * VB->TexCoordPtr[0]->data[vert0][1];
   GLfloat dht = invw1 * VB->TexCoordPtr[0]->data[vert1][1] - ht0;
   GLfloat hu0 = 0, dhu = 0;
   GLfloat hv0 = invw0, dhv = invw1 - invw0;
#endif
#ifdef INTERP_STUV1
   GLfloat hs01 = invw0 * VB->TexCoordPtr[1]->data[vert0][0];
   GLfloat dhs1 = invw1 * VB->TexCoordPtr[1]->data[vert1][0] - hs01;
   GLfloat ht01 = invw0 * VB->TexCoordPtr[1]->data[vert0][1];
   GLfloat dht1 = invw1 * VB->TexCoordPtr[1]->data[vert1][1] - ht01;
   GLfloat hu01 = 0, dhu1 = 0;
   GLfloat hv01 = invw0, dhv1 = invw1 - invw0;
#endif
#ifdef PIXEL_ADDRESS
   PIXEL_TYPE *pixelPtr;
   GLint pixelXstep, pixelYstep;
#endif
#ifdef WIDE
   /* for wide lines, draw all X in [x+min, x+max] or Y in [y+min, y+max] */
   GLint width, min, max;
   width = (GLint) CLAMP( ctx->Line.Width, MIN_LINE_WIDTH, MAX_LINE_WIDTH );
   min = (width-1) / -2;
   max = min + width - 1;
#endif
#ifdef INTERP_STUV0
   if (VB->TexCoordPtr[0]->size > 2) {
      hu0 = invw0 * VB->TexCoordPtr[0]->data[vert0][2];
      dhu = invw1 * VB->TexCoordPtr[0]->data[vert1][2] - hu0;
      if (VB->TexCoordPtr[0]->size > 3) {
	 hv0 = invw0 * VB->TexCoordPtr[0]->data[vert0][3];
	 dhv = invw1 * VB->TexCoordPtr[0]->data[vert1][3] - hv0;
      }
   }
#endif
#ifdef INTERP_STUV1
   if (VB->TexCoordPtr[1]->size > 2) {
      hu01 = invw0 * VB->TexCoordPtr[1]->data[vert0][2];
      dhu1 = invw1 * VB->TexCoordPtr[1]->data[vert1][2] - hu01;
      if (VB->TexCoordPtr[1]->size > 3) {
	 hv01 = invw0 * VB->TexCoordPtr[1]->data[vert0][3];
	 dhv1 = invw1 * VB->TexCoordPtr[1]->data[vert1][3] - hv01;
      }
   }
#endif

/*
 * Despite being clipped to the view volume, the line's window coordinates
 * may just lie outside the window bounds.  That is, if the legal window
 * coordinates are [0,W-1][0,H-1], it's possible for x==W and/or y==H.
 * This quick and dirty code nudges the endpoints inside the window if
 * necessary.
 */
#ifdef CLIP_HACK
   {
      GLint w = ctx->DrawBuffer->Width;
      GLint h = ctx->DrawBuffer->Height;
      if ((x0==w) | (x1==w)) {
         if ((x0==w) & (x1==w))
           return;
         x0 -= x0==w;
         x1 -= x1==w;
      }
      if ((y0==h) | (y1==h)) {
         if ((y0==h) & (y1==h))
           return;
         y0 -= y0==h;
         y1 -= y1==h;
      }
   }
#endif
   dx = x1 - x0;
   dy = y1 - y0;
   if (dx==0 && dy==0) {
      return;
   }

   /*
    * Setup
    */
#ifdef SETUP_CODE
   SETUP_CODE
#endif

#ifdef INTERP_Z
#  ifdef DEPTH_TYPE
     zPtr = (DEPTH_TYPE *) _mesa_zbuffer_address(ctx, x0, y0);
#  endif
   if (depthBits <= 16) {
      z0 = FloatToFixed(VB->Win.data[vert0][2] + ctx->LineZoffset);
      z1 = FloatToFixed(VB->Win.data[vert1][2] + ctx->LineZoffset);
   }
   else {
      z0 = (GLint) (VB->Win.data[vert0][2] + ctx->LineZoffset);
      z1 = (GLint) (VB->Win.data[vert1][2] + ctx->LineZoffset);
   }
#endif
#ifdef PIXEL_ADDRESS
   pixelPtr = (PIXEL_TYPE *) PIXEL_ADDRESS(x0,y0);
#endif

   if (dx<0) {
      dx = -dx;   /* make positive */
#ifdef INTERP_XY
      xstep = -1;
#endif
#if defined(INTERP_Z) && defined(DEPTH_TYPE)
      zPtrXstep = -((GLint)sizeof(DEPTH_TYPE));
#endif
#ifdef PIXEL_ADDRESS
      pixelXstep = -((GLint)sizeof(PIXEL_TYPE));
#endif
   }
   else {
#ifdef INTERP_XY
      xstep = 1;
#endif
#if defined(INTERP_Z) && defined(DEPTH_TYPE)
      zPtrXstep = ((GLint)sizeof(DEPTH_TYPE));
#endif
#ifdef PIXEL_ADDRESS
      pixelXstep = ((GLint)sizeof(PIXEL_TYPE));
#endif
   }

   if (dy<0) {
      dy = -dy;   /* make positive */
#ifdef INTERP_XY
      ystep = -1;
#endif
#if defined(INTERP_Z) && defined(DEPTH_TYPE)
      zPtrYstep = -ctx->DrawBuffer->Width * ((GLint)sizeof(DEPTH_TYPE));
#endif
#ifdef PIXEL_ADDRESS
      pixelYstep = BYTES_PER_ROW;
#endif
   }
   else {
#ifdef INTERP_XY
      ystep = 1;
#endif
#if defined(INTERP_Z) && defined(DEPTH_TYPE)
      zPtrYstep = ctx->DrawBuffer->Width * ((GLint)sizeof(DEPTH_TYPE));
#endif
#ifdef PIXEL_ADDRESS
      pixelYstep = -(BYTES_PER_ROW);
#endif
   }

   /*
    * Draw
    */

   if (dx>dy) {
      /*** X-major line ***/
      GLint i;
      GLint errorInc = dy+dy;
      GLint error = errorInc-dx;
      GLint errorDec = error-dx;
#ifdef INTERP_Z
      dz = (z1-z0) / dx;
#endif
#ifdef INTERP_RGB
      dr /= dx;   /* convert from whole line delta to per-pixel delta */
      dg /= dx;
      db /= dx;
#endif
#ifdef INTERP_SPEC
      dsr /= dx;   /* convert from whole line delta to per-pixel delta */
      dsg /= dx;
      dsb /= dx;
#endif
#ifdef INTERP_ALPHA
      da /= dx;
#endif
#ifdef INTERP_INDEX
      di /= dx;
#endif
#ifdef INTERP_ST
      ds /= dx;
      dt /= dx;
#endif
#ifdef INTERP_STUV0
      {
         GLfloat invDx = 1.0F / (GLfloat) dx;
         dhs *= invDx;
         dht *= invDx;
         dhu *= invDx;
         dhv *= invDx;
      }
#endif
#ifdef INTERP_STUV1
      {
         GLfloat invDx = 1.0F / (GLfloat) dx;
         dhs1 *= invDx;
         dht1 *= invDx;
         dhu1 *= invDx;
         dhv1 *= invDx;
      }
#endif
      for (i=0;i<dx;i++) {
#ifdef STIPPLE
         GLushort m;
         m = 1 << ((ctx->StippleCounter/ctx->Line.StippleFactor) & 0xf);
         if (ctx->Line.StipplePattern & m) {
#endif
#ifdef INTERP_Z
            GLdepth Z = FixedToDepth(z0);
#endif
#ifdef INTERP_INDEX
            GLint I = i0 >> 8;
#endif
#ifdef INTERP_STUV0
            GLfloat invQ = 1.0F / hv0;
            GLfloat s = hs0 * invQ;
            GLfloat t = ht0 * invQ;
            GLfloat u = hu0 * invQ;
#endif
#ifdef INTERP_STUV1
            GLfloat invQ1 = 1.0F / hv01;
            GLfloat s1 = hs01 * invQ1;
            GLfloat t1 = ht01 * invQ1;
            GLfloat u1 = hu01 * invQ1;
#endif
#ifdef WIDE
            GLint yy;
            GLint ymin = y0 + min;
            GLint ymax = y0 + max;
            for (yy=ymin;yy<=ymax;yy++) {
               PLOT( x0, yy );
            }
#else
#  ifdef XMAJOR_PLOT
            XMAJOR_PLOT( x0, y0 );
#  else
            PLOT( x0, y0 );
#  endif
#endif /*WIDE*/
#ifdef STIPPLE
        }
	ctx->StippleCounter++;
#endif
#ifdef INTERP_XY
         x0 += xstep;
#endif
#ifdef INTERP_Z
#  ifdef DEPTH_TYPE
         zPtr = (DEPTH_TYPE *) ((GLubyte*) zPtr + zPtrXstep);
#  endif
         z0 += dz;
#endif
#ifdef INTERP_RGB
         r0 += dr;
         g0 += dg;
         b0 += db;
#endif
#ifdef INTERP_SPEC
         sr0 += dsr;
         sg0 += dsg;
         sb0 += dsb;
#endif
#ifdef INTERP_ALPHA
         a0 += da;
#endif
#ifdef INTERP_INDEX
         i0 += di;
#endif
#ifdef INTERP_ST
         s0 += ds;
         t0 += dt;
#endif
#ifdef INTERP_STUV0
         hs0 += dhs;
         ht0 += dht;
         hu0 += dhu;
         hv0 += dhv;
#endif
#ifdef INTERP_STUV1
         hs01 += dhs1;
         ht01 += dht1;
         hu01 += dhu1;
         hv01 += dhv1;
#endif
#ifdef PIXEL_ADDRESS
         pixelPtr = (PIXEL_TYPE*) ((GLubyte*) pixelPtr + pixelXstep);
#endif
         if (error<0) {
            error += errorInc;
         }
         else {
            error += errorDec;
#ifdef INTERP_XY
            y0 += ystep;
#endif
#if defined(INTERP_Z) && defined(DEPTH_TYPE)
            zPtr = (DEPTH_TYPE *) ((GLubyte*) zPtr + zPtrYstep);
#endif
#ifdef PIXEL_ADDRESS
            pixelPtr = (PIXEL_TYPE*) ((GLubyte*) pixelPtr + pixelYstep);
#endif
         }
      }
   }
   else {
      /*** Y-major line ***/
      GLint i;
      GLint errorInc = dx+dx;
      GLint error = errorInc-dy;
      GLint errorDec = error-dy;
#ifdef INTERP_Z
      dz = (z1-z0) / dy;
#endif
#ifdef INTERP_RGB
      dr /= dy;   /* convert from whole line delta to per-pixel delta */
      dg /= dy;
      db /= dy;
#endif
#ifdef INTERP_SPEC
      dsr /= dy;   /* convert from whole line delta to per-pixel delta */
      dsg /= dy;
      dsb /= dy;
#endif
#ifdef INTERP_ALPHA
      da /= dy;
#endif
#ifdef INTERP_INDEX
      di /= dy;
#endif
#ifdef INTERP_ST
      ds /= dy;
      dt /= dy;
#endif
#ifdef INTERP_STUV0
      {
         GLfloat invDy = 1.0F / (GLfloat) dy;
         dhs *= invDy;
         dht *= invDy;
         dhu *= invDy;
         dhv *= invDy;
      }
#endif
#ifdef INTERP_STUV1
      {
         GLfloat invDy = 1.0F / (GLfloat) dy;
         dhs1 *= invDy;
         dht1 *= invDy;
         dhu1 *= invDy;
         dhv1 *= invDy;
      }
#endif
      for (i=0;i<dy;i++) {
#ifdef STIPPLE
         GLushort m;
         m = 1 << ((ctx->StippleCounter/ctx->Line.StippleFactor) & 0xf);
         if (ctx->Line.StipplePattern & m) {
#endif
#ifdef INTERP_Z
            GLdepth Z = FixedToDepth(z0);
#endif
#ifdef INTERP_INDEX
            GLint I = i0 >> 8;
#endif
#ifdef INTERP_STUV0
            GLfloat invQ = 1.0F / hv0;
            GLfloat s = hs0 * invQ;
            GLfloat t = ht0 * invQ;
            GLfloat u = hu0 * invQ;
#endif
#ifdef INTERP_STUV1
            GLfloat invQ1 = 1.0F / hv01;
            GLfloat s1 = hs01 * invQ1;
            GLfloat t1 = ht01 * invQ1;
            GLfloat u1 = hu01 * invQ1;
#endif
#ifdef WIDE
            GLint xx;
            GLint xmin = x0 + min;
            GLint xmax = x0 + max;
            for (xx=xmin;xx<=xmax;xx++) {
               PLOT( xx, y0 );
            }
#else
#  ifdef YMAJOR_PLOT
            YMAJOR_PLOT( x0, y0 );
#  else
            PLOT( x0, y0 );
#  endif
#endif /*WIDE*/
#ifdef STIPPLE
        }
	ctx->StippleCounter++;
#endif
#ifdef INTERP_XY
         y0 += ystep;
#endif
#ifdef INTERP_Z
#  ifdef DEPTH_TYPE
         zPtr = (DEPTH_TYPE *) ((GLubyte*) zPtr + zPtrYstep);
#  endif
         z0 += dz;
#endif
#ifdef INTERP_RGB
         r0 += dr;
         g0 += dg;
         b0 += db;
#endif
#ifdef INTERP_SPEC
         sr0 += dsr;
         sg0 += dsg;
         sb0 += dsb;
#endif
#ifdef INTERP_ALPHA
         a0 += da;
#endif
#ifdef INTERP_INDEX
         i0 += di;
#endif
#ifdef INTERP_ST
         s0 += ds;
         t0 += dt;
#endif
#ifdef INTERP_STUV0
         hs0 += dhs;
         ht0 += dht;
         hu0 += dhu;
         hv0 += dhv;
#endif
#ifdef INTERP_STUV1
         hs01 += dhs1;
         ht01 += dht1;
         hu01 += dhu1;
         hv01 += dhv1;
#endif
#ifdef PIXEL_ADDRESS
         pixelPtr = (PIXEL_TYPE*) ((GLubyte*) pixelPtr + pixelYstep);
#endif
         if (error<0) {
            error += errorInc;
         }
         else {
            error += errorDec;
#ifdef INTERP_XY
            x0 += xstep;
#endif
#if defined(INTERP_Z) && defined(DEPTH_TYPE)
            zPtr = (DEPTH_TYPE *) ((GLubyte*) zPtr + zPtrXstep);
#endif
#ifdef PIXEL_ADDRESS
            pixelPtr = (PIXEL_TYPE*) ((GLubyte*) pixelPtr + pixelXstep);
#endif
         }
      }
   }

}


#undef INTERP_XY
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_SPEC
#undef INTERP_ALPHA
#undef INTERP_STUV0
#undef INTERP_STUV1
#undef INTERP_INDEX
#undef PIXEL_ADDRESS
#undef PIXEL_TYPE
#undef DEPTH_TYPE
#undef BYTES_PER_ROW
#undef SETUP_CODE
#undef PLOT
#undef XMAJOR_PLOT
#undef YMAJOR_PLOT
#undef CLIP_HACK
#undef STIPPLE
#undef WIDE
#undef FixedToDepth
