
/*
 * Mesa 3-D graphics library
 * Version:  3.3
 * 
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
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
 * pixel span rasterization:
 * These functions implement the rasterization pipeline.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "alpha.h"
#include "alphabuf.h"
#include "blend.h"
#include "depth.h"
#include "fog.h"
#include "logic.h"
#include "macros.h"
#include "masking.h"
#include "mem.h"
#include "scissor.h"
#include "span.h"
#include "stencil.h"
#include "texture.h"
#include "types.h"
#endif




/*
 * Apply the current polygon stipple pattern to a span of pixels.
 */
static void stipple_polygon_span( GLcontext *ctx,
                                  GLuint n, GLint x, GLint y, GLubyte mask[] )
{
   register GLuint i, m, stipple, highbit=0x80000000;

   stipple = ctx->PolygonStipple[y % 32];
   m = highbit >> (GLuint) (x % 32);

   for (i=0;i<n;i++) {
      if ((m & stipple)==0) {
	 mask[i] = 0;
      }
      m = m >> 1;
      if (m==0) {
	 m = 0x80000000;
      }
   }
}



/*
 * Clip a pixel span to the current buffer/window boundaries.
 * Return:  0 = all pixels clipped
 *          1 = at least one pixel is visible
 */
static GLuint clip_span( GLcontext *ctx,
                         GLint n, GLint x, GLint y, GLubyte mask[] )
{
   GLint i;

   /* Clip to top and bottom */
   if (y < 0 || y >= ctx->DrawBuffer->Height) {
      return 0;
   }

   /* Clip to left and right */
   if (x >= 0 && x + n <= ctx->DrawBuffer->Width) {
      /* no clipping needed */
      return 1;
   }
   else if (x + n <= 0) {
      /* completely off left side */
      return 0;
   }
   else if (x >= ctx->DrawBuffer->Width) {
      /* completely off right side */
      return 0;
   }
   else {
      /* clip-test each pixel, this could be done better */
      for (i=0;i<n;i++) {
         if (x + i < 0 || x + i >= ctx->DrawBuffer->Width) {
            mask[i] = 0;
         }
      }
      return 1;
   }
}



/*
 * Draw to more than one color buffer (or none).
 */
static void multi_write_index_span( GLcontext *ctx, GLuint n,
                                    GLint x, GLint y, const GLuint indexes[],
                                    const GLubyte mask[] )
{
   GLuint bufferBit;

   if (ctx->Color.DrawBuffer == GL_NONE)
      return;

   /* loop over four possible dest color buffers */
   for (bufferBit = 1; bufferBit <= 8; bufferBit = bufferBit << 1) {
      if (bufferBit & ctx->Color.DrawDestMask) {
         GLuint indexTmp[MAX_WIDTH];
         ASSERT(n < MAX_WIDTH);

         if (bufferBit == FRONT_LEFT_BIT)
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_LEFT);
         else if (bufferBit == FRONT_RIGHT_BIT)
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_RIGHT);
         else if (bufferBit == BACK_LEFT_BIT)
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_LEFT);
         else
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_RIGHT);

         /* make copy of incoming indexes */
         MEMCPY( indexTmp, indexes, n * sizeof(GLuint) );
         if (ctx->Color.SWLogicOpEnabled) {
            _mesa_logicop_ci_span( ctx, n, x, y, indexTmp, mask );
         }
         if (ctx->Color.SWmasking) {
            if (ctx->Color.IndexMask == 0)
               break;
            _mesa_mask_index_span( ctx, n, x, y, indexTmp );
         }
         (*ctx->Driver.WriteCI32Span)( ctx, n, x, y, indexTmp, mask );
      }
   }

   /* restore default dest buffer */
   (void) (*ctx->Driver.SetDrawBuffer)( ctx, ctx->Color.DriverDrawBuffer);
}



/*
 * Write a horizontal span of color index pixels to the frame buffer.
 * Stenciling, Depth-testing, etc. are done as needed.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         z - array of [n] z-values
 *         index - array of [n] color indexes
 *         primitive - either GL_POINT, GL_LINE, GL_POLYGON, or GL_BITMAP
 */
void gl_write_index_span( GLcontext *ctx,
                          GLuint n, GLint x, GLint y, const GLdepth z[],
			  GLuint indexIn[], GLenum primitive )
{
   const GLuint modBits = FOG_BIT | BLEND_BIT | MASKING_BIT | LOGIC_OP_BIT;
   GLubyte mask[MAX_WIDTH];
   GLuint indexBackup[MAX_WIDTH];
   GLuint *index;  /* points to indexIn or indexBackup */

   /* init mask to 1's (all pixels are to be written) */
   MEMSET(mask, 1, n);

   if ((ctx->RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(ctx,n,x,y,mask)==0) {
	 return;
      }
   }

   if ((primitive==GL_BITMAP && (ctx->RasterMask & modBits))
       || (ctx->RasterMask & MULTI_DRAW_BIT)) {
      /* Make copy of color indexes */
      MEMCPY( indexBackup, indexIn, n * sizeof(GLuint) );
      index = indexBackup;
   }
   else {
      index = indexIn;
   }

   /* Per-pixel fog */
   if (ctx->Fog.Enabled && (primitive==GL_BITMAP || ctx->FogMode == FOG_FRAGMENT)) {
      _mesa_fog_ci_pixels( ctx, n, z, index );
   }

   /* Do the scissor test */
   if (ctx->Scissor.Enabled) {
      if (gl_scissor_span( ctx, n, x, y, mask )==0) {
	 return;
      }
   }

   /* Polygon Stippling */
   if (ctx->Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( ctx, n, x, y, mask );
   }

   if (ctx->Stencil.Enabled) {
      /* first stencil test */
      if (_mesa_stencil_and_ztest_span(ctx, n, x, y, z, mask) == GL_FALSE) {
	 return;
      }
   }
   else if (ctx->Depth.Test) {
      /* regular depth testing */
      if (_mesa_depth_test_span( ctx, n, x, y, z, mask )==0)  return;
   }

   /* if we get here, something passed the depth test */
   ctx->OcclusionResult = GL_TRUE;

   if (ctx->RasterMask & MULTI_DRAW_BIT) {
      /* draw to zero or two or more buffers */
      multi_write_index_span( ctx, n, x, y, index, mask );
   }
   else {
      /* normal situation: draw to exactly one buffer */
      if (ctx->Color.SWLogicOpEnabled) {
         _mesa_logicop_ci_span( ctx, n, x, y, index, mask );
      }
      if (ctx->Color.SWmasking) {
         if (ctx->Color.IndexMask == 0)
            return;
         _mesa_mask_index_span( ctx, n, x, y, index );
      }

      /* write pixels */
      (*ctx->Driver.WriteCI32Span)( ctx, n, x, y, index, mask );
   }
}




void gl_write_monoindex_span( GLcontext *ctx,
                              GLuint n, GLint x, GLint y, const GLdepth z[],
			      GLuint index, GLenum primitive )
{
   GLubyte mask[MAX_WIDTH];
   GLuint i;

   /* init mask to 1's (all pixels are to be written) */
   MEMSET(mask, 1, n);

   if ((ctx->RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span( ctx, n, x, y, mask)==0) {
	 return;
      }
   }

   /* Do the scissor test */
   if (ctx->Scissor.Enabled) {
      if (gl_scissor_span( ctx, n, x, y, mask )==0) {
	 return;
      }
   }

   /* Polygon Stippling */
   if (ctx->Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( ctx, n, x, y, mask );
   }

   if (ctx->Stencil.Enabled) {
      /* first stencil test */
      if (_mesa_stencil_and_ztest_span(ctx, n, x, y, z, mask) == GL_FALSE) {
	 return;
      }
   }
   else if (ctx->Depth.Test) {
      /* regular depth testing */
      if (_mesa_depth_test_span( ctx, n, x, y, z, mask )==0)  return;
   }

   /* if we get here, something passed the depth test */
   ctx->OcclusionResult = GL_TRUE;

   if (ctx->Color.DrawBuffer == GL_NONE) {
      /* write no pixels */
      return;
   }

   if ((ctx->Fog.Enabled && (primitive==GL_BITMAP || ctx->FogMode==FOG_FRAGMENT))
        || ctx->Color.SWLogicOpEnabled || ctx->Color.SWmasking) {
      /* different index per pixel */
      GLuint indexes[MAX_WIDTH];
      for (i=0;i<n;i++) {
	 indexes[i] = index;
      }

      if (ctx->Fog.Enabled && (primitive==GL_BITMAP || ctx->FogMode==FOG_FRAGMENT)) {
	 _mesa_fog_ci_pixels( ctx, n, z, indexes );
      }

      if (ctx->Color.SWLogicOpEnabled) {
	 _mesa_logicop_ci_span( ctx, n, x, y, indexes, mask );
      }

      if (ctx->RasterMask & MULTI_DRAW_BIT) {
         /* draw to zero or two or more buffers */
         multi_write_index_span( ctx, n, x, y, indexes, mask );
      }
      else {
         /* normal situation: draw to exactly one buffer */
         if (ctx->Color.SWLogicOpEnabled) {
            _mesa_logicop_ci_span( ctx, n, x, y, indexes, mask );
         }
         if (ctx->Color.SWmasking) {
            if (ctx->Color.IndexMask == 0)
               return;
            _mesa_mask_index_span( ctx, n, x, y, indexes );
         }
         (*ctx->Driver.WriteCI32Span)( ctx, n, x, y, indexes, mask );
      }
   }
   else {
      /* same color index for all pixels */
      ASSERT(!ctx->Color.SWLogicOpEnabled);
      ASSERT(!ctx->Color.SWmasking);
      if (ctx->RasterMask & MULTI_DRAW_BIT) {
         /* draw to zero or two or more buffers */
         GLuint indexes[MAX_WIDTH];
         for (i=0;i<n;i++)
            indexes[i] = index;
         multi_write_index_span( ctx, n, x, y, indexes, mask );
      }
      else {
         /* normal situation: draw to exactly one buffer */
         (*ctx->Driver.WriteMonoCISpan)( ctx, n, x, y, mask );
      }
   }
}



/*
 * Draw to more than one RGBA color buffer (or none).
 */
static void multi_write_rgba_span( GLcontext *ctx, GLuint n,
                                   GLint x, GLint y, CONST GLubyte rgba[][4],
                                   const GLubyte mask[] )
{
   GLuint bufferBit;

   if (ctx->Color.DrawBuffer == GL_NONE)
      return;

   /* loop over four possible dest color buffers */
   for (bufferBit = 1; bufferBit <= 8; bufferBit = bufferBit << 1) {
      if (bufferBit & ctx->Color.DrawDestMask) {
         GLubyte rgbaTmp[MAX_WIDTH][4];
         ASSERT(n < MAX_WIDTH);

         if (bufferBit == FRONT_LEFT_BIT) {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_LEFT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->FrontLeftAlpha;
         }
         else if (bufferBit == FRONT_RIGHT_BIT) {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_RIGHT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->FrontRightAlpha;
         }
         else if (bufferBit == BACK_LEFT_BIT) {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_LEFT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->BackLeftAlpha;
         }
         else {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_RIGHT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->BackRightAlpha;
         }

         /* make copy of incoming colors */
         MEMCPY( rgbaTmp, rgba, 4 * n * sizeof(GLubyte) );

         if (ctx->Color.SWLogicOpEnabled) {
            _mesa_logicop_rgba_span( ctx, n, x, y, rgbaTmp, mask );
         }
         else if (ctx->Color.BlendEnabled) {
            _mesa_blend_span( ctx, n, x, y, rgbaTmp, mask );
         }
         if (ctx->Color.SWmasking) {
            if (*((GLuint *) ctx->Color.ColorMask) == 0)
               break;
            _mesa_mask_rgba_span( ctx, n, x, y, rgbaTmp );
         }

         (*ctx->Driver.WriteRGBASpan)( ctx, n, x, y, 
				       (CONST GLubyte (*)[4]) rgbaTmp, mask );
         if (ctx->RasterMask & ALPHABUF_BIT) {
            _mesa_write_alpha_span( ctx, n, x, y, 
                                    (CONST GLubyte (*)[4])rgbaTmp, mask );
         }
      }
   }

   /* restore default dest buffer */
   (void) (*ctx->Driver.SetDrawBuffer)( ctx, ctx->Color.DriverDrawBuffer );
}



void gl_write_rgba_span( GLcontext *ctx,
                         GLuint n, GLint x, GLint y, const GLdepth z[],
                         GLubyte rgbaIn[][4],
                         GLenum primitive )
{
   const GLuint modBits = FOG_BIT | BLEND_BIT | MASKING_BIT |
                          LOGIC_OP_BIT | TEXTURE_BIT;
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;
   GLubyte rgbaBackup[MAX_WIDTH][4];
   GLubyte (*rgba)[4];
   const GLubyte *Null = 0;

   /* init mask to 1's (all pixels are to be written) */
   MEMSET(mask, 1, n);

   if ((ctx->RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span( ctx,n,x,y,mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if ((primitive==GL_BITMAP && (ctx->RasterMask & modBits))
       || (ctx->RasterMask & MULTI_DRAW_BIT)) {
      /* must make a copy of the colors since they may be modified */
      MEMCPY( rgbaBackup, rgbaIn, 4 * n * sizeof(GLubyte) );
      rgba = rgbaBackup;
   }
   else {
      rgba = rgbaIn;
   }

   /* Per-pixel fog */
   if (ctx->Fog.Enabled && (primitive==GL_BITMAP || ctx->FogMode==FOG_FRAGMENT)) {
      _mesa_fog_rgba_pixels( ctx, n, z, rgba );
   }

   /* Do the scissor test */
   if (ctx->Scissor.Enabled) {
      if (gl_scissor_span( ctx, n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (ctx->Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( ctx, n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (ctx->Color.AlphaEnabled) {
      if (_mesa_alpha_test( ctx, n, (CONST GLubyte (*)[4]) rgba, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (ctx->Stencil.Enabled) {
      /* first stencil test */
      if (_mesa_stencil_and_ztest_span(ctx, n, x, y, z, mask) == GL_FALSE) {
	 return;
      }
      write_all = GL_FALSE;
   }
   else if (ctx->Depth.Test) {
      /* regular depth testing */
      GLuint m = _mesa_depth_test_span( ctx, n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   /* if we get here, something passed the depth test */
   ctx->OcclusionResult = GL_TRUE;

   if (ctx->RasterMask & MULTI_DRAW_BIT) {
      multi_write_rgba_span( ctx, n, x, y, (CONST GLubyte (*)[4]) rgba, mask );
   }
   else {
      /* normal: write to exactly one buffer */
      /* logic op or blending */
      if (ctx->Color.SWLogicOpEnabled) {
         _mesa_logicop_rgba_span( ctx, n, x, y, rgba, mask );
      }
      else if (ctx->Color.BlendEnabled) {
         _mesa_blend_span( ctx, n, x, y, rgba, mask );
      }

      /* Color component masking */
      if (ctx->Color.SWmasking) {
         if (*((GLuint *) ctx->Color.ColorMask) == 0)
            return;
         _mesa_mask_rgba_span( ctx, n, x, y, rgba );
      }

      /* write pixels */
      (*ctx->Driver.WriteRGBASpan)( ctx, n, x, y, 
				    (CONST GLubyte (*)[4]) rgba, 
				    write_all ? Null : mask );

      if (ctx->RasterMask & ALPHABUF_BIT) {
         _mesa_write_alpha_span( ctx, n, x, y, 
                                 (CONST GLubyte (*)[4]) rgba, 
                                 write_all ? Null : mask );
      }

   }
}



/*
 * Write a horizontal span of color pixels to the frame buffer.
 * The color is initially constant for the whole span.
 * Alpha-testing, stenciling, depth-testing, and blending are done as needed.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         z - array of [n] z-values
 *         r, g, b, a - the color of the pixels
 *         primitive - either GL_POINT, GL_LINE, GL_POLYGON or GL_BITMAP.
 */
void gl_write_monocolor_span( GLcontext *ctx,
                              GLuint n, GLint x, GLint y, const GLdepth z[],
			      const GLubyte color[4],
                              GLenum primitive )
{
   GLuint i;
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;
   GLubyte rgba[MAX_WIDTH][4];
   const GLubyte *Null = 0;

   /* init mask to 1's (all pixels are to be written) */
   MEMSET(mask, 1, n);

   if ((ctx->RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span( ctx,n,x,y,mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Do the scissor test */
   if (ctx->Scissor.Enabled) {
      if (gl_scissor_span( ctx, n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (ctx->Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( ctx, n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (ctx->Color.AlphaEnabled) {
      for (i=0;i<n;i++) {
         rgba[i][ACOMP] = color[ACOMP];
      }
      if (_mesa_alpha_test( ctx, n, (CONST GLubyte (*)[4])rgba, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (ctx->Stencil.Enabled) {
      /* first stencil test */
      if (_mesa_stencil_and_ztest_span(ctx, n, x, y, z, mask) == GL_FALSE) {
	 return;
      }
      write_all = GL_FALSE;
   }
   else if (ctx->Depth.Test) {
      /* regular depth testing */
      GLuint m = _mesa_depth_test_span( ctx, n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   /* if we get here, something passed the depth test */
   ctx->OcclusionResult = GL_TRUE;

   if (ctx->Color.DrawBuffer == GL_NONE) {
      /* write no pixels */
      return;
   }

   if (ctx->Color.SWLogicOpEnabled || ctx->Color.SWmasking ||
       (ctx->RasterMask & (BLEND_BIT | FOG_BIT))) {
      /* assign same color to each pixel */
      for (i=0;i<n;i++) {
	 if (mask[i]) {
            COPY_4UBV(rgba[i], color);
	 }
      }

      /* Per-pixel fog */
      if (ctx->Fog.Enabled &&
          (primitive==GL_BITMAP || ctx->FogMode==FOG_FRAGMENT)) {
         _mesa_fog_rgba_pixels( ctx, n, z, rgba );
      }

      if (ctx->RasterMask & MULTI_DRAW_BIT) {
         multi_write_rgba_span( ctx, n, x, y,
                                (CONST GLubyte (*)[4]) rgba, mask );
      }
      else {
         /* normal: write to exactly one buffer */
         if (ctx->Color.SWLogicOpEnabled) {
            _mesa_logicop_rgba_span( ctx, n, x, y, rgba, mask );
         }
         else if (ctx->Color.BlendEnabled) {
            _mesa_blend_span( ctx, n, x, y, rgba, mask );
         }

         /* Color component masking */
         if (ctx->Color.SWmasking) {
            if (*((GLuint *) ctx->Color.ColorMask) == 0)
               return;
            _mesa_mask_rgba_span( ctx, n, x, y, rgba );
         }

         /* write pixels */
         (*ctx->Driver.WriteRGBASpan)( ctx, n, x, y, 
				       (CONST GLubyte (*)[4]) rgba, 
				       write_all ? Null : mask );
         if (ctx->RasterMask & ALPHABUF_BIT) {
            _mesa_write_alpha_span( ctx, n, x, y, 
                                    (CONST GLubyte (*)[4]) rgba, 
                                    write_all ? Null : mask );
         }
      }
   }
   else {
      /* same color for all pixels */
      ASSERT(!ctx->Color.BlendEnabled);
      ASSERT(!ctx->Color.SWLogicOpEnabled);
      ASSERT(!ctx->Color.SWmasking);

      if (ctx->RasterMask & MULTI_DRAW_BIT) {
         for (i=0;i<n;i++) {
            if (mask[i]) {
               COPY_4UBV(rgba[i], color);
            }
         }
         multi_write_rgba_span( ctx, n, x, y, 
				(CONST GLubyte (*)[4]) rgba, mask );
      }
      else {
         (*ctx->Driver.WriteMonoRGBASpan)( ctx, n, x, y, mask );
         if (ctx->RasterMask & ALPHABUF_BIT) {
            _mesa_write_mono_alpha_span( ctx, n, x, y, (GLubyte) color[ACOMP],
                                         write_all ? Null : mask );
         }
      }
   }
}



/*
 * Add specular color to base color.  This is used only when
 * GL_LIGHT_MODEL_COLOR_CONTROL = GL_SEPARATE_SPECULAR_COLOR.
 */
static void add_colors(GLuint n, GLubyte rgba[][4], CONST GLubyte specular[][4] )
{
   GLuint i;
   for (i=0; i<n; i++) {
      GLint r = rgba[i][RCOMP] + specular[i][RCOMP];
      GLint g = rgba[i][GCOMP] + specular[i][GCOMP];
      GLint b = rgba[i][BCOMP] + specular[i][BCOMP];
      rgba[i][RCOMP] = (GLubyte) MIN2(r, 255);
      rgba[i][GCOMP] = (GLubyte) MIN2(g, 255);
      rgba[i][BCOMP] = (GLubyte) MIN2(b, 255);
   }
}


/*
 * Write a horizontal span of textured pixels to the frame buffer.
 * The color of each pixel is different.
 * Alpha-testing, stenciling, depth-testing, and blending are done
 * as needed.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         z - array of [n] z-values
 *         s, t - array of (s,t) texture coordinates for each pixel
 *         lambda - array of texture lambda values
 *         rgba - array of [n] color components
 *         primitive - either GL_POINT, GL_LINE, GL_POLYGON or GL_BITMAP.
 */
void gl_write_texture_span( GLcontext *ctx,
                            GLuint n, GLint x, GLint y, const GLdepth z[],
			    const GLfloat s[], const GLfloat t[],
                            const GLfloat u[], GLfloat lambda[],
			    GLubyte rgbaIn[][4], CONST GLubyte spec[][4],
			    GLenum primitive )
{
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;
   GLubyte rgbaBackup[MAX_WIDTH][4];
   GLubyte (*rgba)[4];   /* points to either rgbaIn or rgbaBackup */
   const GLubyte *Null = 0;

   /* init mask to 1's (all pixels are to be written) */
   MEMSET(mask, 1, n);

   if ((ctx->RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(ctx, n, x, y, mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }


   if (primitive==GL_BITMAP || (ctx->RasterMask & MULTI_DRAW_BIT)) {
      /* must make a copy of the colors since they may be modified */
      MEMCPY(rgbaBackup, rgbaIn, 4 * n * sizeof(GLubyte));
      rgba = rgbaBackup;
   }
   else {
      rgba = rgbaIn;
   }

   /* Texture */
   ASSERT(ctx->Texture.ReallyEnabled);
   gl_texture_pixels( ctx, 0, n, s, t, u, lambda, rgba, rgba );

   /* Add base and specular colors */
   if (spec && ctx->Light.Enabled
       && ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
      add_colors( n, rgba, spec );   /* rgba = rgba + spec */

   /* Per-pixel fog */
   if (ctx->Fog.Enabled && (primitive==GL_BITMAP || ctx->FogMode==FOG_FRAGMENT)) {
      _mesa_fog_rgba_pixels( ctx, n, z, rgba );
   }

   /* Do the scissor test */
   if (ctx->Scissor.Enabled) {
      if (gl_scissor_span( ctx, n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (ctx->Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( ctx, n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (ctx->Color.AlphaEnabled) {
      if (_mesa_alpha_test( ctx, n, (CONST GLubyte (*)[4]) rgba, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (ctx->Stencil.Enabled) {
      /* first stencil test */
      if (_mesa_stencil_and_ztest_span(ctx, n, x, y, z, mask) == GL_FALSE) {
	 return;
      }
      write_all = GL_FALSE;
   }
   else if (ctx->Depth.Test) {
      /* regular depth testing */
      GLuint m = _mesa_depth_test_span( ctx, n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   /* if we get here, something passed the depth test */
   ctx->OcclusionResult = GL_TRUE;

   if (ctx->RasterMask & MULTI_DRAW_BIT) {
      multi_write_rgba_span( ctx, n, x, y, (CONST GLubyte (*)[4]) rgba, mask );
   }
   else {
      /* normal: write to exactly one buffer */
      if (ctx->Color.SWLogicOpEnabled) {
         _mesa_logicop_rgba_span( ctx, n, x, y, rgba, mask );
      }
      else  if (ctx->Color.BlendEnabled) {
         _mesa_blend_span( ctx, n, x, y, rgba, mask );
      }
      if (ctx->Color.SWmasking) {
         if (*((GLuint *) ctx->Color.ColorMask) == 0)
            return;
         _mesa_mask_rgba_span( ctx, n, x, y, rgba );
      }

      (*ctx->Driver.WriteRGBASpan)( ctx, n, x, y, (CONST GLubyte (*)[4])rgba,
				    write_all ? Null : mask );
      if (ctx->RasterMask & ALPHABUF_BIT) {
         _mesa_write_alpha_span( ctx, n, x, y, (CONST GLubyte (*)[4]) rgba, 
                                 write_all ? Null : mask );
      }
   }
}



/*
 * As above but perform multiple stages of texture application.
 * Input:  texUnits - number of texture units to apply
 */
void
gl_write_multitexture_span( GLcontext *ctx, GLuint texUnits,
                            GLuint n, GLint x, GLint y,
                            const GLdepth z[],
                            CONST GLfloat s[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            CONST GLfloat t[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            CONST GLfloat u[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            GLfloat lambda[][MAX_WIDTH],
                            GLubyte rgbaIn[MAX_TEXTURE_UNITS][4],
                            CONST GLubyte spec[MAX_TEXTURE_UNITS][4],
                            GLenum primitive )
{
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;
   GLubyte rgbaBackup[MAX_WIDTH][4];
   GLubyte (*rgba)[4];   /* points to either rgbaIn or rgbaBackup */
   GLuint i;
   const GLubyte *Null = 0;

   /* init mask to 1's (all pixels are to be written) */
   MEMSET(mask, 1, n);

   if ((ctx->RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(ctx, n, x, y, mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }


   if (primitive==GL_BITMAP || (ctx->RasterMask & MULTI_DRAW_BIT)
                            || texUnits > 1) {
      /* must make a copy of the colors since they may be modified */
      MEMCPY(rgbaBackup, rgbaIn, 4 * n * sizeof(GLubyte));
      rgba = rgbaBackup;
   }
   else {
      rgba = rgbaIn;
   }

   /* Texture */
   ASSERT(ctx->Texture.ReallyEnabled);
   ASSERT(texUnits <= MAX_TEXTURE_UNITS);
   for (i=0;i<texUnits;i++)
      gl_texture_pixels( ctx, i, n, s[i], t[i], u[i], lambda[i], rgbaIn, rgba );

   /* Add base and specular colors */
   if (spec && ctx->Light.Enabled
       && ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
      add_colors( n, rgba, spec );   /* rgba = rgba + spec */

   /* Per-pixel fog */
   if (ctx->Fog.Enabled && (primitive==GL_BITMAP || ctx->FogMode==FOG_FRAGMENT)) {
      _mesa_fog_rgba_pixels( ctx, n, z, rgba );
   }

   /* Do the scissor test */
   if (ctx->Scissor.Enabled) {
      if (gl_scissor_span( ctx, n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (ctx->Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( ctx, n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (ctx->Color.AlphaEnabled) {
      if (_mesa_alpha_test( ctx, n, (CONST GLubyte (*)[4])rgba, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (ctx->Stencil.Enabled) {
      /* first stencil test */
      if (_mesa_stencil_and_ztest_span(ctx, n, x, y, z, mask) == GL_FALSE) {
	 return;
      }
      write_all = GL_FALSE;
   }
   else if (ctx->Depth.Test) {
      /* regular depth testing */
      GLuint m = _mesa_depth_test_span( ctx, n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   /* if we get here, something passed the depth test */
   ctx->OcclusionResult = GL_TRUE;

   if (ctx->RasterMask & MULTI_DRAW_BIT) {
      multi_write_rgba_span( ctx, n, x, y, (CONST GLubyte (*)[4]) rgba, mask );
   }
   else {
      /* normal: write to exactly one buffer */

      if (ctx->Color.SWLogicOpEnabled) {
         _mesa_logicop_rgba_span( ctx, n, x, y, rgba, mask );
      }
      else  if (ctx->Color.BlendEnabled) {
         _mesa_blend_span( ctx, n, x, y, rgba, mask );
      }
      if (ctx->Color.SWmasking) {
         if (*((GLuint *) ctx->Color.ColorMask) == 0)
            return;
         _mesa_mask_rgba_span( ctx, n, x, y, rgba );
      }

      (*ctx->Driver.WriteRGBASpan)( ctx, n, x, y, (CONST GLubyte (*)[4])rgba, write_all ? Null : mask );
      if (ctx->RasterMask & ALPHABUF_BIT) {
         _mesa_write_alpha_span( ctx, n, x, y, (CONST GLubyte (*)[4])rgba,
                                 write_all ? Null : mask );
      }
   }
}



/*
 * Read RGBA pixels from frame buffer.  Clipping will be done to prevent
 * reading ouside the buffer's boundaries.
 */
void gl_read_rgba_span( GLcontext *ctx, GLframebuffer *buffer,
                        GLuint n, GLint x, GLint y,
                        GLubyte rgba[][4] )
{
   if (y < 0 || y >= buffer->Height
       || x + (GLint) n < 0 || x >= buffer->Width) {
      /* completely above, below, or right */
      /* XXX maybe leave undefined? */
      BZERO(rgba, 4 * n * sizeof(GLubyte));
   }
   else {
      GLint skip, length;
      if (x < 0) {
         /* left edge clippping */
         skip = -x;
         length = (GLint) n - skip;
         if (length < 0) {
            /* completely left of window */
            return;
         }
         if (length > buffer->Width) {
            length = buffer->Width;
         }
      }
      else if ((GLint) (x + n) > buffer->Width) {
         /* right edge clipping */
         skip = 0;
         length = buffer->Width - x;
         if (length < 0) {
            /* completely to right of window */
            return;
         }
      }
      else {
         /* no clipping */
         skip = 0;
         length = (GLint) n;
      }

      (*ctx->Driver.ReadRGBASpan)( ctx, length, x + skip, y, rgba + skip );
      if (buffer->UseSoftwareAlphaBuffers) {
         _mesa_read_alpha_span( ctx, length, x + skip, y, rgba + skip );
      }
   }
}




/*
 * Read CI pixels from frame buffer.  Clipping will be done to prevent
 * reading ouside the buffer's boundaries.
 */
void gl_read_index_span( GLcontext *ctx, GLframebuffer *buffer,
                         GLuint n, GLint x, GLint y, GLuint indx[] )
{
   if (y < 0 || y >= buffer->Height
       || x + (GLint) n < 0 || x >= buffer->Width) {
      /* completely above, below, or right */
      BZERO(indx, n * sizeof(GLuint));
   }
   else {
      GLint skip, length;
      if (x < 0) {
         /* left edge clippping */
         skip = -x;
         length = (GLint) n - skip;
         if (length < 0) {
            /* completely left of window */
            return;
         }
         if (length > buffer->Width) {
            length = buffer->Width;
         }
      }
      else if ((GLint) (x + n) > buffer->Width) {
         /* right edge clipping */
         skip = 0;
         length = buffer->Width - x;
         if (length < 0) {
            /* completely to right of window */
            return;
         }
      }
      else {
         /* no clipping */
         skip = 0;
         length = (GLint) n;
      }

      (*ctx->Driver.ReadCI32Span)( ctx, length, skip + x, y, indx + skip );
   }
}
