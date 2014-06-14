#include "videowindow.h"
#include "segment.h"
#include <QVBoxLayout>
#include <QTime>
#include <QInputDialog>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

VideoWindow::VideoWindow(QWidget *parent, VideoCore *_player, TrackList* lista_track, SegmentList *lista_seg):
QWidget(parent)
{
    currentTool = TOOL_NONE;
    setVideoPlayer(_player);
    setTrackList(lista_track);
    setSegmentList(lista_seg);

    initScreen();
    initMediaPlayer();
    createActions();
    setOrigin();

    std::cout<< "Se creo la ventana de video satisfactoriamente"<< std::endl;
}


void VideoWindow::setVideoPlayer(VideoCore *_player){
    Player = _player;
}

void VideoWindow::setTrackList(TrackList* lista){
    lista_trackers = lista;

}

void VideoWindow::setSegmentList(SegmentList* lista){
    lista_segmentos = lista;

}

void VideoWindow::hideControls(){
    bn_stop->hide();
    bn_prev->hide();
    bn_play->hide();
    bn_next->hide();
    cb_loop->hide();
    sl_percentRate->hide();
    lb_videotime->hide();
}

void VideoWindow::showControls(){
    bn_stop->show();
    bn_prev->show();
    bn_play->show();
    bn_next->show();
    cb_loop->show();
    sl_percentRate->show();
    lb_videotime->show();
}

VideoWindow::~VideoWindow()
{
    delete screen;
    delete Player;
}

/*********************Screen*********************/

void VideoWindow::on_screen_mouseDown(Point2i point){
    Segment *tmp;
    Track* tmp1;
    tmp_point=point;

    Player->Pause();
    bn_play->setIcon(QIcon(QPixmap("../images/media-playback-start.png")));

    //se ponen los punteros al 'objeto' actual a NULL
    lista_segmentos->setcurrentSegment(NULL);
    lista_trackers->setcurrentTrack(NULL);
    //se busca si el punto pertenece a un area de interes
    tmp1 = lista_trackers->searchByPoint(point);
    if(tmp1){
        lista_trackers->setcurrentTrack(tmp1);
        std::cout<< "el punto no pertenece a un tracker (onDown)"<<endl;
    } else {
        //se busca si el punto pertenece a algun segmento
        tmp = lista_segmentos->searchByPoint(point);
        if(tmp){
            //el punto pertenece a un segmento
            lista_segmentos->setcurrentSegment(tmp);
             //std::cout << "(screen_mouseDow) El punto pertenece: " << tmp->label()<< std::endl;
        }
    }
    //std::cout<< "Presiono sobre el video (desde mainwin): x: " << pos.x() << "y: " << pos.y() << std::endl;
}

void VideoWindow::on_screen_mouseUp(Point2i act_point){

    long int d2;

    Segment *stmp = lista_segmentos->getcurrentSegment();
    Track* ttmp = lista_trackers->getcurrentTrack();
    Mat mtmp = Player->getCurrentImage()->clone();

    if (ttmp){
        if (TOOL_MANUALPATH){
            //si el punto pertenece a una area de interes
            cout<<"hasta aqui llego"<<endl;
            ttmp->setCenter(act_point);
            cv::rectangle(mtmp, *(ttmp->getStartPoint()),*(ttmp->getEndPoint()),*(ttmp->getColor(TRACKCOLOR_NORMAL)),2, 3);
            ttmp->setPosinPath(Player->getCurrentFrame(),act_point);
            on_nextButton_clicked();
        }

    }else if(stmp){
        //si el punto pertenece a un segmento, s
        cv::line(mtmp, *(stmp->getStartPoint()),*(stmp->getEndPoint()),*(stmp->getColor(TRACKCOLOR_SELECT)), 2, 3);
        screen->showImage(mtmp);


    }else {
        //el punto no pertenece a nada
        std::cout<< "efectivemente, el punto no pertenece a nada (onUp)"<<endl;
        switch (currentTool){
        case TOOL_TRACKER:
            {
            lista_trackers->addTrack(new Track(QString("tracker %1").arg(lista_trackers->size()).toLatin1().data(),
                                               act_point,Player->getCurrentFrame(), Scalar(0,200,0)));
            Player->update();
            break;
            }
        case TOOL_SEGMENT:
            {
            //se verifica si hubo desplazamiento
            d2 = (act_point.x-tmp_point.x)*(act_point.x-tmp_point.x) +
                 (act_point.y-tmp_point.y)*(act_point.y-tmp_point.y);
            //std::cout << "(screen_mouseUp) no pertenece a ningun segmento, se verifica desplazamiento"<<endl;
            if (d2>4){ //en caso de existir un desplazamiento mayor de 2 px, se crea un nuevo segmento
                lista_segmentos->addSegment(new Segment(QString("segmento %1").arg(lista_segmentos->size()).toLatin1().data(), tmp_point, act_point,Scalar(0,200,0)));
                //std::cout<< "(screen_mouseUp) desplazamiento "<<d2<<">1 "<<"se crea un segmento"<<endl;
               }
            //Player->Play();
            //bn_play->setIcon(QIcon(QPixmap("../images/media-playback-pause.png")));
            break;
            }
        }

    }

    //std::cout<< "Solto sobre el video (desde mainwin): x: " << pos.x() << "y: " << pos.y() << std::endl;
}

void VideoWindow::on_screen_mouseDrag(Point2i act_point){

    Mat tmp;
    Track* ttmp = lista_trackers->getcurrentTrack();
    tmp = Player->getCurrentImage()->clone();
    switch (currentTool){
        case TOOL_TRACKER:
        case TOOL_MANUALPATH:
        {
            if(ttmp){
                Point2i p1 = act_point;
                Point2i p2 = act_point;
                p1.x -= DEF_ROI1/2;
                p1.y -= DEF_ROI1/2;

                p2.x += DEF_ROI1/2;
                p2.y += DEF_ROI1/2;

                cv::rectangle(tmp, p1, p2, *(ttmp->getColor(TRACKCOLOR_SELECT)),2, 3);
            }
            break;
          }
        case TOOL_SEGMENT:
            cv::line(tmp, tmp_point,act_point, cv::Scalar(0,250,0), 2, 3);
            break;
    }
    updateScreen(&tmp);
    //std::cout<< "Player->getCurrentImage()"  << pos.x() << "y: " << pos.y() << std::endl;
}

void VideoWindow::updateScreen(Mat* frame)
{

    drawSegment(frame);
    drawTracker(frame);
    screen->showImage(*frame);
    sl_timeLine->setValue(Player->getCurrentFrame());
    lb_videotime->setText(getFormattedTime( (int)Player->getCurrentFrame()/(int)Player->getFrameRate()) );
}

void VideoWindow::drawTracker(Mat *frame){
    //optimizar esto con un buffer
    Track* tmp;
    Point2i* currentPos;
    Point2i textPos;
    int currentFrame = Player->getCurrentFrame();
    for (int i = 0; i < lista_trackers->size(); ++i) {
        tmp = lista_trackers->searchByIndex(i);
        if(!tmp){
            cout << "(VideoWindow::draw) no hay trackers para dibujar "<<endl;
        }else{
            currentPos = tmp->getPosinPath(currentFrame);
            if(currentPos){
                tmp->setCenter(*currentPos);
                cv::rectangle(*frame, *(tmp->getStartPoint()),*(tmp->getEndPoint()), *(tmp->getColor(TRACKCOLOR_NORMAL)), 2, 3);
                if (tmp->isLabelVisible()){
                    //la conversion a toLatin1().data() puede ser lenta
                    textPos = *(tmp->getPostext());

                    cv::putText(*frame,tmp->label(), textPos, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,0,0),3);
                    cv::putText(*frame,tmp->label(), textPos, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(250,250,250),1.5);
                }
                if (tmp->isPathVisible()){


                    int firstframe=tmp->frameStartTrack+1;
                    int lastframe = currentFrame - firstframe;
                    vector<Point2i> path_buffer;

                    for(int i=0; i<=lastframe; i++)
                        path_buffer.push_back(*(tmp->getPosinPath(firstframe+i)));


                   //cv::polylines(*frame,&nCurvePts,false,*(tmp->getColor(TRACKCOLOR_PATH)),1,8,0);
                   const cv::Point *pts = (const cv::Point*) Mat(path_buffer).data;

                   polylines(*frame, &pts,&lastframe, 1, false,*(tmp->getColor(TRACKCOLOR_PATH)),2, CV_AA, 0);

                }

            }
        }
    }
}

void VideoWindow::drawSegment(Mat *frame){
    int x1,y1;
    Segment* tmp;
    for (int i = 0; i < lista_segmentos->size(); ++i) {
        tmp = lista_segmentos->searchByIndex(i);
        if(!tmp){
            cout << "(VideoWindow::draw) no hay segmentos para dibujar "<<endl;
        }else{

            x1=tmp->getPostext()->x;
            y1=tmp->getPostext()->y;
            cv::Point2i pvalue(x1,y1+15);

            cv::line(*frame, *(tmp->getStartPoint()),*(tmp->getEndPoint()), *(tmp->getColor(TRACKCOLOR_NORMAL)), 2, 3);
            if (tmp->isLabelVisible()){
                //la conversion a toLatin1().data() puede ser lenta
                cv::putText(*frame,tmp->label(), *(tmp->getPostext()), CV_FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,0,0),3);
                cv::putText(*frame,tmp->label(), *(tmp->getPostext()), CV_FONT_HERSHEY_PLAIN, 1.0, CV_RGB(250,250,250),1.5);
            }
            if (tmp->isValueVisible()){
                cv::putText(*frame,tmp->value(), pvalue, CV_FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,0,0),3);
                cv::putText(*frame,tmp->value(), pvalue, CV_FONT_HERSHEY_PLAIN, 1.0, CV_RGB(250,250,250),1.5);
            }

            //cout << tmp->label().toLatin1().data()<< endl;
        }
    }
    //se grafican los ejes
    if (isXaxisVisible){
        cv::line(*frame, *(axisX->getStartPoint()),*(axisX->getEndPoint()), *(axisX->getColor(TRACKCOLOR_NORMAL)), 2, 3 );
    }
    if (isYaxisVisible){
        cv::line(*frame, *(axisY->getStartPoint()),*(axisY->getEndPoint()), *(axisY->getColor(TRACKCOLOR_NORMAL)), 2, 3);
    }
}

/*********************MediaPlayer*********************/
QString VideoWindow::getFormattedTime(int timeInSeconds){
    int seconds = (int) (timeInSeconds) % 60 ;
    int minutes = (int) ((timeInSeconds / 60) % 60);
    int hours   = (int) ((timeInSeconds / (60*60)) % 24);
    QTime t(hours, minutes, seconds);
    if (hours == 0 )
        return t.toString("mm:ss");
    else
        return t.toString("h:mm:ss");
}

void VideoWindow::openfile(string filename)
{

    if (!Player->loadVideo(filename))
    {
        QMessageBox msgBox;
        msgBox.setText("The selected video could not be opened!");
        msgBox.exec();
    }
    else{

        sl_timeLine->setEnabled(true);
        sl_percentRate->setEnabled(true);
        bn_next->setEnabled(true);
        bn_play->setEnabled(true);
        bn_prev->setEnabled(true);
        bn_stop->setEnabled(true);
        sl_timeLine->setMaximum(Player->getNumberOfFrames());
        lb_videotime->setText( getFormattedTime( (int)Player->getNumberOfFrames()/(int)Player->getFrameRate()) );
      }

    Track::setSamplingRate((float)(Player->getFrameRate()));
}

void VideoWindow::on_playButton_clicked()
{
    if (Player->isStopped())
    {
        Player->Play();
        bn_play->setIcon(QIcon(QPixmap("../images/media-playback-pause.png")));
    }else
    {
        Player->Pause();
        bn_play->setIcon(QIcon(QPixmap("../images/media-playback-start.png")));
    }
}


void VideoWindow::on_nextButton_clicked()
{
    if (Player->isStopped())
    {
        //nextFrame = Player->getCurrentFrame()+1;
        //sl_timeLine->setValue(nextFrame);

        Player->update();
    }
}

void VideoWindow::on_prevButton_clicked()
{
    double prevFrame;
    if (Player->isStopped())
    {
        prevFrame = Player->getCurrentFrame()-2;
        sl_timeLine->setValue(prevFrame);
        Player->update();
    }
}


void VideoWindow::on_stopButton_clicked()
{
   sl_timeLine->setValue(0);
   Player->update();
   bn_play->setIcon(QIcon(QPixmap("../images/media-playback-pause.png")));
}

void VideoWindow::on_loopCheck_clicked(){
    if(cb_loop->isChecked()){
        Player->setLoop(true);
    }else{
        Player->setLoop(false);
    }
}

void VideoWindow::on_horizontalSlider_sliderPressed()
{
    Player->Pause();
}

void VideoWindow::on_horizontalSlider_sliderReleased()
{
    Player->Play();
}

void VideoWindow::on_horizontalSlider_sliderMoved(int position)
{
    Player->setCurrentFrame(position);
    lb_videotime->setText( getFormattedTime( position/(int)Player->getFrameRate()) );
}

void VideoWindow::on_percentRate_sliderMoved(int position)
{
    QString label;
    Player->setPercentRate(position);
    sl_percentRate->setToolTip(label.append(QString("%1").arg(position)).append("%"));
}

/*********************Menu Contextual*********************/
void VideoWindow::contextMenuEvent(QContextMenuEvent *event)
 {
     QMenu menu(this);
     menu.addAction(showXaxisAct);
     menu.addAction(showYaxisAct);
     menu.addAction(setOriginAct);
     menu.addSeparator();
     if (lista_segmentos->getcurrentSegment()){
        menu.addAction(showLabelAct);
        menu.addAction(showValueAct);
        menu.addSeparator();
        menu.addAction(setLabelAct);
        menu.addAction(setMeasureAct);
        menu.addAction(setEndTrackAct);
        menu.addSeparator();
        menu.addAction(deleteCurrentObjectAct);

        showLabelAct->setChecked(lista_segmentos->getcurrentSegment()->isLabelVisible());
        showValueAct->setChecked(lista_segmentos->getcurrentSegment()->isValueVisible());
     }

     else if(lista_trackers->getcurrentTrack()){
         menu.addAction(showLabelAct);
         menu.addAction(setLabelAct);
         menu.addAction(setEndTrackAct);
         menu.addSeparator();
         menu.addAction(deleteCurrentObjectAct);

         showLabelAct->setChecked(lista_trackers->getcurrentTrack()->isLabelVisible());
     }
     menu.exec(event->globalPos());
 }

//acciones globales
void VideoWindow::showXaxis(){
     isXaxisVisible = showXaxisAct->isChecked();

}

void VideoWindow::showYaxis(){
    isYaxisVisible = showYaxisAct->isChecked();

}

void VideoWindow::setOrigin(){

    //enviar mensaje barra de estado, indicando que se esta cambiando el origen
    //
    int xend, yend;

    xend = screen->width();
    yend = screen->height();
    origin = tmp_point;
    axisX = new Segment("x",Point2i(0, origin.y),Point2i(xend,origin.y),Scalar(230,100,100));
    axisY = new Segment("x",Point2i(origin.x, 0),Point2i(origin.x, yend),Scalar(230,100,100) );
    isYaxisVisible = true;
    isXaxisVisible = true;
    Track::setOrigin(origin);
    std::cout<< "(desde setOrigin): x: " << origin.x<< "y: " <<  origin.y << std::endl;
    //Actualizar todos los tracks
    lista_trackers->updateTrackers();
}

//acciones asociadas a segmentos

void VideoWindow::setMeasure(){
    bool ok;
    Segment* tmp;
    tmp = lista_segmentos->getcurrentSegment();

    double d = QInputDialog::getDouble(this, QString::fromStdString(tmp->label()).append(" - establecer medida"),
                                            tr("Medida:"), 1.0, -10000, 10000, 2, &ok);
         if (ok)
             std::cout << tmp->value_f() << " px, equivalen a " << d << " m" << endl;
             Track::setMeasure(tmp->value_f(), d, TRACKUNIT_METER);
             //Actualizar todos los valores de segmentos y tracks
             lista_segmentos->updateSegments();
             lista_trackers->updateTrackers();

}

void VideoWindow::showValue(){
    Segment *tmp;
    tmp = lista_segmentos->getcurrentSegment();
    tmp->setValueVisible(!(tmp->isValueVisible()));
}

//acciones asociadas a segmentos y trackers

void VideoWindow::setEndTrack(){
    Track* tmp;
    tmp = lista_trackers->getcurrentTrack();
    tmp->setEndTrack(Player->getCurrentFrame());
}

void VideoWindow::setLabel(){
   bool ok;
   QString old_label;
   Segment *tmp;
   Track* ttmp;
   tmp  = lista_segmentos->getcurrentSegment();
   ttmp = lista_trackers->getcurrentTrack();

   if(ttmp){
        old_label = QString::fromStdString(ttmp->label());
        QString text = QInputDialog::getText(this, tr("Nueva etiqueta"),tr("Etiqueta:"), QLineEdit::Normal,old_label ,&ok);

        if (ok && !text.isEmpty()) {
            lista_trackers->getcurrentTrack()->setLabel(text.toLatin1().data());
        }
    }else if(tmp){
       old_label = QString::fromStdString(tmp->label());
       QString text = QInputDialog::getText(this, tr("Nueva etiqueta"),tr("Etiqueta:"), QLineEdit::Normal,old_label ,&ok);

       if (ok && !text.isEmpty()) {
           lista_segmentos->getcurrentSegment()->setLabel(text.toLatin1().data());
       }
   }
}

//modificar para incluir trackers
void VideoWindow::showLabel(){
    Segment *tmp;
    Track* ttmp;
    tmp  = lista_segmentos->getcurrentSegment();
    ttmp = lista_trackers->getcurrentTrack();

    if(tmp)
        tmp->setLabelVisible(!(tmp->isLabelVisible()));

    if(ttmp)
        ttmp->setLabelVisible(!(ttmp->isLabelVisible()));
}

//modificar para incluir trackers
void VideoWindow::deleteCurrentObject(){
    Segment *tmp;
    Track *ttmp;
    tmp = lista_segmentos->getcurrentSegment();
    ttmp = lista_trackers->getcurrentTrack();
    if(tmp){
        lista_segmentos->delSegment(tmp->label());
        //screen->showImage(*(Player->getCurrentImage()));
        Player->Play();

    }else if(ttmp){
        lista_trackers->rmTrack(ttmp->label());
        //screen->showImage(*(Player->getCurrentImage()));
        Player->Play();
    }else{
        QMessageBox msgBox;
        msgBox.setText("No ha seleccionado ningun segmento!");
        msgBox.exec();
    }

}
void VideoWindow::setTool(int  tooltype){
    currentTool = tooltype;
}

void VideoWindow::initScreen(){
    screen = new CVImageWidget();
    scrollArea = new QScrollArea();
    //lista_segmentos = new SegmentList();
    //lista_trackers = new TrackList();
    //tmp_point = new QPoint();

    screen->scaleImage(1.0);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setMinimumSize(320,240);
    scrollArea->setWidget(screen);

    tmp_point = Point2i(0,0);

    QObject::connect(screen, SIGNAL(onMouseDown(Point2i )), this, SLOT(on_screen_mouseDown(Point2i )));
    QObject::connect(screen, SIGNAL(onMouseUp(Point2i )), this, SLOT(on_screen_mouseUp(Point2i )));
    QObject::connect(screen, SIGNAL(onMouseDrag(Point2i)), this, SLOT(on_screen_mouseDrag(Point2i )));
}

void VideoWindow::initMediaPlayer(){

    //Player = new VideoPlayer();

    QGridLayout* layout = new QGridLayout(); //this


    bn_stop= new QPushButton(this);
    bn_stop->setIcon(QIcon(QPixmap("../images/media-playback-stop.png")));
    bn_stop->setIconSize(QSize(32, 32));
    bn_stop->setStyleSheet("QPushButton{border: none;outline: none;}");
    bn_stop->setEnabled(false);

    bn_prev= new QPushButton(this);
    bn_prev->setIcon(QIcon(QPixmap("../images/media-seek-backward.png")));
    bn_prev->setIconSize(QSize(32, 32));
    bn_prev->setStyleSheet("QPushButton{border: none;outline: none;}");
    bn_prev->setEnabled(false);

    bn_play = new QPushButton(this);
    bn_play->setIcon(QIcon(QPixmap("../images/media-playback-start.png")));
    bn_play->setIconSize(QSize(32, 32));
    bn_play->setStyleSheet("QPushButton{border: none;outline: none;}");
    bn_play->setEnabled(false);

    bn_next = new QPushButton(this);
    bn_next->setIcon(QIcon(QPixmap("../images/media-seek-forward.png")));
    bn_next->setIconSize(QSize(32, 32));
    bn_next->setStyleSheet("QPushButton{border: none;outline: none;}");
    bn_next->setEnabled(false);
    cb_loop = new QCheckBox("Repetir");

    sl_timeLine = new QSlider(Qt::Horizontal,0);
    sl_percentRate= new QSlider(Qt::Horizontal,0);
    lb_videotime = new QLabel("00:00");
    //lb_videovel = new QLabel("Velocidad:");

    layout->addWidget(scrollArea,0,0,1,8,0);
    layout->addWidget(sl_timeLine,1,0,1,8,0);


    layout->addWidget(bn_stop,2,0,0);
    layout->addWidget(bn_prev,2,1,0);
    layout->addWidget(bn_play,2,2,0);
    layout->addWidget(bn_next,2,3,0);
    layout->addWidget(cb_loop,2,4,0);
    layout->addWidget(lb_videotime,2,5,0);
    //layout->addWidget(lb_videovel,2,5,0);
    layout->addWidget(sl_percentRate,2,7,Qt::AlignLeft);

    sl_percentRate->setMaximum(100);
    sl_percentRate->setMinimum(5);
    sl_percentRate->setValue(100);
    sl_percentRate->setEnabled(false);
    //window->setLayout(layout);
    this->setLayout(layout);
    // Set QWidget as the central layout of the main window
    //setCentralWidget(window);
    QShortcut *shortcut_play = new QShortcut(QKeySequence("Ctrl+P"), this);
    QShortcut *shortcut_stop = new QShortcut(QKeySequence("Ctrl+Left"), this);
    QObject::connect(shortcut_play, SIGNAL(activated()), this, SLOT(on_playButton_clicked()));
    QObject::connect(shortcut_stop, SIGNAL(activated()), this, SLOT(on_stopButton_clicked()));
    QObject::connect(Player, SIGNAL(onEnterFrame(Mat*)),this, SLOT(updateScreen(Mat*)));

    QObject::connect(bn_play, SIGNAL(released()), this, SLOT(on_playButton_clicked()));
    QObject::connect(bn_next, SIGNAL(released()), this, SLOT(on_nextButton_clicked()));
    QObject::connect(bn_prev, SIGNAL(released()), this, SLOT(on_prevButton_clicked()));
    QObject::connect(bn_stop, SIGNAL(released()), this, SLOT(on_stopButton_clicked()));

    QObject::connect(cb_loop, SIGNAL(clicked()), this, SLOT(on_loopCheck_clicked()));

    QObject::connect(sl_timeLine, SIGNAL(sliderPressed()), this, SLOT(on_horizontalSlider_sliderPressed()));
    QObject::connect(sl_timeLine, SIGNAL(sliderReleased()), this, SLOT(on_horizontalSlider_sliderReleased()));
    QObject::connect(sl_timeLine, SIGNAL(sliderMoved(int)), this, SLOT(on_horizontalSlider_sliderMoved(int)));
    QObject::connect(sl_timeLine, SIGNAL(valueChanged(int)), this, SLOT(on_horizontalSlider_sliderMoved(int)));

    QObject::connect(sl_percentRate, SIGNAL(sliderPressed()), this, SLOT(on_horizontalSlider_sliderPressed()));
    QObject::connect(sl_percentRate, SIGNAL(sliderReleased()), this, SLOT(on_horizontalSlider_sliderReleased()));
    QObject::connect(sl_percentRate, SIGNAL(sliderMoved(int)), this, SLOT(on_percentRate_sliderMoved(int)));
    QObject::connect(sl_percentRate, SIGNAL(valueChanged(int)), this, SLOT(on_percentRate_sliderMoved(int)));
}

void VideoWindow::createActions()
 {
     showXaxisAct = new QAction(tr("Mostrar eje &X"), this);
     showXaxisAct->setStatusTip(tr("Muestra el eje X"));
     showXaxisAct->setCheckable(true);
     showXaxisAct->setChecked(true);

     connect(showXaxisAct, SIGNAL(triggered()), this, SLOT(showXaxis()));

     showYaxisAct = new QAction(tr("Mostrar eje &Y"), this);
     showYaxisAct->setStatusTip(tr("Muestra el eje Y"));
     showYaxisAct->setCheckable(true);
     showYaxisAct->setChecked(true);
     connect(showYaxisAct, SIGNAL(triggered()), this, SLOT(showYaxis()));

     setOriginAct = new QAction(tr("Establecer &Origen Aquí"), this);;
     setOriginAct->setStatusTip(tr("Selecciona un punto en la pantallo como origen de coordenadas"));
     connect(setOriginAct, SIGNAL(triggered()), this, SLOT(setOrigin()));

     setEndTrackAct = new QAction(tr("Punto Final"), this);
     setEndTrackAct->setStatusTip(tr("Marcar Fin de la Trayectoria"));
     connect(setEndTrackAct, SIGNAL(triggered()), this, SLOT(setEndTrack()));

      //Acciones asociadas a segmentos

     setLabelAct = new QAction(tr("Cambiar &Etiqueta"), this);
     setLabelAct->setStatusTip(tr("Establece la media real del segmento seleccionado"));
     connect(setLabelAct, SIGNAL(triggered()), this, SLOT(setLabel()));

     setMeasureAct = new QAction(tr("Establecer &Medidas"), this);
     setMeasureAct->setStatusTip(tr("Establece la media real del segmento seleccionado"));
     connect(setMeasureAct, SIGNAL(triggered()), this, SLOT(setMeasure()));

     showLabelAct = new QAction(tr("Mostrar Etiqueta"), this);
     showLabelAct->setStatusTip(tr("Muestra la Etiqueta del segmento seleccionado"));
     showLabelAct->setCheckable(true);
     connect(showLabelAct, SIGNAL(triggered()), this, SLOT(showLabel()));

     showValueAct = new QAction(tr("Mostrar Valor"), this);
     showValueAct->setStatusTip(tr("Muestra el valor del segmento seleccionado"));
     showValueAct->setCheckable(true);
     connect(showValueAct, SIGNAL(triggered()), this, SLOT(showValue()));

     deleteCurrentObjectAct = new QAction(tr("&Eliminar"), this);
     deleteCurrentObjectAct->setShortcuts(QKeySequence::Delete);
     deleteCurrentObjectAct->setStatusTip(tr("Elimina el segmento seleccionado"));
     connect(deleteCurrentObjectAct, SIGNAL(triggered()), this, SLOT(deleteCurrentObject()));

 }
