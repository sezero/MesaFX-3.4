/* $Id: dotprod_tmp.h,v 1.1.1.1 1999/08/19 00:55:41 jtg Exp $ */

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
 * New (3.1) transformation code written by Keith Whitwell.
 */


/* Note - respects the stride of the output vector.
 */
static void TAG(dotprod_vec2)( GLvector4f *out_vec, 
			     GLuint elt,
			     const GLvector4f *coord_vec, 
			     const GLfloat plane[4], 
			     const GLubyte mask[])
{
   GLuint stride = coord_vec->stride;
   GLfloat *coord = coord_vec->start;
   GLuint count = coord_vec->count;

   GLuint outstride = out_vec->stride;
   GLfloat *out = out_vec->start + elt;
   GLuint i;
   
   const GLfloat plane0 = plane[0], plane1 = plane[1], plane3 = plane[3];
      
   (void) mask;

   for (i=0;i<count;i++,STRIDE_F(coord,stride),STRIDE_F(out,outstride)) {
      CULL_CHECK {
	 *out = (coord[0] * plane0 +
		 coord[1] * plane1 +
		            plane3);
      }
   }   
   out_vec->count = coord_vec->count;
}

static void TAG(dotprod_vec3)( GLvector4f *out_vec, 
			     GLuint elt,
			     const GLvector4f *coord_vec, 
			     const GLfloat plane[4], 
			     const GLubyte mask[])
{
   GLuint stride = coord_vec->stride;
   GLfloat *coord = coord_vec->start;
   GLuint count = coord_vec->count;

   GLuint outstride = out_vec->stride;
   GLfloat *out = out_vec->start + elt;
   GLuint i;

   const GLfloat plane0 = plane[0], plane1 = plane[1], plane2 = plane[2];
   const GLfloat plane3 = plane[3];

   (void) mask;

   for (i=0;i<count;i++,STRIDE_F(coord,stride),STRIDE_F(out,outstride)) {
      CULL_CHECK {
	 *out = (coord[0] * plane0 +
		 coord[1] * plane1 +
		 coord[2] * plane2 +
		            plane3);
      }
   }   
   out_vec->count = coord_vec->count;
}

static void TAG(dotprod_vec4)( GLvector4f *out_vec, 
			     GLuint elt,
			     const GLvector4f *coord_vec, 
			     const GLfloat plane[4], 
			     const GLubyte mask[])
{
   GLuint stride = coord_vec->stride;
   GLfloat *coord = coord_vec->start;
   GLuint count = coord_vec->count;

   GLuint outstride = out_vec->stride;
   GLfloat *out = out_vec->start + elt;
   GLuint i;

   const GLfloat plane0 = plane[0], plane1 = plane[1], plane2 = plane[2];
   const GLfloat plane3 = plane[3];

   (void) mask;

   for (i=0;i<count;i++,STRIDE_F(coord,stride),STRIDE_F(out,outstride)) {
      CULL_CHECK {
	 *out = (coord[0] * plane0 +
		 coord[1] * plane1 +
		 coord[2] * plane2 +
		 coord[3] * plane3);
      }
   }   
   out_vec->count = coord_vec->count;
}


static void TAG(init_dotprod)( void )
{
   gl_dotprod_tab[IDX&1][2] = TAG(dotprod_vec2);
   gl_dotprod_tab[IDX&1][3] = TAG(dotprod_vec3);
   gl_dotprod_tab[IDX&1][4] = TAG(dotprod_vec4);
}
