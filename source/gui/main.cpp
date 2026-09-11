#include "mainui.h"

#include <QApplication>
#include <QStyleFactory>
#include <QScreen>

#include "qdatabrowser.h"

int main(int argc, char *argv[])
{
    // The 3D track viewport requires a desktop OpenGL 3.3 core profile
    // context, which ANGLE (Windows' GLES backend) cannot provide. Forcing
    // desktop OpenGL avoids depending on Qt's ANGLE DLLs (libEGL,
    // libGLESv2, d3dcompiler_47), which require a Visual C++ runtime that
    // may not be present on the target machine.
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication app(argc, argv);

    // QDataBrowser::initResources();

    // select fusion style if available
    // for consistent look among different platforms
    if (QStyleFactory::keys().contains("Fusion"))
        QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainUI w;
    w.setLocale(QLocale::c());
    w.show();

#ifdef Q_OS_WIN
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeom = screen->availableGeometry();
    w.move(screenGeom.center() - w.frameGeometry().center());
#endif

    return app.exec();
}
