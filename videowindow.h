#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>

#include <QSlider>
#include <QScrollArea>
#include <QCheckBox>
#include <QAction>
#include <QMenu>
#include <QShortcut>

#include "cvimagewidget.h"
#include "videocore.h"
#include "segment.h"
#include "track.h"
#include "tracklist.h"

#define TOOL_NONE    0
#define TOOL_TRACKER 1
#define TOOL_SEGMENT 2
#define TOOL_MANUALPATH 3

class VideoWindow : public QWidget
{
    Q_OBJECT

public:
    //explicit VideoWindow(QWidget *parent = 0);
   VideoWindow(QWidget *parent = 0,VideoCore* _player=NULL, TrackList* lista_track=NULL, SegmentList* lista_seg=NULL);
    ~VideoWindow();

    void setVideoPlayer(VideoCore* _player);
    void setTrackList(TrackList* lista);
    void setSegmentList(SegmentList *lista);
    void hideControls();
    void showControls();
    void openfile(string filename);
    void setTool(int  tooltype);
private slots:

    QString getFormattedTime(int timeInSeconds);
    void on_horizontalSlider_sliderPressed();
    void on_horizontalSlider_sliderReleased();
    void on_horizontalSlider_sliderMoved(int position);

    void on_percentRate_sliderMoved(int position);


    //Display video frame in player UI
    void updateScreen(Mat *frame);
    void on_playButton_clicked();

    void on_nextButton_clicked();
    void on_prevButton_clicked();
    void on_stopButton_clicked();

    void on_loopCheck_clicked();
    void deleteCurrentObject();

    void on_screen_mouseDown(Point2i act_point);
    void on_screen_mouseDrag(Point2i act_point);
    void on_screen_mouseUp(Point2i act_point);

    //acciones globales
    void showXaxis();
    void showYaxis();
    void setOrigin();

    //acciones asociadas a segmentos
    void setLabel();
    void setMeasure();
    void showLabel();
    void showValue();
    void setEndTrack();

private:
    //QWidget *window;
    //QImage *drawable;

    CVImageWidget* screen;
    QScrollArea *scrollArea;

    int currentTool;
    QPushButton* bn_play;
    QPushButton* bn_next;
    QPushButton* bn_prev;
    QPushButton* bn_stop;
    QCheckBox* cb_loop;

    QSlider *sl_timeLine;
    QSlider *sl_percentRate;
    QLabel *lb_videotime;
   // QLabel *lb_videovel;
    SegmentList* lista_segmentos;
    TrackList* lista_trackers;

    VideoCore* Player;


    Point2i tmp_point; //se almacena de forma temporal el punto en donde el usuario hace click

    Segment* axisX;
    Segment* axisY;
    Point2i origin;
    bool isXaxisVisible;
    bool isYaxisVisible;

    //acciones globales
    QAction *setOriginAct;
    QAction *showXaxisAct;
    QAction *showYaxisAct;

    //acciones asociadas a segmentos
    QAction *setMeasureAct;
    QAction *setLabelAct;
    QAction *showLabelAct;
    QAction *showValueAct;
    QAction *setEndTrackAct;
    QAction *deleteCurrentObjectAct;

    void drawSegment(Mat* frame);
    void drawTracker(Mat* frame);
    void initScreen();
    void initMediaPlayer();
    void createActions();
    void contextMenuEvent(QContextMenuEvent *event);


};

#endif // MAINWINDOW_H
