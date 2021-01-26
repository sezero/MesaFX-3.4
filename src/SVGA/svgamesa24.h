/* $Id: svgamesa24.h,v 1.2 2000/01/22 20:08:36 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.2
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


#ifndef SVGA_MESA_24_H
#define SVGA_MESA_24_H

extern void __set_color24( GLcontext *ctx, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );
extern void __clear_color24( GLcontext *ctx, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );
extern GLbitfield __clear24( GLcontext *ctx, GLbitfield mask, GLboolean all, GLint x, GLint y, GLint width, GLint height );
extern void __write_rgba_span24( const GLcontext *ctx, GLuint n, GLint x, GLint y, const GLubyte rgba[][4], const GLubyte mask[] );
extern void __write_mono_rgba_span24( const GLcontext *ctx, GLuint n, GLint x, GLint y, const GLubyte mask[]);
extern void __read_rgba_span24( const GLcontext *ctx, GLuint n, GLint x, GLint y, GLubyte rgba[][4] );
extern void __write_rgba_pixels24( const GLcontext *ctx, GLuint n, const GLint x[], const GLint y[], const GLubyte rgba[][4], const GLubyte mask[] );
extern void __write_mono_rgba_pixels24( const GLcontext *ctx, GLuint n, const GLint x[], const GLint y[], const GLubyte mask[] );
extern void __read_rgba_pixels24( const GLcontext *ctx, GLuint n, const GLint x[], const GLint y[], GLubyte rgba[][4], const GLubyte mask[] );

#endif /* SVGA_MESA_24_H */

