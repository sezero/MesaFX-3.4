/* $Id: macros.h,v 1.8.4.4 2001/06/11 15:18:09 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4
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
 * A collection of useful macros.
 */


#ifndef MACROS_H
#define MACROS_H


#include "glheader.h"


#ifdef DEBUG
#  define ASSERT(X)   assert(X)
#else
#  define ASSERT(X)
#endif


#if defined(__GNUC__)
#define INLINE __inline__
#elif defined(__MSC__)
#define INLINE __inline
#else
#define INLINE
#endif


/* Limits: */
#define MAX_GLUSHORT	0xffff
#define MAX_GLUINT	0xffffffff


/* Some compilers don't like some of Mesa's const usage */
#ifdef NO_CONST
#  define CONST
#else
#  define CONST const
#endif


/* Pi */
#ifndef M_PI
#define M_PI (3.1415926)
#endif


/* Degrees to radians conversion: */
#define DEG2RAD (M_PI/180.0)


#ifndef NULL
#define NULL 0
#endif



/*
 * Bitmask helpers
 */
#define SET_BITS(WORD, BITS)    (WORD) |= (BITS)
#define CLEAR_BITS(WORD, BITS)  (WORD) &= ~(BITS)
#define TEST_BITS(WORD, BITS)   ((WORD) & (BITS))


/* Stepping a GLfloat pointer by a byte stride
 */
#define STRIDE_F(p, i)  (p = (GLfloat *)((GLubyte *)p + i))
#define STRIDE_UI(p, i)  (p = (GLuint *)((GLubyte *)p + i))
#define STRIDE_T(p, t, i)  (p = (t *)((GLubyte *)p + i))


#define ZERO_2V( DST )	(DST)[0] = (DST)[1] = 0
#define ZERO_3V( DST )	(DST)[0] = (DST)[1] = (DST)[2] = 0
#define ZERO_4V( DST )	(DST)[0] = (DST)[1] = (DST)[2] = (DST)[3] = 0


/* Copy short vectors: */
#define COPY_2V( DST, SRC )			\
do {						\
   (DST)[0] = (SRC)[0];				\
   (DST)[1] = (SRC)[1];				\
} while (0)


#define COPY_3V( DST, SRC )			\
do {						\
   (DST)[0] = (SRC)[0];				\
   (DST)[1] = (SRC)[1];				\
   (DST)[2] = (SRC)[2];				\
} while (0)

#define COPY_4V( DST, SRC )			\
do {						\
   (DST)[0] = (SRC)[0];				\
   (DST)[1] = (SRC)[1];				\
   (DST)[2] = (SRC)[2];				\
   (DST)[3] = (SRC)[3];				\
} while (0)


#define COPY_2FV( DST, SRC )			\
do {						\
   const GLfloat *_tmp = (SRC);			\
   (DST)[0] = _tmp[0];				\
   (DST)[1] = _tmp[1];				\
} while (0)


#define COPY_3FV( DST, SRC )			\
do {						\
   const GLfloat *_tmp = (SRC);			\
   (DST)[0] = _tmp[0];				\
   (DST)[1] = _tmp[1];				\
   (DST)[2] = _tmp[2];				\
} while (0)

#define COPY_4FV( DST, SRC )			\
do {						\
   const GLfloat *_tmp = (SRC);			\
   (DST)[0] = _tmp[0];				\
   (DST)[1] = _tmp[1];				\
   (DST)[2] = _tmp[2];				\
   (DST)[3] = _tmp[3];				\
} while (0)



#define COPY_SZ_4V(DST, SZ, SRC) 		\
do {						\
   switch (SZ) {				\
   case 4: (DST)[3] = (SRC)[3];			\
   case 3: (DST)[2] = (SRC)[2];			\
   case 2: (DST)[1] = (SRC)[1];			\
   case 1: (DST)[0] = (SRC)[0];			\
   }  						\
} while(0)

#define SUB_4V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] - (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] - (SRCB)[1];		\
      (DST)[2] = (SRCA)[2] - (SRCB)[2];		\
      (DST)[3] = (SRCA)[3] - (SRCB)[3];		\
} while (0)

#define ADD_4V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] + (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] + (SRCB)[1];		\
      (DST)[2] = (SRCA)[2] + (SRCB)[2];		\
      (DST)[3] = (SRCA)[3] + (SRCB)[3];		\
} while (0)

#define SCALE_4V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] * (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] * (SRCB)[1];		\
      (DST)[2] = (SRCA)[2] * (SRCB)[2];		\
      (DST)[3] = (SRCA)[3] * (SRCB)[3];		\
} while (0)

#define ACC_4V( DST, SRC )			\
do {						\
      (DST)[0] += (SRC)[0];			\
      (DST)[1] += (SRC)[1];			\
      (DST)[2] += (SRC)[2];			\
      (DST)[3] += (SRC)[3];			\
} while (0)

#define ACC_SCALE_4V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] += (SRCA)[0] * (SRCB)[0];	\
      (DST)[1] += (SRCA)[1] * (SRCB)[1];	\
      (DST)[2] += (SRCA)[2] * (SRCB)[2];	\
      (DST)[3] += (SRCA)[3] * (SRCB)[3];	\
} while (0)

#define ACC_SCALE_SCALAR_4V( DST, S, SRCB )	\
do {						\
      (DST)[0] += S * (SRCB)[0];		\
      (DST)[1] += S * (SRCB)[1];		\
      (DST)[2] += S * (SRCB)[2];		\
      (DST)[3] += S * (SRCB)[3];		\
} while (0)

#define SCALE_SCALAR_4V( DST, S, SRCB )		\
do {						\
      (DST)[0] = S * (SRCB)[0];			\
      (DST)[1] = S * (SRCB)[1];			\
      (DST)[2] = S * (SRCB)[2];			\
      (DST)[3] = S * (SRCB)[3];			\
} while (0)


#define SELF_SCALE_SCALAR_4V( DST, S )		\
do {						\
      (DST)[0] *= S;				\
      (DST)[1] *= S;				\
      (DST)[2] *= S;				\
      (DST)[3] *= S;				\
} while (0)


/*
 * Similarly for 3-vectors.
 */
#define SUB_3V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] - (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] - (SRCB)[1];		\
      (DST)[2] = (SRCA)[2] - (SRCB)[2];		\
} while (0)

#define ADD_3V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] + (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] + (SRCB)[1];		\
      (DST)[2] = (SRCA)[2] + (SRCB)[2];		\
} while (0)

#define SCALE_3V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] * (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] * (SRCB)[1];		\
      (DST)[2] = (SRCA)[2] * (SRCB)[2];		\
} while (0)

#define ACC_3V( DST, SRC )			\
do {						\
      (DST)[0] += (SRC)[0];			\
      (DST)[1] += (SRC)[1];			\
      (DST)[2] += (SRC)[2];			\
} while (0)

#define ACC_SCALE_3V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] += (SRCA)[0] * (SRCB)[0];	\
      (DST)[1] += (SRCA)[1] * (SRCB)[1];	\
      (DST)[2] += (SRCA)[2] * (SRCB)[2];	\
} while (0)

#define SCALE_SCALAR_3V( DST, S, SRCB ) 	\
do {						\
      (DST)[0] = S * (SRCB)[0];			\
      (DST)[1] = S * (SRCB)[1];			\
      (DST)[2] = S * (SRCB)[2];			\
} while (0)

#define ACC_SCALE_SCALAR_3V( DST, S, SRCB )	\
do {						\
      (DST)[0] += S * (SRCB)[0];		\
      (DST)[1] += S * (SRCB)[1];		\
      (DST)[2] += S * (SRCB)[2];		\
} while (0)

#define SELF_SCALE_SCALAR_3V( DST, S )		\
do {						\
      (DST)[0] *= S;				\
      (DST)[1] *= S;				\
      (DST)[2] *= S;				\
} while (0)

#define ACC_SCALAR_3V( DST, S ) 		\
do {						\
      (DST)[0] += S;				\
      (DST)[1] += S;				\
      (DST)[2] += S;				\
} while (0)

/* And also for 2-vectors
 */
#define SUB_2V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] - (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] - (SRCB)[1];		\
} while (0)

#define ADD_2V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] + (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] + (SRCB)[1];		\
} while (0)

#define SCALE_2V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] = (SRCA)[0] * (SRCB)[0];		\
      (DST)[1] = (SRCA)[1] * (SRCB)[1];		\
} while (0)

#define ACC_2V( DST, SRC )			\
do {						\
      (DST)[0] += (SRC)[0];			\
      (DST)[1] += (SRC)[1];			\
} while (0)

#define ACC_SCALE_2V( DST, SRCA, SRCB )		\
do {						\
      (DST)[0] += (SRCA)[0] * (SRCB)[0];	\
      (DST)[1] += (SRCA)[1] * (SRCB)[1];	\
} while (0)

#define SCALE_SCALAR_2V( DST, S, SRCB ) 	\
do {						\
      (DST)[0] = S * (SRCB)[0];			\
      (DST)[1] = S * (SRCB)[1];			\
} while (0)

#define ACC_SCALE_SCALAR_2V( DST, S, SRCB )	\
do {						\
      (DST)[0] += S * (SRCB)[0];		\
      (DST)[1] += S * (SRCB)[1];		\
} while (0)

#define SELF_SCALE_SCALAR_2V( DST, S )		\
do {						\
      (DST)[0] *= S;				\
      (DST)[1] *= S;				\
} while (0)

#define ACC_SCALAR_2V( DST, S ) 		\
do {						\
      (DST)[0] += S;				\
      (DST)[1] += S;				\
} while (0)



/*
 * Copy a vector of 4 GLubytes from SRC to DST.
 */
#if defined(__i386__)
#define COPY_4UBV(DST, SRC)			\
do {						\
   *((GLuint*)(DST)) = *((GLuint*)(SRC));	\
} while (0)
#else
/* The GLuint cast might fail if DST or SRC are not dword-aligned (RISC) */
#define COPY_4UBV(DST, SRC)			\
do {						\
   (DST)[0] = (SRC)[0];				\
   (DST)[1] = (SRC)[1];				\
   (DST)[2] = (SRC)[2];				\
   (DST)[3] = (SRC)[3];				\
} while (0)
#endif



/* Assign scalers to short vectors: */
#define ASSIGN_2V( V, V0, V1 )  \
do { V[0] = V0;  V[1] = V1; } while(0)

#define ASSIGN_3V( V, V0, V1, V2 )  \
do { V[0] = V0;  V[1] = V1;  V[2] = V2; } while(0)

#define ASSIGN_4V( V, V0, V1, V2, V3 ) 		\
do { 						\
    V[0] = V0;					\
    V[1] = V1;					\
    V[2] = V2;					\
    V[3] = V3; 					\
} while(0)




/* Absolute value (for Int, Float, Double): */
#define ABSI(X)  ((X) < 0 ? -(X) : (X))
#define ABSF(X)  ((X) < 0.0F ? -(X) : (X))
#define ABSD(X)  ((X) < 0.0 ? -(X) : (X))



/* Round a floating-point value to the nearest integer: */
#define ROUNDF(X)  ( (X)<0.0F ? ((GLint) ((X)-0.5F)) : ((GLint) ((X)+0.5F)) )


/* Compute ceiling of integer quotient of A divided by B: */
#define CEILING( A, B )  ( (A) % (B) == 0 ? (A)/(B) : (A)/(B)+1 )


/* Clamp X to [MIN,MAX]: */
#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )

/* Assign X to CLAMP(X, MIN, MAX) */
#define CLAMP_SELF(x, mn, mx)  \
   ( (x)<(mn) ? ((x) = (mn)) : ((x)>(mx) ? ((x)=(mx)) : (x)) )



/* Min of two values: */
#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )


/* MAX of two values: */
#define MAX2( A, B )   ( (A)>(B) ? (A) : (B) )

/* Dot product of two 2-element vectors */
#define DOT2( a, b )  ( (a)[0]*(b)[0] + (a)[1]*(b)[1] )

/* Dot product of two 3-element vectors */
#define DOT3( a, b )  ( (a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2] )


/* Dot product of two 4-element vectors */
#define DOT4( a, b )  ( (a)[0]*(b)[0] + (a)[1]*(b)[1] + \
			(a)[2]*(b)[2] + (a)[3]*(b)[3] )

#define DOT4V(v,a,b,c,d) (v[0]*a + v[1]*b + v[2]*c + v[3]*d)


#define CROSS3(n, u, v) 			\
do {						\
   (n)[0] = (u)[1]*(v)[2] - (u)[2]*(v)[1]; 	\
   (n)[1] = (u)[2]*(v)[0] - (u)[0]*(v)[2]; 	\
   (n)[2] = (u)[0]*(v)[1] - (u)[1]*(v)[0];	\
} while (0)


/*
 * Integer / float conversion for colors, normals, etc.
 */

#define BYTE_TO_UBYTE(b)   (b < 0 ? 0 : (GLubyte) b)
#define SHORT_TO_UBYTE(s)  (s < 0 ? 0 : (GLubyte) (s >> 7))
#define USHORT_TO_UBYTE(s)              (GLubyte) (s >> 8)
#define INT_TO_UBYTE(i)    (i < 0 ? 0 : (GLubyte) (i >> 23))
#define UINT_TO_UBYTE(i)                (GLubyte) (i >> 24)

/* Convert GLubyte in [0,255] to GLfloat in [0.0,1.0] */
#define UBYTE_TO_FLOAT(B)	((GLfloat) (B) * (1.0F / 255.0F))

/* Convert GLfloat in [0.0,1.0] to GLubyte in [0,255] */
#define FLOAT_TO_UBYTE(X)	((GLubyte) (GLint) (((X)) * 255.0F))


/* Convert GLbyte in [-128,127] to GLfloat in [-1.0,1.0] */
#define BYTE_TO_FLOAT(B)	((2.0F * (B) + 1.0F) * (1.0F/255.0F))

/* Convert GLfloat in [-1.0,1.0] to GLbyte in [-128,127] */
#define FLOAT_TO_BYTE(X)	( (((GLint) (255.0F * (X))) - 1) / 2 )


/* Convert GLushort in [0,65536] to GLfloat in [0.0,1.0] */
#define USHORT_TO_FLOAT(S)	((GLfloat) (S) * (1.0F / 65535.0F))

/* Convert GLfloat in [0.0,1.0] to GLushort in [0,65536] */
#define FLOAT_TO_USHORT(X)	((GLushort) (GLint) ((X) * 65535.0F))


/* Convert GLshort in [-32768,32767] to GLfloat in [-1.0,1.0] */
#define SHORT_TO_FLOAT(S)	((2.0F * (S) + 1.0F) * (1.0F/65535.0F))

/* Convert GLfloat in [0.0,1.0] to GLshort in [-32768,32767] */
#define FLOAT_TO_SHORT(X)	( (((GLint) (65535.0F * (X))) - 1) / 2 )


/* Convert GLuint in [0,4294967295] to GLfloat in [0.0,1.0] */
#define UINT_TO_FLOAT(U)	((GLfloat) (U) * (1.0F / 4294967295.0F))

/* Convert GLfloat in [0.0,1.0] to GLuint in [0,4294967295] */
#define FLOAT_TO_UINT(X)	((GLuint) ((X) * 4294967295.0))


/* Convert GLint in [-2147483648,2147483647] to GLfloat in [-1.0,1.0] */
#define INT_TO_FLOAT(I)		((2.0F * (I) + 1.0F) * (1.0F/4294967294.0F))

/* Convert GLfloat in [-1.0,1.0] to GLint in [-2147483648,2147483647] */
/* causes overflow:
#define FLOAT_TO_INT(X)		( (((GLint) (4294967294.0F * (X))) - 1) / 2 )
*/
/* a close approximation: */
#define FLOAT_TO_INT(X)		( (GLint) (2147483647.0 * (X)) )



/* Generic color packing macros
 */

#define PACK_COLOR_8888( a, b, c, d )					\
   (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define PACK_COLOR_888( a, b, c )					\
   (((a) << 16) | ((b) << 8) | (c))

#define PACK_COLOR_565( a, b, c )					\
   ((((a) & 0xf8) << 8) | (((b) & 0xfc) << 3) | (((c) & 0xf8) >> 3))

#define PACK_COLOR_1555( a, b, c, d )					\
   ((((b) & 0xf8) << 7) | (((c) & 0xf8) << 2) | (((d) & 0xf8) >> 3) |	\
    ((a) ? 0x8000 : 0))

#define PACK_COLOR_4444( a, b, c, d )					\
   ((((a) & 0xf0) << 8) | (((b) & 0xf0) << 4) | ((c) & 0xf0) | ((d) >> 4))

#define PACK_COLOR_88( a, b )						\
   (((a) << 8) | (b))

#define PACK_COLOR_332( a, b, c )					\
   (((a) & 0xe0) | (((b) & 0xe0) >> 3) | (((c) & 0xc0) >> 6))


#endif /*MACROS_H*/
