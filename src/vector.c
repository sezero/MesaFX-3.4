/* $Id: vector.c,v 1.8 2000/06/27 22:10:00 brianp Exp $ */

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

/*
 * New (3.1) transformation code written by Keith Whitwell.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "config.h"
#include "macros.h"
#include "mem.h"
#include "vector.h"
#endif



static const GLubyte elem_bits[4] = {
   VEC_DIRTY_0,
   VEC_DIRTY_1, 
   VEC_DIRTY_2, 
   VEC_DIRTY_3
};

void gl_vector4f_clean_elem( GLvector4f *vec, GLuint count, GLuint elt )
{
   static GLfloat clean[4] = { 0, 0, 0, 1 };
   GLfloat v = clean[elt];
   GLfloat (*data)[4] = (GLfloat (*)[4])vec->start;
   GLuint i; 

   for (i = 0 ; i < count ; i++)
      data[i][elt] = v;

   vec->flags &= ~elem_bits[elt];
}

static const GLubyte size_bits[5] = {
   0,
   VEC_SIZE_1,
   VEC_SIZE_2,
   VEC_SIZE_3,
   VEC_SIZE_4,
};



void gl_vector4f_init( GLvector4f *v, GLuint flags, GLfloat (*storage)[4] )
{
   v->stride = 4*sizeof(GLfloat);
   v->size = 2;
   v->data = storage;
   v->start = (GLfloat *)storage;
   v->count = 0;
   v->flags = size_bits[4] | flags | VEC_GOOD_STRIDE;
}

void gl_vector3f_init( GLvector3f *v, GLuint flags, GLfloat (*storage)[3] )
{
   v->stride = 3*sizeof(GLfloat);
   v->data = storage;
   v->start = (GLfloat *)storage;
   v->count = 0;
   v->flags = flags | VEC_GOOD_STRIDE;
}

void gl_vector4ub_init( GLvector4ub *v, GLuint flags, GLubyte (*storage)[4] )
{
   v->stride = 4*sizeof(GLubyte);
   v->data = storage;
   v->start = (GLubyte *)storage;
   v->count = 0;
   v->flags = flags | VEC_GOOD_STRIDE;
}

void gl_vector1ub_init( GLvector1ub *v, GLuint flags, GLubyte *storage )
{
   v->stride = 1*sizeof(GLubyte);
   v->data = storage;
   v->start = (GLubyte *)storage;
   v->count = 0;
   v->flags = flags | VEC_GOOD_STRIDE;
}

void gl_vector1ui_init( GLvector1ui *v, GLuint flags, GLuint *storage )
{
   v->stride = 1*sizeof(GLuint);
   v->data = storage;
   v->start = (GLuint *)storage;
   v->count = 0;
   v->flags = flags | VEC_GOOD_STRIDE;
}



void gl_vector4f_alloc( GLvector4f *v, GLuint sz, GLuint flags, GLuint count,
			GLuint alignment )
{
   (void) sz;
   v->stride = 4*sizeof(GLfloat);
   v->size = 2;
   v->storage = v->start = (GLfloat *)
      ALIGN_MALLOC( count * 4 * sizeof(GLfloat), alignment );
   v->data = (GLfloat (*)[4])v->start;
   v->count = 0;
   v->flags = size_bits[4] | flags | VEC_MALLOC | VEC_GOOD_STRIDE;
}

void gl_vector3f_alloc( GLvector3f *v, GLuint flags, GLuint count,
			GLuint alignment )
{
   v->stride = 3*sizeof(GLfloat);
   v->storage = v->start = (GLfloat *)
      ALIGN_MALLOC( count * 3 * sizeof(GLfloat), alignment );
   v->data = (GLfloat (*)[3])v->start;
   v->count = 0;
   v->flags = flags | VEC_MALLOC | VEC_GOOD_STRIDE;
}

void gl_vector4ub_alloc( GLvector4ub *v, GLuint flags, GLuint count,
			 GLuint alignment )
{
   v->stride = 4*sizeof(GLubyte);
   v->storage = v->start = (GLubyte *)
      ALIGN_MALLOC( count * 4 * sizeof(GLubyte), alignment );
   v->data = (GLubyte (*)[4])v->start;
   v->count = 0;
   v->flags = flags | VEC_MALLOC | VEC_GOOD_STRIDE;
}

void gl_vector1ub_alloc( GLvector1ub *v, GLuint flags, GLuint count,
			 GLuint alignment )
{
   v->stride = 1*sizeof(GLubyte);
   v->storage = v->start = (GLubyte *)
      ALIGN_MALLOC( count * sizeof(GLubyte), alignment );
   v->data = v->start;
   v->count = 0;
   v->flags = flags | VEC_MALLOC | VEC_GOOD_STRIDE;
}

void gl_vector1ui_alloc( GLvector1ui *v, GLuint flags, GLuint count,
			 GLuint alignment )
{
   v->stride = 1*sizeof(GLuint);
   v->storage = v->start = (GLuint *)
      ALIGN_MALLOC( count * sizeof(GLuint), alignment );
   v->data = v->start;
   v->count = 0;
   v->flags = flags | VEC_MALLOC | VEC_GOOD_STRIDE;
}



void gl_vector4f_free( GLvector4f *v )
{
   if (v->flags & VEC_MALLOC) {
      ALIGN_FREE( v->storage );
      v->data = 0;
      v->start = 0;
      v->storage = 0;
      v->flags &= ~VEC_MALLOC;
   }
}

void gl_vector3f_free( GLvector3f *v )
{
   if (v->flags & VEC_MALLOC) {
      ALIGN_FREE( v->storage );
      v->data = 0;
      v->start = 0;
      v->storage = 0;
      v->flags &= ~VEC_MALLOC;
   }
}

void gl_vector4ub_free( GLvector4ub *v )
{
   if (v->flags & VEC_MALLOC) {
      ALIGN_FREE( v->storage );
      v->data = 0;
      v->start = 0;
      v->storage = 0;
      v->flags &= ~VEC_MALLOC;
   }
}

void gl_vector1ub_free( GLvector1ub *v )
{
   if (v->flags & VEC_MALLOC) {
      ALIGN_FREE( v->storage );
      v->data = 0;
      v->start = 0;
      v->storage = 0;
      v->flags &= ~VEC_MALLOC;
   }
}

void gl_vector1ui_free( GLvector1ui *v )
{
   if (v->flags & VEC_MALLOC) {
      ALIGN_FREE( v->storage );
      v->data = 0;
      v->start = 0;
      v->storage = 0;
      v->flags &= ~VEC_MALLOC;
   }
}



void gl_vector4f_print( GLvector4f *v, GLubyte *cullmask, GLboolean culling )
{
   GLfloat c[4] = { 0, 0, 0, 1 };
   const char *templates[5] = {
      "%d:\t0, 0, 0, 1\n",
      "%d:\t%f, 0, 0, 1\n",
      "%d:\t%f, %f, 0, 1\n",
      "%d:\t%f, %f, %f, 1\n",
      "%d:\t%f, %f, %f, %f\n"
   };

   const char *t = templates[v->size];
   GLfloat *d = (GLfloat *)v->data;
   GLuint j, i = 0, count;

   printf("data-start\n");
   for ( ; d != v->start ; STRIDE_F(d, v->stride), i++) 
      printf( t, i, d[0], d[1], d[2], d[3]);
   
   printf("start-count(%u)\n", v->count);
   count = i + v->count;

   if (culling) {
      for ( ; i < count ; STRIDE_F(d, v->stride), i++) 
	 if (cullmask[i]) 
	    printf( t, i, d[0], d[1], d[2], d[3]);      
   } else {
      for ( ; i < count ; STRIDE_F(d, v->stride), i++) 
	 printf( t, i, d[0], d[1], d[2], d[3]);      
   }

   for (j = v->size ; j < 4; j++) {
      if ((v->flags & (1<<j)) == 0) {

	 printf("checking col %u is clean as advertised ", j);

	 for (i = 0, d = (GLfloat *) v->data ; 
	      i < count && d[j] == c[j] ; 
	      i++, STRIDE_F(d, v->stride)) {};

	 if (i == count) 
	    printf(" --> ok\n");
	 else 
	    printf(" --> Failed at %u ******\n", i);
      }
   }
}

void gl_vector3f_print( GLvector3f *v, GLubyte *cullmask, GLboolean culling )
{
   GLfloat *d = (GLfloat *)v->data;
   GLuint i = 0, count;

   printf("data-start\n");
   for ( ; d != v->start ; STRIDE_F(d,v->stride), i++) 
      printf( "%u:\t%f, %f, %f\n", i, d[0], d[1], d[2]);
   
   printf("start-count(%u)\n", v->count);
   count = i + v->count;

   if (culling) {
      for ( ; i < count ; STRIDE_F(d,v->stride), i++) 
	 if (cullmask[i])
	    printf( "%u:\t%f, %f, %f\n", i, d[0], d[1], d[2]);      
   } else {
      for ( ; i < count ; STRIDE_F(d,v->stride), i++) 
	 printf( "%u:\t%f, %f, %f\n", i, d[0], d[1], d[2]);      
   }
}
