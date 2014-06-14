/*
#include "videowindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoWindow* w = new VideoWindow();

    w->setAttribute(Qt::WA_DeleteOnClose, true);

    w->show();

    return a.exec();
}*/

#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    if (argc == 2){
         cout<<"su argumento es"<<argv[1]<<endl;
         mainWin.videowin->openfile(argv[1]);
         mainWin.setLowRes();
    }

    return app.exec();
}
