/* $Id: texture.h,v 1.3 2000/06/27 21:42:13 brianp Exp $ */

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





#ifndef TEXTURE_H
#define TEXTURE_H


#include "types.h"


extern void
_mesa_set_texture_sampler( struct gl_texture_object *t );

extern void gl_init_texture( void );

extern void gl_update_texture_unit( GLcontext *ctx, 
				    struct gl_texture_unit *texUnit );


extern void gl_texture_pixels( GLcontext *ctx, GLuint texSet, GLuint n,
                               const GLfloat s[], const GLfloat t[],
                               const GLfloat r[], GLfloat lambda[],
                               GLubyte primary_rgba[][4], GLubyte rgba[][4] );


#endif

