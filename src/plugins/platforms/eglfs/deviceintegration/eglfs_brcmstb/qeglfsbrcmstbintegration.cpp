/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qeglfsbrcmstbintegration.h"
#include <v3dplatform.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <private/qcore_unix_p.h>

QT_BEGIN_NAMESPACE

static V3D_PlatformHandle s_platformHandle = 0;

void QEglFSBrcmSTBIntegration::platformInit()
{
    QEglFSDeviceIntegration::platformInit();

    V3D_RegisterDisplayPlatform(&s_platformHandle, EGL_DEFAULT_DISPLAY);

    struct fb_var_screeninfo vinfo;
    int xres = 1280;
    int yres = 720;
    int bits_per_pixel = 32;
    int fd = qt_safe_open("/dev/fb0", O_RDWR, 0);
    if (fd != -1) {
        if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
            qWarning("eglconvenience: Could not read screen info");
        } else {
            xres = vinfo.xres;
            yres = vinfo.yres;
            bits_per_pixel = vinfo.bits_per_pixel;
        }
    }
    qt_safe_close(fd);
    mScreenSize.setWidth(xres);
    mScreenSize.setHeight(yres);
    mScreenDepth = bits_per_pixel;
}

EGLNativeDisplayType QEglFSBrcmSTBIntegration::platformDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

void QEglFSBrcmSTBIntegration::platformDestroy()
{
    V3D_UnregisterDisplayPlatform(s_platformHandle);
}

QSize QEglFSBrcmSTBIntegration::screenSize() const
{
    return mScreenSize;
}

int QEglFSBrcmSTBIntegration::screenDepth() const
{
    return mScreenDepth;
}

QImage::Format QEglFSBrcmSTBIntegration::screenFormat() const
{
    return mScreenDepth == 32 ? QImage::Format_ARGB32 : QImage::Format_RGB16;
}

qreal QEglFSBrcmSTBIntegration::refreshRate() const
{
    return 50;
}

QSurfaceFormat QEglFSBrcmSTBIntegration::surfaceFormatFor(const QSurfaceFormat &inputFormat) const
{
    QSurfaceFormat format = inputFormat;
    if (mScreenDepth == 32) {
        format.setAlphaBufferSize(8);
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
    }
    return format;
}

EGLNativeWindowType QEglFSBrcmSTBIntegration::createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format)
{
    Q_UNUSED(window)
    Q_UNUSED(size)
    Q_UNUSED(format)

    V3D_NativeWindowInfo info;

    info.x = 0;
    info.y = 0;
    info.width = mScreenSize.width();
    info.height = mScreenSize.height();
    info.stretch = false;
    info.clientID = 0;
    info.zOrder = 0;

    return V3D_CreateNativeWindow(&info);
}

void QEglFSBrcmSTBIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    V3D_DestroyNativeWindow(window);
}

QT_END_NAMESPACE
