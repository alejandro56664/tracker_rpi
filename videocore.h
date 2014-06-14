#ifndef VIDEOCORE_H
#define VIDEOCORE_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include <QImage>
#include <QPushButton>
#include <QLabel>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/video/tracking.hpp"

#include <iostream>
#include <ctype.h>
#include <cmath>

#include <tracklist.h>


#define MIN_RAD	10
#define NUM_FRAMES_HC 5
#define MAX_POINTS 5

using namespace cv;
using namespace std;

class VideoCore : public QThread
{    Q_OBJECT
 private:
    bool stop;
    bool loop;
    bool ended;
    bool addPt;
    bool rmPt;
    bool autoTracking;
    QMutex mutex;
    QWaitCondition condition;
    Mat frame;
    Mat lastframe; ///MEJORAR EL ACCESO!!!!!!!!!
    int frameRate;
    int frameRateRep;
    VideoCapture *capture;
    string videoUrl;
    Mat gray, prevGray, image;
    Point2f point;
    vector<Point2f> points[2];

     int index;

 signals:
 //Signal to output frame to be displayed
      //void processedImage(const QImage &image);

    // void onEnterFrame(const Mat* frame);
     void onEnterFrame( Mat* frame);
     void onNextFrame();
 protected:
     void run();
     void msleep(int ms);

 private slots:
     void addpointtotrack(int index);
     void rmpointtotrack(int index);
 public:
    TrackList* Trackers;
    //Constructor
    VideoCore( TrackList *trackers, QObject *parent = 0);
    //Destructor
    ~VideoCore();
    //Load a video from memory
    bool loadVideo(string filename);
    //Play the video
    void Play();
    //Stop the video
    void Pause();

    void setTrackList(TrackList* trackers);
    void updateTracks();
    void setLoop(bool active);
    void update();
    void setCurrentFrame( int frameNumber);
    void setPercentRate(int PercentRate);
     //Get video properties
    double getFrameRate();
    double getCurrentFrame();
    double getNumberOfFrames();

    Mat* getCurrentImage();
    //check if the player has been stopped
    bool isStopped() const;

    void pointTracking(Mat* pframe);
    void searchCircle();
};
#endif // VIDEOPLAYER_H
