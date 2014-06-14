#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QListWidget>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>

#include "track.h"
#include "plotwindow.h"
#include "videowindow.h"
#include "videocore.h"

class QAction;
class QListWidget;
class QMenu;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();
    VideoCore* player;
    TrackList* trackers;
    SegmentList* segments;
    VideoWindow *videowin;
    void setLowRes();
private slots:
    void open();
    void save();
    void selectCursor(bool state);
    void about();
    void selectNewSegment(bool state);
    void selectNewTracker(bool state);
    void selectManualTrackAct(bool state);
    void selectAutoDetectAct();
    void changeLowResWin();
private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();

    void read_segment();
    void read_tracker();
    void read_tracker_path(vector<Point2i> *path, int size);

    PlotWindow  *plotwin;


    QXmlStreamReader Rxml;

    QDockWidget *dock1, *dock2;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QAction *saveAct;
    QAction *openAct;
    QAction *cursorAct;
    QAction *addTrackerAct;
    QAction *addSegmentAct;
    QAction *manualTrackAct;
    QAction *autoDetectAct;
    QAction *viewLowRes;
    QAction *viewGraphAct;
    QAction *viewVideoAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;


};

#endif
