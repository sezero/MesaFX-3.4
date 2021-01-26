/* $Id: translate.c,v 1.5 1999/11/12 16:46:56 kendallb Exp $ */

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
#include "mem.h"
#include "mmath.h"
#include "translate.h"
#include "types.h"
#endif

/* This macro is used on other systems, so undefine it for this module */

#undef	CHECK

trans_1ui_func gl_trans_1ui_tab[MAX_TYPES];
trans_1ub_func gl_trans_1ub_tab[MAX_TYPES];
trans_3f_func  gl_trans_3f_tab[MAX_TYPES];
trans_4ub_func gl_trans_4ub_tab[5][MAX_TYPES];
trans_4f_func  gl_trans_4f_tab[5][MAX_TYPES];

trans_elt_1ui_func gl_trans_elt_1ui_tab[MAX_TYPES];
trans_elt_1ub_func gl_trans_elt_1ub_tab[MAX_TYPES];
trans_elt_3f_func  gl_trans_elt_3f_tab[MAX_TYPES];
trans_elt_4ub_func gl_trans_elt_4ub_tab[5][MAX_TYPES];
trans_elt_4f_func  gl_trans_elt_4f_tab[5][MAX_TYPES];


#define PTR_ELT(ptr, elt) (((SRC *)ptr)[elt])


#define TAB(x) gl_trans##x##_tab
#define ARGS   GLuint start, GLuint n
#define SRC_START  start
#define DST_START  0
#define STRIDE stride
#define NEXT_F f += stride
#define NEXT_F2
#define CHECK




/* GL_BYTE
 */
#define SRC GLbyte
#define SRC_IDX TYPE_IDX(GL_BYTE)
#define TRX_3F(f,n)   BYTE_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = BYTE_TO_UBYTE( PTR_ELT(f,n) )
#define TRX_UI(f,n)  (PTR_ELT(f,n) < 0 ? 0 : (GLuint)  PTR_ELT(f,n))


#define SZ 4
#define INIT init_trans_4_GLbyte_raw
#define DEST_4F trans_4_GLbyte_4f_raw
#define DEST_4UB trans_4_GLbyte_4ub_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLbyte_raw
#define DEST_4F trans_3_GLbyte_4f_raw
#define DEST_4UB trans_3_GLbyte_4ub_raw
#define DEST_3F trans_3_GLbyte_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLbyte_raw
#define DEST_4F trans_2_GLbyte_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLbyte_raw
#define DEST_4F trans_1_GLbyte_4f_raw
#define DEST_1UB trans_1_GLbyte_1ub_raw
#define DEST_1UI trans_1_GLbyte_1ui_raw
#include "trans_tmp.h"

#undef SRC
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI
#undef SRC_IDX

/* GL_UNSIGNED_BYTE
 */
#define SRC GLubyte
#define SRC_IDX TYPE_IDX(GL_UNSIGNED_BYTE)
#define TRX_3F(f,n)		/* unused */
#define TRX_4F(f,n)		/* unused */
#define TRX_UB(ub, f,n)	     ub = PTR_ELT(f,n)
#define TRX_UI(f,n)          (GLuint)PTR_ELT(f,n)

/* 4ub->4ub handled in special case below.
 */

#define SZ 3
#define INIT init_trans_3_GLubyte_raw
#define DEST_4UB trans_3_GLubyte_4ub_raw
#include "trans_tmp.h"


#define SZ 1
#define INIT init_trans_1_GLubyte_raw
#define DEST_1UI trans_1_GLubyte_1ui_raw
#define DEST_1UB trans_1_GLubyte_1ub_raw
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_SHORT
 */
#define SRC GLshort
#define SRC_IDX TYPE_IDX(GL_SHORT)
#define TRX_3F(f,n)   SHORT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = SHORT_TO_UBYTE(PTR_ELT(f,n))
#define TRX_UI(f,n)  (PTR_ELT(f,n) < 0 ? 0 : (GLuint)  PTR_ELT(f,n))


#define SZ  4
#define INIT init_trans_4_GLshort_raw
#define DEST_4F trans_4_GLshort_4f_raw
#define DEST_4UB trans_4_GLshort_4ub_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLshort_raw
#define DEST_4F trans_3_GLshort_4f_raw
#define DEST_4UB trans_3_GLshort_4ub_raw
#define DEST_3F trans_3_GLshort_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLshort_raw
#define DEST_4F trans_2_GLshort_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLshort_raw
#define DEST_4F trans_1_GLshort_4f_raw
#define DEST_1UB trans_1_GLshort_1ub_raw
#define DEST_1UI trans_1_GLshort_1ui_raw
#include "trans_tmp.h"


#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_UNSIGNED_SHORT
 */
#define SRC GLushort
#define SRC_IDX TYPE_IDX(GL_UNSIGNED_SHORT)
#define TRX_3F(f,n)   USHORT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub,f,n)  ub = (GLubyte) (PTR_ELT(f,n) >> 8)
#define TRX_UI(f,n)  (GLuint)   PTR_ELT(f,n)


#define SZ 4
#define INIT init_trans_4_GLushort_raw
#define DEST_4F trans_4_GLushort_4f_raw
#define DEST_4UB trans_4_GLushort_4ub_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLushort_raw
#define DEST_4F trans_3_GLushort_4f_raw
#define DEST_4UB trans_3_GLushort_4ub_raw
#define DEST_3F trans_3_GLushort_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLushort_raw
#define DEST_4F trans_2_GLushort_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLushort_raw
#define DEST_4F trans_1_GLushort_4f_raw
#define DEST_1UB trans_1_GLushort_1ub_raw
#define DEST_1UI trans_1_GLushort_1ui_raw
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_INT
 */
#define SRC GLint
#define SRC_IDX TYPE_IDX(GL_INT)
#define TRX_3F(f,n)   INT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = INT_TO_UBYTE(PTR_ELT(f,n))
#define TRX_UI(f,n)  (PTR_ELT(f,n) < 0 ? 0 : (GLuint)  PTR_ELT(f,n))


#define SZ 4
#define INIT init_trans_4_GLint_raw
#define DEST_4F trans_4_GLint_4f_raw
#define DEST_4UB trans_4_GLint_4ub_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLint_raw
#define DEST_4F trans_3_GLint_4f_raw
#define DEST_4UB trans_3_GLint_4ub_raw
#define DEST_3F trans_3_GLint_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLint_raw
#define DEST_4F trans_2_GLint_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLint_raw
#define DEST_4F trans_1_GLint_4f_raw
#define DEST_1UB trans_1_GLint_1ub_raw
#define DEST_1UI trans_1_GLint_1ui_raw
#include "trans_tmp.h"


#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_UNSIGNED_INT
 */
#define SRC GLuint
#define SRC_IDX TYPE_IDX(GL_UNSIGNED_INT)
#define TRX_3F(f,n)   INT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = (GLubyte) (PTR_ELT(f,n) >> 24)
#define TRX_UI(f,n)		PTR_ELT(f,n)


#define SZ 4
#define INIT init_trans_4_GLuint_raw
#define DEST_4F trans_4_GLuint_4f_raw
#define DEST_4UB trans_4_GLuint_4ub_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLuint_raw
#define DEST_4F trans_3_GLuint_4f_raw
#define DEST_4UB trans_3_GLuint_4ub_raw
#define DEST_3F trans_3_GLuint_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLuint_raw
#define DEST_4F trans_2_GLuint_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLuint_raw
#define DEST_4F trans_1_GLuint_4f_raw
#define DEST_1UB trans_1_GLuint_1ub_raw
#define DEST_1UI trans_1_GLuint_1ui_raw
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_DOUBLE
 */
#define SRC GLdouble
#define SRC_IDX TYPE_IDX(GL_DOUBLE)
#define TRX_3F(f,n)   PTR_ELT(f,n)
#define TRX_4F(f,n)   PTR_ELT(f,n)
#define TRX_UB(ub,f,n) FLOAT_COLOR_TO_UBYTE_COLOR(ub, PTR_ELT(f,n))
#define TRX_UI(f,n)  (GLuint) (GLint) PTR_ELT(f,n)


#define SZ 4
#define INIT init_trans_4_GLdouble_raw
#define DEST_4F trans_4_GLdouble_4f_raw
#define DEST_4UB trans_4_GLdouble_4ub_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLdouble_raw
#define DEST_4F trans_3_GLdouble_4f_raw
#define DEST_4UB trans_3_GLdouble_4ub_raw
#define DEST_3F trans_3_GLdouble_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLdouble_raw
#define DEST_4F trans_2_GLdouble_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLdouble_raw
#define DEST_4F trans_1_GLdouble_4f_raw
#define DEST_1UB trans_1_GLdouble_1ub_raw
#define DEST_1UI trans_1_GLdouble_1ui_raw
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX

/* GL_FLOAT
 */
#define SRC GLfloat
#define SRC_IDX TYPE_IDX(GL_FLOAT)
#define SZ 4
#define INIT init_trans_4_GLfloat_raw 
#define DEST_4UB trans_4_GLfloat_4ub_raw 
#define DEST_4F  trans_4_GLfloat_4f_raw
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLfloat_raw
#define DEST_4F  trans_3_GLfloat_4f_raw
#define DEST_4UB trans_3_GLfloat_4ub_raw
#define DEST_3F trans_3_GLfloat_3f_raw
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLfloat_raw
#define DEST_4F trans_2_GLfloat_4f_raw
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLfloat_raw
#define DEST_4F  trans_1_GLfloat_4f_raw
#define DEST_1UB trans_1_GLfloat_1ub_raw
#define DEST_1UI trans_1_GLfloat_1ui_raw

#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


static void trans_4_GLubyte_4ub_raw (GLubyte (*t)[4],
				     const struct gl_client_array *from,
				     ARGS )
{
   GLuint stride = from->StrideB;
   const GLubyte *f = (GLubyte *) from->Ptr + SRC_START * stride;
   GLuint i;

   if (((((long) f | (long) stride)) & 3L) == 0L) { 
      /* Aligned.
       */
      for (i = DST_START ; i < n ; i++, f += stride) {
	 COPY_4UBV( t[i], f );
      }
   } else {
      for (i = DST_START ; i < n ; i++, f += stride) {
	 t[i][0] = f[0];
	 t[i][1] = f[1];
	 t[i][2] = f[2];
	 t[i][3] = f[3];
      }
   }
}


static void init_translate_raw(void)
{
   MEMSET( TAB(_1ui), 0, sizeof(TAB(_1ui)) );
   MEMSET( TAB(_1ub), 0, sizeof(TAB(_1ub)) );
   MEMSET( TAB(_3f),  0, sizeof(TAB(_3f)) );
   MEMSET( TAB(_4ub), 0, sizeof(TAB(_4ub)) );
   MEMSET( TAB(_4f),  0, sizeof(TAB(_4f)) );

   TAB(_4ub)[4][TYPE_IDX(GL_UNSIGNED_BYTE)] = trans_4_GLubyte_4ub_raw;

   init_trans_4_GLbyte_raw();
   init_trans_3_GLbyte_raw();
   init_trans_2_GLbyte_raw();
   init_trans_1_GLbyte_raw();
   init_trans_1_GLubyte_raw();
   init_trans_3_GLubyte_raw();
   init_trans_4_GLshort_raw();
   init_trans_3_GLshort_raw();
   init_trans_2_GLshort_raw();
   init_trans_1_GLshort_raw();
   init_trans_4_GLushort_raw();
   init_trans_3_GLushort_raw();
   init_trans_2_GLushort_raw();
   init_trans_1_GLushort_raw();
   init_trans_4_GLint_raw();
   init_trans_3_GLint_raw();
   init_trans_2_GLint_raw();
   init_trans_1_GLint_raw();
   init_trans_4_GLuint_raw();
   init_trans_3_GLuint_raw();
   init_trans_2_GLuint_raw();
   init_trans_1_GLuint_raw();
   init_trans_4_GLdouble_raw();
   init_trans_3_GLdouble_raw();
   init_trans_2_GLdouble_raw();
   init_trans_1_GLdouble_raw();
   init_trans_4_GLfloat_raw();
   init_trans_3_GLfloat_raw();
   init_trans_2_GLfloat_raw();
   init_trans_1_GLfloat_raw();
}


#undef TAB
#undef CLASS
#undef ARGS
#undef CHECK
#undef SRC_START
#undef DST_START
#undef NEXT_F
#undef NEXT_F2


/* Code specific to array element implementation.  There is a small
 * subtlety in the bits CHECK() tests, and the way bits are set in
 * glArrayElement which ensures that if, eg, in the case that the
 * vertex array is disabled and normal array is enabled, and we get
 * either sequence:
 *
 * ArrayElement()    OR   Normal()
 * Normal()               ArrayElement()
 * Vertex()               Vertex()
 *
 * That the correct value for normal is used.  
 */
#define TAB(x) gl_trans_elt##x##_tab
#define ARGS   GLuint *flags, GLuint *elts, GLuint match, \
               GLuint start, GLuint n
#define SRC_START  0
#define DST_START  start
#define CHECK  if ((flags[i]&match) == VERT_ELT)
#define NEXT_F  (void)1
#define NEXT_F2 f = first + elts[i] * stride; 


/* GL_BYTE
 */
#define SRC GLbyte
#define SRC_IDX TYPE_IDX(GL_BYTE)
#define TRX_3F(f,n)   BYTE_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = BYTE_TO_UBYTE( PTR_ELT(f,n) )
#define TRX_UI(f,n)  (PTR_ELT(f,n) < 0 ? 0 : (GLuint)  PTR_ELT(f,n))


#define SZ 4
#define INIT init_trans_4_GLbyte_elt
#define DEST_4F trans_4_GLbyte_4f_elt
#define DEST_4UB trans_4_GLbyte_4ub_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLbyte_elt
#define DEST_4F trans_3_GLbyte_4f_elt
#define DEST_4UB trans_3_GLbyte_4ub_elt
#define DEST_3F trans_3_GLbyte_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLbyte_elt
#define DEST_4F trans_2_GLbyte_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLbyte_elt
#define DEST_4F trans_1_GLbyte_4f_elt
#define DEST_1UB trans_1_GLbyte_1ub_elt
#define DEST_1UI trans_1_GLbyte_1ui_elt
#include "trans_tmp.h"

#undef SRC
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI
#undef SRC_IDX

/* GL_UNSIGNED_BYTE
 */
#define SRC GLubyte
#define SRC_IDX TYPE_IDX(GL_UNSIGNED_BYTE)
#define TRX_3F(f,n)		/* unused */
#define TRX_4F(f,n)		/* unused */
#define TRX_UB(ub, f,n)	     ub = PTR_ELT(f,n)
#define TRX_UI(f,n)          (GLuint)PTR_ELT(f,n)

/* 4ub->4ub handled in special case below.
 */

#define SZ 3
#define INIT init_trans_3_GLubyte_elt
#define DEST_4UB trans_3_GLubyte_4ub_elt
#include "trans_tmp.h"


#define SZ 1
#define INIT init_trans_1_GLubyte_elt
#define DEST_1UI trans_1_GLubyte_1ui_elt
#define DEST_1UB trans_1_GLubyte_1ub_elt
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_SHORT
 */
#define SRC GLshort
#define SRC_IDX TYPE_IDX(GL_SHORT)
#define TRX_3F(f,n)   SHORT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = SHORT_TO_UBYTE(PTR_ELT(f,n))
#define TRX_UI(f,n)  (PTR_ELT(f,n) < 0 ? 0 : (GLuint)  PTR_ELT(f,n))


#define SZ  4
#define INIT init_trans_4_GLshort_elt
#define DEST_4F trans_4_GLshort_4f_elt
#define DEST_4UB trans_4_GLshort_4ub_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLshort_elt
#define DEST_4F trans_3_GLshort_4f_elt
#define DEST_4UB trans_3_GLshort_4ub_elt
#define DEST_3F trans_3_GLshort_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLshort_elt
#define DEST_4F trans_2_GLshort_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLshort_elt
#define DEST_4F trans_1_GLshort_4f_elt
#define DEST_1UB trans_1_GLshort_1ub_elt
#define DEST_1UI trans_1_GLshort_1ui_elt
#include "trans_tmp.h"


#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_UNSIGNED_SHORT
 */
#define SRC GLushort
#define SRC_IDX TYPE_IDX(GL_UNSIGNED_SHORT)
#define TRX_3F(f,n)   USHORT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub,f,n)  ub = (GLubyte) (PTR_ELT(f,n) >> 8)
#define TRX_UI(f,n)  (GLuint)   PTR_ELT(f,n)


#define SZ 4
#define INIT init_trans_4_GLushort_elt
#define DEST_4F trans_4_GLushort_4f_elt
#define DEST_4UB trans_4_GLushort_4ub_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLushort_elt
#define DEST_4F trans_3_GLushort_4f_elt
#define DEST_4UB trans_3_GLushort_4ub_elt
#define DEST_3F trans_3_GLushort_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLushort_elt
#define DEST_4F trans_2_GLushort_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLushort_elt
#define DEST_4F trans_1_GLushort_4f_elt
#define DEST_1UB trans_1_GLushort_1ub_elt
#define DEST_1UI trans_1_GLushort_1ui_elt
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_INT
 */
#define SRC GLint
#define SRC_IDX TYPE_IDX(GL_INT)
#define TRX_3F(f,n)   INT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = INT_TO_UBYTE(PTR_ELT(f,n))
#define TRX_UI(f,n)  (PTR_ELT(f,n) < 0 ? 0 : (GLuint)  PTR_ELT(f,n))


#define SZ 4
#define INIT init_trans_4_GLint_elt
#define DEST_4F trans_4_GLint_4f_elt
#define DEST_4UB trans_4_GLint_4ub_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLint_elt
#define DEST_4F trans_3_GLint_4f_elt
#define DEST_4UB trans_3_GLint_4ub_elt
#define DEST_3F trans_3_GLint_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLint_elt
#define DEST_4F trans_2_GLint_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLint_elt
#define DEST_4F trans_1_GLint_4f_elt
#define DEST_1UB trans_1_GLint_1ub_elt
#define DEST_1UI trans_1_GLint_1ui_elt
#include "trans_tmp.h"


#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_UNSIGNED_INT
 */
#define SRC GLuint
#define SRC_IDX TYPE_IDX(GL_UNSIGNED_INT)
#define TRX_3F(f,n)   UINT_TO_FLOAT( PTR_ELT(f,n) )
#define TRX_4F(f,n)   (GLfloat)( PTR_ELT(f,n) )
#define TRX_UB(ub, f,n)  ub = (GLubyte) (PTR_ELT(f,n) >> 24)
#define TRX_UI(f,n)		PTR_ELT(f,n)


#define SZ 4
#define INIT init_trans_4_GLuint_elt
#define DEST_4F trans_4_GLuint_4f_elt
#define DEST_4UB trans_4_GLuint_4ub_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLuint_elt
#define DEST_4F trans_3_GLuint_4f_elt
#define DEST_4UB trans_3_GLuint_4ub_elt
#define DEST_3F trans_3_GLuint_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLuint_elt
#define DEST_4F trans_2_GLuint_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLuint_elt
#define DEST_4F trans_1_GLuint_4f_elt
#define DEST_1UB trans_1_GLuint_1ub_elt
#define DEST_1UI trans_1_GLuint_1ui_elt
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


/* GL_DOUBLE
 */
#define SRC GLdouble
#define SRC_IDX TYPE_IDX(GL_DOUBLE)
#define TRX_3F(f,n)   PTR_ELT(f,n)
#define TRX_4F(f,n)   PTR_ELT(f,n)
#define TRX_UB(ub,f,n) FLOAT_COLOR_TO_UBYTE_COLOR(ub, PTR_ELT(f,n))
#define TRX_UI(f,n)  (GLuint) (GLint) PTR_ELT(f,n)


#define SZ 4
#define INIT init_trans_4_GLdouble_elt
#define DEST_4F trans_4_GLdouble_4f_elt
#define DEST_4UB trans_4_GLdouble_4ub_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLdouble_elt
#define DEST_4F trans_3_GLdouble_4f_elt
#define DEST_4UB trans_3_GLdouble_4ub_elt
#define DEST_3F trans_3_GLdouble_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLdouble_elt
#define DEST_4F trans_2_GLdouble_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLdouble_elt
#define DEST_4F trans_1_GLdouble_4f_elt
#define DEST_1UB trans_1_GLdouble_1ub_elt
#define DEST_1UI trans_1_GLdouble_1ui_elt
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX

/* GL_FLOAT
 */
#define SRC GLfloat
#define SRC_IDX TYPE_IDX(GL_FLOAT)
#define SZ 4
#define INIT init_trans_4_GLfloat_elt 
#define DEST_4UB trans_4_GLfloat_4ub_elt 
#define DEST_4F  trans_4_GLfloat_4f_elt
#include "trans_tmp.h"

#define SZ 3
#define INIT init_trans_3_GLfloat_elt
#define DEST_4F  trans_3_GLfloat_4f_elt
#define DEST_4UB trans_3_GLfloat_4ub_elt
#define DEST_3F trans_3_GLfloat_3f_elt
#include "trans_tmp.h"

#define SZ 2
#define INIT init_trans_2_GLfloat_elt
#define DEST_4F trans_2_GLfloat_4f_elt
#include "trans_tmp.h"

#define SZ 1
#define INIT init_trans_1_GLfloat_elt
#define DEST_4F  trans_1_GLfloat_3f_elt
#define DEST_1UB trans_1_GLfloat_1ub_elt
#define DEST_1UI trans_1_GLfloat_1ui_elt
#include "trans_tmp.h"

#undef SRC
#undef SRC_IDX
#undef TRX_3F
#undef TRX_4F
#undef TRX_UB
#undef TRX_UI


static void trans_4_GLubyte_4ub(GLubyte (*t)[4],
				const struct gl_client_array *from,
				ARGS )
{
   GLuint stride = from->StrideB;
   const GLubyte *f = (GLubyte *) from->Ptr + SRC_START * stride;
   const GLubyte *first = f;
   GLuint i;
   (void) start;
   if (((((long) f | (long) stride)) & 3L) == 0L) { 
      /* Aligned.
       */
      for (i = DST_START ; i < n ; i++, NEXT_F) {
	 CHECK {
	    NEXT_F2;
	    COPY_4UBV( t[i], f );
	 }
      }
   } else {
      for (i = DST_START ; i < n ; i++, NEXT_F) {
	 CHECK {
	    NEXT_F2;
	    t[i][0] = f[0];
	    t[i][1] = f[1];
	    t[i][2] = f[2];
	    t[i][3] = f[3];
	 }
      }
   }
}


static void init_translate_elt(void)
{
   MEMSET( TAB(_1ui), 0, sizeof(TAB(_1ui)) );
   MEMSET( TAB(_1ub), 0, sizeof(TAB(_1ub)) );
   MEMSET( TAB(_3f),  0, sizeof(TAB(_3f)) );
   MEMSET( TAB(_4ub), 0, sizeof(TAB(_4ub)) );
   MEMSET( TAB(_4f),  0, sizeof(TAB(_4f)) );

   TAB(_4ub)[4][TYPE_IDX(GL_UNSIGNED_BYTE)] = trans_4_GLubyte_4ub;

   init_trans_4_GLbyte_elt();
   init_trans_3_GLbyte_elt();
   init_trans_2_GLbyte_elt();
   init_trans_1_GLbyte_elt();
   init_trans_1_GLubyte_elt();
   init_trans_3_GLubyte_elt();
   init_trans_4_GLshort_elt();
   init_trans_3_GLshort_elt();
   init_trans_2_GLshort_elt();
   init_trans_1_GLshort_elt();
   init_trans_4_GLushort_elt();
   init_trans_3_GLushort_elt();
   init_trans_2_GLushort_elt();
   init_trans_1_GLushort_elt();
   init_trans_4_GLint_elt();
   init_trans_3_GLint_elt();
   init_trans_2_GLint_elt();
   init_trans_1_GLint_elt();
   init_trans_4_GLuint_elt();
   init_trans_3_GLuint_elt();
   init_trans_2_GLuint_elt();
   init_trans_1_GLuint_elt();
   init_trans_4_GLdouble_elt();
   init_trans_3_GLdouble_elt();
   init_trans_2_GLdouble_elt();
   init_trans_1_GLdouble_elt();
   init_trans_4_GLfloat_elt();
   init_trans_3_GLfloat_elt();
   init_trans_2_GLfloat_elt();
   init_trans_1_GLfloat_elt();
}


#undef TAB
#undef CLASS
#undef ARGS
#undef CHECK
#undef START




void gl_init_translate( void )
{
   init_translate_raw();
   init_translate_elt();
}
