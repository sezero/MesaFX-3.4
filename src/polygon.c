/* $Id: polygon.c,v 1.11 2000/03/13 18:32:37 brianp Exp $ */

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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "image.h"
#include "enums.h"
#include "macros.h"
#include "mem.h"
#include "polygon.h"
#include "types.h"
#endif



void
_mesa_CullFace( GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glCullFace");

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glCullFace %s\n", gl_lookup_enum_by_nr(mode));

   if (mode!=GL_FRONT && mode!=GL_BACK && mode!=GL_FRONT_AND_BACK) {
      gl_error( ctx, GL_INVALID_ENUM, "glCullFace" );
      return;
   }

   ctx->Polygon.CullFaceMode = mode;
   ctx->NewState |= NEW_POLYGON;

   if (ctx->Driver.CullFace)
      ctx->Driver.CullFace( ctx, mode );
}



void
_mesa_FrontFace( GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glFrontFace");

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glFrontFace %s\n", gl_lookup_enum_by_nr(mode));

   if (mode!=GL_CW && mode!=GL_CCW) {
      gl_error( ctx, GL_INVALID_ENUM, "glFrontFace" );
      return;
   }

   ctx->Polygon.FrontFace = mode;
   ctx->Polygon.FrontBit = (GLboolean) (mode == GL_CW);
   ctx->NewState |= NEW_POLYGON;

   if (ctx->Driver.FrontFace)
      ctx->Driver.FrontFace( ctx, mode );
}



void
_mesa_PolygonMode( GLenum face, GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPolygonMode");

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glPolygonMode %s %s\n", 
	      gl_lookup_enum_by_nr(face),
	      gl_lookup_enum_by_nr(mode));

   if (face!=GL_FRONT && face!=GL_BACK && face!=GL_FRONT_AND_BACK) {
      gl_error( ctx, GL_INVALID_ENUM, "glPolygonMode(face)" );
      return;
   }
   else if (mode!=GL_POINT && mode!=GL_LINE && mode!=GL_FILL) {
      gl_error( ctx, GL_INVALID_ENUM, "glPolygonMode(mode)" );
      return;
   }

   if (face==GL_FRONT || face==GL_FRONT_AND_BACK) {
      ctx->Polygon.FrontMode = mode;
   }
   if (face==GL_BACK || face==GL_FRONT_AND_BACK) {
      ctx->Polygon.BackMode = mode;
   }

   /* Compute a handy "shortcut" value: */
   ctx->TriangleCaps &= ~DD_TRI_UNFILLED;
   ctx->Polygon.Unfilled = GL_FALSE;

   if (ctx->Polygon.FrontMode!=GL_FILL || ctx->Polygon.BackMode!=GL_FILL) {
      ctx->Polygon.Unfilled = GL_TRUE;
      ctx->TriangleCaps |= DD_TRI_UNFILLED;
   }

   ctx->NewState |= (NEW_POLYGON | NEW_RASTER_OPS);

   if (ctx->Driver.PolygonMode) {
      (*ctx->Driver.PolygonMode)( ctx, face, mode );
   }
}



void
_mesa_PolygonStipple( const GLubyte *pattern )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPolygonStipple");

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glPolygonStipple\n");

   _mesa_unpack_polygon_stipple(pattern, ctx->PolygonStipple, &ctx->Unpack);

   if (ctx->Polygon.StippleFlag) {
      ctx->NewState |= NEW_RASTER_OPS;
   }
   
   if (ctx->Driver.PolygonStipple)
      ctx->Driver.PolygonStipple( ctx, (const GLubyte *) ctx->PolygonStipple );
}



void
_mesa_GetPolygonStipple( GLubyte *dest )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPolygonOffset");

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glGetPolygonStipple\n");

   _mesa_pack_polygon_stipple(ctx->PolygonStipple, dest, &ctx->Pack);
}



void
_mesa_PolygonOffset( GLfloat factor, GLfloat units )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPolygonOffset");

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glPolygonOffset %f %f\n", factor, units);

   ctx->Polygon.OffsetFactor = factor;
   ctx->Polygon.OffsetUnits = units;
}



void
_mesa_PolygonOffsetEXT( GLfloat factor, GLfloat bias )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPolygonOffsetEXT");
   _mesa_PolygonOffset(factor, bias * ctx->Visual->DepthMaxF );
}
