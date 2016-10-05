/*
Copyright (C) 2009-2013 Baker

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// gl_renderer.c


#include "quakedef.h"



// The function set provided by the wrapper
void (APIENTRY *eglAlphaFunc) (GLenum func, GLclampf ref);
void (APIENTRY *eglBegin) (GLenum mode);
void (APIENTRY *eglBindTexture) (GLenum target, GLuint texture);
void (APIENTRY *eglBlendFunc) (GLenum sfactor, GLenum dfactor);
void (APIENTRY *eglClear) (GLbitfield mask);
void (APIENTRY *eglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (APIENTRY *eglClearStencil) (GLint s);
void (APIENTRY *eglColor3f) (GLfloat red, GLfloat green, GLfloat blue);
void (APIENTRY *eglColor3fv) (const GLfloat *v);
void (APIENTRY *eglColor3ubv) (const GLubyte *v);
void (APIENTRY *eglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void (APIENTRY *eglColor4fv) (const GLfloat *v);
void (APIENTRY *eglColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void (APIENTRY *eglColor4ubv) (const GLubyte *v);

// Baker: eglCopyTexSubImage2D is not implemented in wrapper.  I suspect because r_oldwater 0 in Fitz achieves what it is doing
// in a very strange way that would require an unreasonable amount of work and this function was only used there.  I doubt
// this function presented any kind of challenge.

void (APIENTRY *eglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void (APIENTRY *eglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void (APIENTRY *eglCullFace) (GLenum mode);
void (APIENTRY *eglDeleteTextures) (GLsizei n, const GLuint *textures);
void (APIENTRY *eglDepthFunc) (GLenum func);
void (APIENTRY *eglDepthMask) (GLboolean flag);
void (APIENTRY *eglDepthRange) (GLclampd zNear, GLclampd zFar);
void (APIENTRY *eglDisable) (GLenum cap);
void (APIENTRY *eglDrawBuffer) (GLenum mode);
void (APIENTRY *eglEnable) (GLenum cap);
void (APIENTRY *eglEnd) (void);
void (APIENTRY *eglFinish) (void);
void (APIENTRY *eglFogf) (GLenum pname, GLfloat param);
void (APIENTRY *eglFogfv) (GLenum pname, const GLfloat *params);
void (APIENTRY *eglFogi) (GLenum pname, GLint param);
void (APIENTRY *eglFogiv) (GLenum pname, const GLint *params);
void (APIENTRY *eglFrontFace) (GLenum mode);
void (APIENTRY *eglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void (APIENTRY *eglGenTextures) (GLsizei n, GLuint *textures);
void (APIENTRY *eglGetFloatv) (GLenum pname, GLfloat *params);
void (APIENTRY *eglGetIntegerv) (GLenum pname, GLint *params);
const GLubyte *(APIENTRY *eglGetString) (GLenum name);
void (APIENTRY *eglGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void (APIENTRY *eglGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);void (APIENTRY *eglHint) (GLenum target, GLenum mode);
void (APIENTRY *eglLineWidth) (GLfloat width);
void (APIENTRY *eglLoadIdentity) (void);
void (APIENTRY *eglLoadMatrixf) (const GLfloat *m);
void (APIENTRY *eglMatrixMode) (GLenum mode);
void (APIENTRY *eglMultMatrixf) (const GLfloat *m);
void (APIENTRY *eglNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
void (APIENTRY *eglOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void (APIENTRY *eglPixelStorei) (GLenum pname, GLint param);
void (APIENTRY *eglPolygonMode) (GLenum face, GLenum mode);
void (APIENTRY *eglPolygonOffset) (GLfloat factor, GLfloat units);
void (APIENTRY *eglPopMatrix) (void);
void (APIENTRY *eglPushMatrix) (void);
void (APIENTRY *eglReadBuffer) (GLenum mode);
void (APIENTRY *eglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void (APIENTRY *eglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglScalef) (GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
void (APIENTRY *eglSelectBuffer) (GLsizei size, GLuint *buffer);
void (APIENTRY *eglShadeModel) (GLenum mode);
void (APIENTRY *eglStencilFunc) (GLenum func, GLint ref, GLuint mask);
void (APIENTRY *eglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
void (APIENTRY *eglTexCoord2f) (GLfloat s, GLfloat t);
void (APIENTRY *eglTexCoord2fv) (const GLfloat *v);
void (APIENTRY *eglTexEnvf) (GLenum target, GLenum pname, GLfloat param);
void (APIENTRY *eglTexEnvi) (GLenum target, GLenum pname, GLint param);
void (APIENTRY *eglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (APIENTRY *eglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
void (APIENTRY *eglTexParameteri) (GLenum target, GLenum pname, GLint param);
void (APIENTRY *eglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void (APIENTRY *eglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglVertex2f) (GLfloat x, GLfloat y);
void (APIENTRY *eglVertex2fv) (const GLfloat *v);
void (APIENTRY *eglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglVertex3fv) (const GLfloat *v);
void (APIENTRY *eglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef _WIN32

LONG (WINAPI *eChangeDisplaySettings) (LPDEVMODE lpDevMode, DWORD dwflags);

HGLRC (WINAPI *ewglCreateContext) (HDC);
BOOL  (WINAPI *ewglDeleteContext) (HGLRC);
HGLRC (WINAPI *ewglGetCurrentContext) (VOID);
HDC   (WINAPI *ewglGetCurrentDC) (VOID);
PROC  (WINAPI *ewglGetProcAddress)(LPCSTR);
BOOL  (WINAPI *ewglMakeCurrent) (HDC, HGLRC);
BOOL  (WINAPI *eSetPixelFormat) (HDC, int, CONST PIXELFORMATDESCRIPTOR *);
#endif

#ifndef DIRECT3D_WRAPPER

void VID_Renderer_Set_OpenGL (void)
{
	eglAlphaFunc            = glAlphaFunc;
	eglBegin                = glBegin;
	eglBindTexture          = glBindTexture;
	eglBlendFunc            = glBlendFunc;
	eglClear                = glClear;
	eglClearColor           = glClearColor;
	eglClearStencil         = glClearStencil;
	eglColor3f              = glColor3f;
	eglColor3fv             = glColor3fv;
	eglColor3ubv            = glColor3ubv;
	eglColor4f              = glColor4f;
	eglColor4fv             = glColor4fv;
	eglColor4ub             = glColor4ub;
	eglColor4ubv            = glColor4ubv;
	eglColorMask            = glColorMask;

	eglCopyTexSubImage2D	= glCopyTexSubImage2D;

	eglCullFace             = glCullFace;
	eglDeleteTextures       = glDeleteTextures;
	eglDepthFunc            = glDepthFunc;
	eglDepthMask            = glDepthMask;
	eglDepthRange           = glDepthRange;
	eglDisable              = glDisable;
	eglDrawBuffer           = glDrawBuffer;
	eglEnable               = glEnable;
	eglEnd                  = glEnd;
	eglFinish               = glFinish;
	eglFogf                 = glFogf;
	eglFogfv                = glFogfv;
	eglFogi                 = glFogi;
	eglFogiv                = glFogiv;
	eglFrontFace            = glFrontFace;
	eglFrustum              = glFrustum;
	eglGenTextures          = glGenTextures;
	eglGetFloatv            = glGetFloatv;
	eglGetIntegerv          = glGetIntegerv;
	eglGetString            = glGetString;
	eglGetTexImage          = glGetTexImage;
	eglGetTexParameterfv    = glGetTexParameterfv;
	eglHint                 = glHint;
	eglLineWidth            = glLineWidth;
	eglLoadIdentity         = glLoadIdentity;
	eglLoadMatrixf          = glLoadMatrixf;
	eglMatrixMode           = glMatrixMode;
	eglMultMatrixf          = glMultMatrixf;
	eglNormal3f             = glNormal3f;
	eglOrtho                = glOrtho;
	eglPixelStorei			= glPixelStorei;
	eglPolygonMode          = glPolygonMode;
	eglPolygonOffset        = glPolygonOffset;
	eglPopMatrix            = glPopMatrix;
	eglPushMatrix           = glPushMatrix;
	eglReadBuffer           = glReadBuffer;
	eglReadPixels           = glReadPixels;
	eglRotatef              = glRotatef;
	eglScalef               = glScalef;
	eglScissor              = glScissor;
	eglShadeModel           = glShadeModel;
	eglStencilFunc          = glStencilFunc;
	eglStencilOp            = glStencilOp;
	eglTexCoord2f           = glTexCoord2f;
	eglTexCoord2fv          = glTexCoord2fv;
	eglTexEnvf              = glTexEnvf;
	eglTexEnvi              = glTexEnvi;
	eglTexImage2D           = glTexImage2D;
	eglTexParameterf        = glTexParameterf;
	eglTexParameteri        = glTexParameteri;
	eglTexSubImage2D        = glTexSubImage2D;
	eglTranslatef           = glTranslatef;
	eglVertex2f             = glVertex2f;
	eglVertex2fv            = glVertex2fv;
	eglVertex3f             = glVertex3f;
	eglVertex3fv            = glVertex3fv;
	eglViewport             = glViewport;

#ifdef _WIN32
	ewglCreateContext       = wglCreateContext;
	ewglDeleteContext       = wglDeleteContext;
	ewglGetCurrentContext   = wglGetCurrentContext;
	ewglGetCurrentDC        = wglGetCurrentDC;
	ewglMakeCurrent         = wglMakeCurrent;
	ewglGetProcAddress		= wglGetProcAddress;

	eSetPixelFormat         = SetPixelFormat;

	eChangeDisplaySettings  = ChangeDisplaySettings;
#endif

	vid.direct3d = 0;

}

#endif // ! DIRECT3D_WRAPPER

#ifdef DIRECT3D_WRAPPER

void VID_Renderer_Set_Direct3D (void)
{
	eglAlphaFunc            = d3dmh_glAlphaFunc;
	eglBegin                = d3dmh_glBegin;
	eglBindTexture          = d3dmh_glBindTexture;
	eglBlendFunc            = d3dmh_glBlendFunc;
	eglClear                = d3dmh_glClear;
	eglClearColor           = d3dmh_glClearColor;
	eglClearStencil         = d3dmh_glClearStencil;
	eglColor3f              = d3dmh_glColor3f;
	eglColor3fv             = d3dmh_glColor3fv;
	eglColor3ubv            = d3dmh_glColor3ubv;
	eglColor4f              = d3dmh_glColor4f;
	eglColor4fv             = d3dmh_glColor4fv;
	eglColor4ub             = d3dmh_glColor4ub;
	eglColor4ubv            = d3dmh_glColor4ubv;
	eglColorMask            = d3dmh_glColorMask;

	eglCopyTexSubImage2D	= d3dmh_glCopyTexSubImage2D;

	eglCullFace             = d3dmh_glCullFace;
	eglDeleteTextures       = d3dmh_glDeleteTextures;
	eglDepthFunc            = d3dmh_glDepthFunc;
	eglDepthMask            = d3dmh_glDepthMask;
	eglDepthRange           = d3dmh_glDepthRange;
	eglDisable              = d3dmh_glDisable;
	eglDrawBuffer           = d3dmh_glDrawBuffer;
	eglEnable               = d3dmh_glEnable;
	eglEnd                  = d3dmh_glEnd;
	eglFinish               = d3dmh_glFinish;
	eglFogf                 = d3dmh_glFogf;
	eglFogfv                = d3dmh_glFogfv;
	eglFogi                 = d3dmh_glFogi;
	eglFogiv                = d3dmh_glFogiv;
	eglFrontFace            = d3dmh_glFrontFace;
	eglFrustum              = d3dmh_glFrustum;
	eglGenTextures          = d3dmh_glGenTextures;
	eglGetFloatv            = d3dmh_glGetFloatv;
	eglGetIntegerv          = d3dmh_glGetIntegerv;
	eglGetString            = d3dmh_glGetString;
	eglGetTexImage          = d3dmh_glGetTexImage;
	eglGetTexParameterfv    = d3dmh_glGetTexParameterfv;
	eglHint                 = d3dmh_glHint;
	eglLineWidth            = d3dmh_glLineWidth;
	eglLoadIdentity         = d3dmh_glLoadIdentity;
	eglLoadMatrixf          = d3dmh_glLoadMatrixf;
	eglMatrixMode           = d3dmh_glMatrixMode;
	eglMultMatrixf          = d3dmh_glMultMatrixf;
	eglNormal3f             = d3dmh_glNormal3f;
	eglOrtho                = d3dmh_glOrtho;
	eglPixelStorei			= d3dmh_glPixelStorei;
	eglPolygonMode          = d3dmh_glPolygonMode;
	eglPolygonOffset        = d3dmh_glPolygonOffset;
	eglPopMatrix            = d3dmh_glPopMatrix;
	eglPushMatrix           = d3dmh_glPushMatrix;
	eglReadBuffer           = d3dmh_glReadBuffer;
	eglReadPixels           = d3dmh_glReadPixels;
	eglRotatef              = d3dmh_glRotatef;
	eglScalef               = d3dmh_glScalef;
	eglScissor              = d3dmh_glScissor;
	eglShadeModel           = d3dmh_glShadeModel;
	eglStencilFunc          = d3dmh_glStencilFunc;
	eglStencilOp            = d3dmh_glStencilOp;
	eglTexCoord2f           = d3dmh_glTexCoord2f;
	eglTexCoord2fv          = d3dmh_glTexCoord2fv;
	eglTexEnvf              = d3dmh_glTexEnvf;
	eglTexEnvi              = d3dmh_glTexEnvi;
	eglTexImage2D           = d3dmh_glTexImage2D;
	eglTexParameterf        = d3dmh_glTexParameterf;
	eglTexParameteri        = d3dmh_glTexParameteri;
	eglTexSubImage2D        = d3dmh_glTexSubImage2D;
	eglTranslatef           = d3dmh_glTranslatef;
	eglVertex2f             = d3dmh_glVertex2f;
	eglVertex2fv            = d3dmh_glVertex2fv;
	eglVertex3f             = d3dmh_glVertex3f;
	eglVertex3fv            = d3dmh_glVertex3fv;
	eglViewport             = d3dmh_glViewport;

#ifdef _WIN32
	ewglCreateContext       = d3dmh_wglCreateContext;
	ewglDeleteContext       = d3dmh_wglDeleteContext;
	ewglGetCurrentContext   = d3dmh_wglGetCurrentContext;
	ewglGetCurrentDC        = d3dmh_wglGetCurrentDC;
	ewglMakeCurrent         = d3dmh_wglMakeCurrent;
	ewglGetProcAddress		= d3dmh_wglGetProcAddress;

	eSetPixelFormat         = d3dmh_SetPixelFormat;

	eChangeDisplaySettings  = ChangeDisplaySettings_FakeGL;
#endif


	vid.direct3d = 1;
}


#endif // DIRECT3D_WRAPPER

void VID_Renderer_Setup (void)
{

#ifdef DIRECT3D_WRAPPER
	VID_Renderer_Set_Direct3D ();
#else
	VID_Renderer_Set_OpenGL ();
#endif // DIRECT3D_WRAPPER

}

