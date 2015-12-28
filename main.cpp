#include "mainwindow.h"
#include <QApplication>
#include <QProxyStyle>
#include <QStyleFactory>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStyle *style = new QProxyStyle(QStyleFactory::create("fusion"));
    a.setStyle(style);

    MainWindow w;
    w.show();

    return a.exec();
}
