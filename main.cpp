#include "mainwindow.h"

#include <QApplication>
#include <QStringList>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString filename;
    QStringList args = a.arguments(); args.removeFirst();
    if (args.length() > 0) filename = args.join(" "); else filename = "settings.yaml";
    MainWindow w(filename);
    w.show();
    return a.exec();
}
