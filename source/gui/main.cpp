#include "mainui.h"

#include <QApplication>
#include <QStyleFactory>

#include "qdatabrowser.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDataBrowser::initResources();

    // select fusion style if available
    // for consistent look among different platforms
    if (QStyleFactory::keys().contains("Fusion"))
        QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainUI w;
    w.setLocale(QLocale::c());
    w.show();
    return app.exec();
}
