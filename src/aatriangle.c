/* $Id: aatriangle.c,v 1.5.4.1 2001/05/18 21:46:06 brianp Exp $ */

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
 * Antialiased Triangle rasterizers
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "aatriangle.h"
#include "span.h"
#include "types.h"
#include "vb.h"
#endif


/*
 * Compute coefficients of a plane using the X,Y coords of the v0, v1, v2
 * vertices and the given Z values.
 */
static INLINE void
compute_plane(const GLfloat v0[], const GLfloat v1[], const GLfloat v2[],
              GLfloat z0, GLfloat z1, GLfloat z2, GLfloat plane[4])
{
   const GLfloat px = v1[0] - v0[0];
   const GLfloat py = v1[1] - v0[1];
   const GLfloat pz = z1 - z0;

   const GLfloat qx = v2[0] - v0[0];
   const GLfloat qy = v2[1] - v0[1];
   const GLfloat qz = z2 - z0;

   const GLfloat a = py * qz - pz * qy;
   const GLfloat b = pz * qx - px * qz;
   const GLfloat c = px * qy - py * qx;
   const GLfloat d = -(a * v0[0] + b * v0[1] + c * z0);

   plane[0] = a;
   plane[1] = b;
   plane[2] = c;
   plane[3] = d;
}


/*
 * Compute coefficients of a plane with a constant Z value.
 */
static INLINE void
constant_plane(GLfloat value, GLfloat plane[4])
{
   plane[0] = 0.0;
   plane[1] = 0.0;
   plane[2] = -1.0;
   plane[3] = value;
}

#define CONSTANT_PLANE(VALUE, PLANE)	\
do {					\
   PLANE[0] = 0.0F;			\
   PLANE[1] = 0.0F;			\
   PLANE[2] = -1.0F;			\
   PLANE[3] = VALUE;			\
} while (0)



/*
 * Solve plane equation for Z at (X,Y).
 */
static INLINE GLfloat
solve_plane(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = (plane[3] + plane[0] * x + plane[1] * y) / -plane[2];
   return z;
}


#define SOLVE_PLANE(X, Y, PLANE) \
   ((PLANE[3] + PLANE[0] * (X) + PLANE[1] * (Y)) / -PLANE[2])


/*
 * Return 1 / solve_plane().
 */
static INLINE GLfloat
solve_plane_recip(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = -plane[2] / (plane[3] + plane[0] * x + plane[1] * y);
   return z;
}



/*
 * Solve plane and return clamped GLubyte value.
 */
static INLINE GLubyte
solve_plane_0_255(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = (plane[3] + plane[0] * x + plane[1] * y) / -plane[2] + 0.5F;
   if (z < 0.0F)
      return 0;
   else if (z > 255.0F)
      return 255;
   return (GLubyte) (GLint) z;
}



/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 1].
 */
static GLfloat
compute_coveragef(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
#define B 0.125
   static const GLfloat samples[16][2] = {
      /* start with the four corners */
      { 0.00+B, 0.00+B },
      { 0.75+B, 0.00+B },
      { 0.00+B, 0.75+B },
      { 0.75+B, 0.75+B },
      /* continue with interior samples */
      { 0.25+B, 0.00+B },
      { 0.50+B, 0.00+B },
      { 0.00+B, 0.25+B },
      { 0.25+B, 0.25+B },
      { 0.50+B, 0.25+B },
      { 0.75+B, 0.25+B },
      { 0.00+B, 0.50+B },
      { 0.25+B, 0.50+B },
      { 0.50+B, 0.50+B },
      { 0.75+B, 0.50+B },
      { 0.25+B, 0.75+B },
      { 0.50+B, 0.75+B }
   };
   const GLfloat x = (GLfloat) winx;
   const GLfloat y = (GLfloat) winy;
   const GLfloat dx0 = v1[0] - v0[0];
   const GLfloat dy0 = v1[1] - v0[1];
   const GLfloat dx1 = v2[0] - v1[0];
   const GLfloat dy1 = v2[1] - v1[1];
   const GLfloat dx2 = v0[0] - v2[0];
   const GLfloat dy2 = v0[1] - v2[1];
   GLint stop = 4, i;
   GLfloat insideCount = 16.0F;

#ifdef DEBUG
   {
      const GLfloat area = dx0 * dy1 - dx1 * dy0;
      assert(area >= 0.0);
   }
#endif

   for (i = 0; i < stop; i++) {
      const GLfloat sx = x + samples[i][0];
      const GLfloat sy = y + samples[i][1];
      const GLfloat fx0 = sx - v0[0];
      const GLfloat fy0 = sy - v0[1];
      const GLfloat fx1 = sx - v1[0];
      const GLfloat fy1 = sy - v1[1];
      const GLfloat fx2 = sx - v2[0];
      const GLfloat fy2 = sy - v2[1];
      /* cross product determines if sample is inside or outside each edge */
      GLfloat cross0 = (dx0 * fy0 - dy0 * fx0);
      GLfloat cross1 = (dx1 * fy1 - dy1 * fx1);
      GLfloat cross2 = (dx2 * fy2 - dy2 * fx2);
      /* Check if the sample is exactly on an edge.  If so, let cross be a
       * positive or negative value depending on the direction of the edge.
       */
      if (cross0 == 0.0F)
         cross0 = dx0 + dy0;
      if (cross1 == 0.0F)
         cross1 = dx1 + dy1;
      if (cross2 == 0.0F)
         cross2 = dx2 + dy2;
      if (cross0 < 0.0F || cross1 < 0.0F || cross2 < 0.0F) {
         /* point is outside triangle */
         insideCount -= 1.0F;
         stop = 16;
      }
   }
   if (stop == 4)
      return 1.0F;
   else
      return insideCount * (1.0F / 16.0F);
}



/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 15].
 */
static GLint
compute_coveragei(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
   /* NOTE: 15 samples instead of 16.
    * A better sample distribution could be used.
    */
   static const GLfloat samples[15][2] = {
      /* start with the four corners */
      { 0.00+B, 0.00+B },
      { 0.75+B, 0.00+B },
      { 0.00+B, 0.75+B },
      { 0.75+B, 0.75+B },
      /* continue with interior samples */
      { 0.25+B, 0.00+B },
      { 0.50+B, 0.00+B },
      { 0.00+B, 0.25+B },
      { 0.25+B, 0.25+B },
      { 0.50+B, 0.25+B },
      { 0.75+B, 0.25+B },
      { 0.00+B, 0.50+B },
      { 0.25+B, 0.50+B },
      /*{ 0.50, 0.50 },*/
      { 0.75+B, 0.50+B },
      { 0.25+B, 0.75+B },
      { 0.50+B, 0.75+B }
   };
   const GLfloat x = (GLfloat) winx;
   const GLfloat y = (GLfloat) winy;
   const GLfloat dx0 = v1[0] - v0[0];
   const GLfloat dy0 = v1[1] - v0[1];
   const GLfloat dx1 = v2[0] - v1[0];
   const GLfloat dy1 = v2[1] - v1[1];
   const GLfloat dx2 = v0[0] - v2[0];
   const GLfloat dy2 = v0[1] - v2[1];
   GLint stop = 4, i;
   GLint insideCount = 15;

#ifdef DEBUG
   {
      const GLfloat area = dx0 * dy1 - dx1 * dy0;
      assert(area >= 0.0);
   }
#endif

   for (i = 0; i < stop; i++) {
      const GLfloat sx = x + samples[i][0];
      const GLfloat sy = y + samples[i][1];
      const GLfloat fx0 = sx - v0[0];
      const GLfloat fy0 = sy - v0[1];
      const GLfloat fx1 = sx - v1[0];
      const GLfloat fy1 = sy - v1[1];
      const GLfloat fx2 = sx - v2[0];
      const GLfloat fy2 = sy - v2[1];
      /* cross product determines if sample is inside or outside each edge */
      GLfloat cross0 = (dx0 * fy0 - dy0 * fx0);
      GLfloat cross1 = (dx1 * fy1 - dy1 * fx1);
      GLfloat cross2 = (dx2 * fy2 - dy2 * fx2);
      /* Check if the sample is exactly on an edge.  If so, let cross be a
       * positive or negative value depending on the direction of the edge.
       */
      if (cross0 == 0.0F)
         cross0 = dx0 + dy0;
      if (cross1 == 0.0F)
         cross1 = dx1 + dy1;
      if (cross2 == 0.0F)
         cross2 = dx2 + dy2;
      if (cross0 < 0.0F || cross1 < 0.0F || cross2 < 0.0F) {
         /* point is outside triangle */
         insideCount--;
         stop = 15;
      }
   }
   if (stop == 4)
      return 15;
   else
      return insideCount;
}



static void
rgba_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define DO_Z
#define DO_RGBA
#include "aatritemp.h"
}


static void
index_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define DO_Z
#define DO_INDEX
#include "aatritemp.h"
}


/*
 * Compute mipmap level of detail.
 */
static INLINE GLfloat
compute_lambda(const GLfloat sPlane[4], const GLfloat tPlane[4],
               GLfloat invQ, GLfloat width, GLfloat height)
{
   GLfloat dudx = sPlane[0] / sPlane[2] * invQ * width;
   GLfloat dudy = sPlane[1] / sPlane[2] * invQ * width;
   GLfloat dvdx = tPlane[0] / tPlane[2] * invQ * height;
   GLfloat dvdy = tPlane[1] / tPlane[2] * invQ * height;
   GLfloat r1 = dudx * dudx + dudy * dudy;
   GLfloat r2 = dvdx * dvdx + dvdy * dvdy;
   GLfloat rho2 = r1 + r2;
   /* return log base 2 of rho */
   return log(rho2) * 1.442695 * 0.5;       /* 1.442695 = 1/log(2) */
}


static void
tex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define DO_Z
#define DO_RGBA
#define DO_STUV0
#include "aatritemp.h"
}


static void
spec_tex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define DO_Z
#define DO_RGBA
#define DO_STUV0
#define DO_SPEC
#include "aatritemp.h"
}


static void
multitex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define DO_Z
#define DO_RGBA
#define DO_STUV0
#define DO_STUV1
#include "aatritemp.h"
}

static void
spec_multitex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define DO_Z
#define DO_RGBA
#define DO_STUV0
#define DO_STUV1
#define DO_SPEC
#include "aatritemp.h"
}


/*
 * Examine GL state and set ctx->Driver.TriangleFunc to an
 * appropriate antialiased triangle rasterizer function.
 */
void
_mesa_set_aa_triangle_function(GLcontext *ctx)
{
   ASSERT(ctx->Polygon.SmoothFlag);
   if (ctx->Texture.ReallyEnabled) {
      if (ctx->Light.Enabled &&
          ctx->Light.Model.ColorControl==GL_SEPARATE_SPECULAR_COLOR) {
         if (ctx->Texture.ReallyEnabled >= TEXTURE1_1D) {
            ctx->Driver.TriangleFunc = spec_multitex_aa_tri;
         }
         else {
            ctx->Driver.TriangleFunc = spec_tex_aa_tri;
         }
      }
      else {
         if (ctx->Texture.ReallyEnabled >= TEXTURE1_1D) {
            ctx->Driver.TriangleFunc = multitex_aa_tri;
         }
         else {
            ctx->Driver.TriangleFunc = tex_aa_tri;
         }
      }
   }
   else {
      if (ctx->Visual->RGBAflag) {
         ctx->Driver.TriangleFunc = rgba_aa_tri;
      }
      else {
         ctx->Driver.TriangleFunc = index_aa_tri;
      }
   }
   ASSERT(ctx->Driver.TriangleFunc);
}
