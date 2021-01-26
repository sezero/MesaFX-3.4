/* $Id: fxmesa.h,v 1.2 1999/11/24 18:44:53 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 * Copyright (C) 1995-1999  Brian Paul
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
 * FXMesa - 3Dfx Glide driver for Mesa.  Contributed by David Bucciarelli
 *
 * NOTE: This version requires Glide 2.3 or later.
 */


#ifndef FXMESA_H
#define FXMESA_H


#include <glide.h>


#ifdef __cplusplus
extern "C" {
#endif


#define FXMESA_MAJOR_VERSION 3
#define FXMESA_MINOR_VERSION 3


/*
 * Values for attribList parameter to fxMesaCreateContext():
 */
#define FXMESA_NONE		0	/* to terminate attribList */
#define FXMESA_DOUBLEBUFFER	10
#define FXMESA_ALPHA_SIZE	11      /* followed by an integer */
#define FXMESA_DEPTH_SIZE	12      /* followed by an integer */
#define FXMESA_STENCIL_SIZE	13      /* followed by an integer */
#define FXMESA_ACCUM_SIZE	14      /* followed by an integer */



typedef struct tfxMesaContext *fxMesaContext;


#if defined (__BEOS__)
#pragma export on
#endif


GLAPI fxMesaContext GLAPIENTRY fxMesaCreateContext(GLuint win, GrScreenResolution_t,
						  GrScreenRefresh_t,
						  const GLint attribList[]);

GLAPI fxMesaContext GLAPIENTRY fxMesaCreateBestContext(GLuint win,
						      GLint width, GLint height,
						      const GLint attribList[]);
GLAPI void GLAPIENTRY fxMesaDestroyContext(fxMesaContext ctx);

GLAPI GLboolean GLAPIENTRY fxMesaSelectCurrentBoard(int n);

GLAPI void GLAPIENTRY fxMesaMakeCurrent(fxMesaContext ctx);

GLAPI fxMesaContext GLAPIENTRY fxMesaGetCurrentContext(void);

GLAPI void GLAPIENTRY fxMesaSwapBuffers(void);

GLAPI void GLAPIENTRY fxMesaSetNearFar(GLfloat nearVal, GLfloat farVal);

GLAPI void GLAPIENTRY fxMesaUpdateScreenSize(fxMesaContext ctx);

GLAPI int GLAPIENTRY fxQueryHardware(void);

GLAPI void GLAPIENTRY fxCloseHardware(void);

#ifdef __MSDOS__ /* O.S. - added for DXE3 (DJGPP 2.04+) builds */
typedef void (*fxMesaProc) ();
fxMesaProc fxMesaGetProcAddress (const char * sym);
#endif

#if defined (__BEOS__)
#pragma export off
#endif


#ifdef __cplusplus
}
#endif


#endif
