#include "videocore.h"


VideoCore::VideoCore(TrackList* trackers, QObject *parent)
 : QThread(parent)
{
    qRegisterMetaType<Mat>("Mat");
    Trackers = trackers;
    connect(Trackers, SIGNAL(onNewTracker(int)),this, SLOT(addpointtotrack(int)));
    connect(Trackers, SIGNAL(onRemoveTracker(int)),this, SLOT(rmpointtotrack(int)));
    addPt = false;
    rmPt = false;
    autoTracking = true;
    loop = false;
    ended = false;
    stop = true;
}

bool VideoCore::loadVideo(string filename) {
    videoUrl = filename;
    capture  =  new cv::VideoCapture(filename);

       if (capture->isOpened())
       {
           frameRate = (int) ceil(capture->get(CV_CAP_PROP_FPS));
           frameRateRep = frameRate;
           return true;
       }
       else
           return false;
 }

void VideoCore::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(LowPriority);
    }

}


void VideoCore::run()
{
    int delay = (1000/frameRateRep);
    while(!stop){
        update();

        this->msleep(delay);
    }
}
    
void VideoCore::update(){

    if (!capture->read(frame)){
        setCurrentFrame(0);
        if(!loop){
            stop = true;
        }
         ended = true;
         autoTracking=false;
        /*if(loop){
           setCurrentFrame(0);
        }else{
        frame = lastframe;
        ended = true;
        stop = true;
        }*/
     }
    if (autoTracking){
        // proceso de tracking
        pointTracking(&frame);
    }
    //lastframe = frame;
    emit onEnterFrame(&frame);
    emit onNextFrame();

}

void VideoCore::updateTracks(){
    stop = true;
    autoTracking = true;
    setCurrentFrame(0);
    while(!capture->read(frame)){
        pointTracking(&frame);
        cout<<"calculando..."<<endl;
    }
    setCurrentFrame(0);
    stop = false;
}

void VideoCore::searchCircle(){
    vector<Point2f> good_points;
    Vec3i c_big;

    c_big[0] = 0;
    c_big[1] = 0;
    c_big[2] = MIN_RAD;

    //se reinicia
    stop=true;
    setCurrentFrame(0);
    //se captura el primeros NUM_FRAMES_HC  frames para detectar estimar que circulo representa el disco
    for(int i=0;i<NUM_FRAMES_HC;i++){
        capture->read(frame);
        frame.copyTo(image);
        medianBlur(image, image, 5);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        vector<Vec3f> circles;
        HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1,
                 gray.rows, //tenemos la certeza que solo habra a lo sumo un circulo en la imagen
                 100, 50, MIN_RAD, gray.rows/3 // change the last two parameters
                                // (min_radius & max_radius) to detect larger circles
                 );
        cout<<"(VideoCore::searchCircle) se detectaron "<<circles.size()<<" circulos"<<endl;
        for( size_t i = 0; i < circles.size(); i++ )
        {
            Vec3i c = circles[i];
            //en caso de reconocer varios
            if ( c[2] > c_big[2]){
                c_big[0] = c[0];
                    c_big[1] = c[1];
                    c_big[2] = c[2];
            }
        }
    }
    int ratio = (c_big[2]*71)/100;
    int side = 2*ratio;

    Mat mask = Mat::zeros(image.size(), CV_8U);  // type of mask is CV_8U
    Mat roi(mask, cv::Rect(c_big[0]-ratio,c_big[1]-ratio,side,side));
    roi = Scalar(255, 255, 255);

    goodFeaturesToTrack(gray, good_points, MAX_POINTS, 0.01, 10, mask, 3, 0, 0.04);
    Point2i tmp, tmp2;
    tmp.x = c_big[0];
    tmp.y = c_big[1];
    int index_min = 0;
    double min = 100.0;
    for(int i=0;i<MAX_POINTS;i++){
        tmp2 = good_points[i];
    double d = norm(tmp - tmp2);
    if(d<min) {
        min = d;
        index_min = i;}
    }
    cout<<"el mas cercano es: "<<index_min<<endl;
    //se agrega el tracker
    Trackers->addTrack(new Track("barra", good_points[index_min],this->getCurrentFrame(), Scalar(100,100,250)));
    //se reinicia
    setCurrentFrame(0);
    stop=false;
}
void VideoCore::pointTracking(Mat* pframe){
    ///esto deberia ir en otro lugar, solo deberia ejecutarse una vez
    TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
    Size winSize(31,31);
    Track* tmp;
    pframe->copyTo(image);
    cvtColor(image, gray, COLOR_BGR2GRAY);

    if( !points[0].empty() )
    {
        vector<uchar> status;
        vector<float> err;
        if(prevGray.empty())
            gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
                             3, termcrit, 0, 0.001);
        size_t i, k;
        for( i = k = 0; i < points[1].size(); i++ )
        {
            if( !status[i] )
                continue;

            points[1][k++] = points[1][i];
            cout<<"x: "<<points[1][i].x<<" y: "<<points[1][i].y<<endl;
            //Trackers->searchByIndex(i)->setPosinPath(getCurrentFrame(),points[1][i]);
            tmp = Trackers->searchByIndex(i);
            tmp->path_px.push_back(points[1][i]);
            //circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
        }
        points[1].resize(k);
        if(rmPt){
            if (index>0){
               // points[1].erase(points[1].begin()+index);
            }
            cout<<"se va eliminar un elemento. Nuevo tamaño: "<<points[1].size()<<" Index: "<<index<<endl;
            rmPt = false;
        }

    }

    if (addPt){
        vector<Point2f> tmp;
        Point2i* p;
        p = Trackers->searchByIndex(index)->getCenter();
        point.x = (float)p->x;
        point.y = (float)p->y;
        tmp.push_back(point);
        cornerSubPix( gray, tmp, winSize, cvSize(-1,-1), termcrit);
        points[1].push_back(point);
        //cout<<"-nuevo tamaño points[1]: "<<points[1].size()<<endl;
        addPt = false;
    }

    std::swap(points[1], points[0]);
    cv::swap(prevGray, gray);
    ///--------------------------------------
}

void VideoCore::addpointtotrack(int _index){
    addPt = true;
    rmPt = false;
    index = _index;
    cout<<"desde videocore: se agrego un nuevo punto para rastrear"<<endl;

}

void VideoCore::rmpointtotrack(int _index){
    addPt = false;
    rmPt = true;
    index = _index;
    cout<<"desde videocore: se elimina un punto de rastrear: "<<index<<endl;

}

void VideoCore::setTrackList(TrackList* trackers){
    Trackers = trackers;
}

double VideoCore::getCurrentFrame(){
    return capture->get(CV_CAP_PROP_POS_FRAMES);
}
double VideoCore::getNumberOfFrames(){
    return capture->get(CV_CAP_PROP_FRAME_COUNT);
}
double VideoCore::getFrameRate(){
    return frameRate;
}
void VideoCore::setCurrentFrame( int frameNumber )
{
    bool success = capture->set(CV_CAP_PROP_POS_FRAMES, frameNumber);
    if (!success) {
        std::cout << "Cannot set frame position from video file at " << frameNumber << std::endl;
        return;
    }
}

void VideoCore::setPercentRate( int PercentRate )
{
    frameRateRep = (frameRate*PercentRate)/100+1;
}

void VideoCore::setLoop(bool active)
{
    loop = active;
}

VideoCore::~VideoCore()
{
    mutex.lock();
    stop = true;
    capture->release();
    delete capture;
    condition.wakeOne();
    mutex.unlock();
    wait();
}
Mat* VideoCore::getCurrentImage(){
    return &frame;
}

void VideoCore::Pause()
{
    stop = true;
}

void VideoCore::msleep(int ms){
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
}

bool VideoCore::isStopped() const{
    return this->stop;
}
