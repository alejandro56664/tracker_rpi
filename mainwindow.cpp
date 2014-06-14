#include <QtWidgets>
#ifndef QT_NO_PRINTDIALOG
#include <QtPrintSupport>
#endif

#include <iostream>
#include "mainwindow.h"
#include "plotwindow.h"
#include "videowindow.h"

MainWindow::MainWindow()
{

    trackers = new TrackList();
    segments = new SegmentList();
    player = new VideoCore(trackers);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();
    setUnifiedTitleAndToolBarOnMac(true);
    std::cout<< "Se creo la ventana principal satisfactoriamente"<< std::endl;
}

void MainWindow::save()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                          tr("Save Xml"), ".",
                                          tr("Xml files (*.xml)"));

    QFile file(filename);
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("elements");
    xmlWriter.writeAttribute("px2unit", QString::number(Track::px2unit) );
    xmlWriter.writeAttribute("samplingRate", QString::number(Track::samplingRate));
    xmlWriter.writeAttribute("unit",(Track::typeunit==TRACKUNIT_METER)?"m":"cm");

    xmlWriter.writeStartElement("origin");
        xmlWriter.writeAttribute("x", QString::number(Track::origin.x));
        xmlWriter.writeAttribute("y", QString::number(Track::origin.y));

    Segment* tmp; Track* ttmp;
    for (int i = 0; i < segments->size(); ++i) {
        tmp = segments->searchByIndex(i);
        if(!tmp){
            cout << "no hay Segmentos para guardar "<<endl;
        }else{
            xmlWriter.writeStartElement("segment");
                xmlWriter.writeAttribute("label", QString::fromStdString(tmp->label()));
                //xmlWriter.writeAttribute("value", QString::number(tmp->value_f()));
                //xmlWriter.writeTextElement("color",tmp->getColor());
                xmlWriter.writeEmptyElement("p1");
                xmlWriter.writeAttribute("x",  QString::number(tmp->getStartPoint()->x));
                xmlWriter.writeAttribute("y",  QString::number(tmp->getStartPoint()->y));

                xmlWriter.writeEmptyElement("p2");
                xmlWriter.writeAttribute("x",  QString::number(tmp->getEndPoint()->x));
                xmlWriter.writeAttribute("y",  QString::number(tmp->getEndPoint()->y));
            xmlWriter.writeEndElement();
        }
    }

    for (int i = 0; i < trackers->size(); ++i) {
        ttmp = trackers->searchByIndex(i);
        if(!ttmp){
            cout << "no hay Trackers para guardar "<<endl;
        }else{
            xmlWriter.writeStartElement("tracker");
                xmlWriter.writeAttribute("label", QString::fromStdString(ttmp->label()));
                xmlWriter.writeAttribute("framestart", QString::number(ttmp->frameStartTrack));
                //xmlWriter.writeTextElement("color",ttmp->getColor());

                xmlWriter.writeEmptyElement("center");
                xmlWriter.writeAttribute("x",  QString::number(ttmp->getCenter()->x));
                xmlWriter.writeAttribute("y",  QString::number(ttmp->getCenter()->y));

                xmlWriter.writeStartElement("path");
                xmlWriter.writeAttribute("size",  QString::number(ttmp->path_px.size()));
                    for(int j=0; j<ttmp->path_px.size();++j){
                        xmlWriter.writeEmptyElement("p");
                        xmlWriter.writeAttribute("x",  QString::number(ttmp->path_px[j].x));
                        xmlWriter.writeAttribute("y",  QString::number(ttmp->path_px[j].y));
                        //xmlWriter.writeEndElement();
                    }
                xmlWriter.writeEndElement();
             xmlWriter.writeEndElement();
        }
    }
  xmlWriter.writeEndDocument();

  file.close();
  statusBar()->showMessage(tr("Xml Saved"));

}


void MainWindow::open()
{

   QString px2unit, sampleRate, unit, origenx, origeny;
   QString filename = QFileDialog::getOpenFileName(this,tr("Abrir Video"), "../tracker_capture/",tr("Video Files (*.avi *.mpg *.mp4)"));
    if (!filename.isEmpty()){
        videowin->openfile(filename.toLatin1().data());
    }

    QString textfile = filename.section(".",0,0).append(".xml");
    QFile file(textfile);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        std::cerr << "Error: Cannot read file " << qPrintable(textfile)
         << ": " << qPrintable(file.errorString())
         << std::endl;
    }else{
    Rxml.setDevice(&file);
    Rxml.readNext();
    while(!Rxml.atEnd()){
            if(Rxml.isStartElement())
            {
                if(Rxml.name() == "elements")
                {
                   //leer propiedades de los elementos
                    px2unit=Rxml.attributes().value("px2unit").toString();
                    sampleRate=Rxml.attributes().value("samplingRate").toString();
                    unit=Rxml.attributes().value("unit").toString();

                    Track::px2unit = px2unit.toFloat();
                    Track::setSamplingRate(sampleRate.toFloat());
                    Track::typeunit = (unit=="m")?TRACKUNIT_METER:TRACKUNIT_CENTIMETER;
                    //std::cout<<"px2unit: "<< px2unit.toLatin1().data() << std::endl;
                    //std::cout<<"sampleRate: "<<sampleRate.toLatin1().data()<<std::endl;
                    //std::cout<<"unit: "<<unit.toLatin1().data()<<std::endl;
                   Rxml.readNext();
                }else if(Rxml.name() == "origin"){
                    //leer origen
                    origenx=Rxml.attributes().value("x").toString();
                    origeny=Rxml.attributes().value("y").toString();

                    Track::setOrigin(Point2i(origenx.toInt(),origeny.toInt()));
                    //std::cout<<"origenx: "<<origenx.toLatin1().data()<<std::endl;
                    //std::cout<<"origeny: "<<origeny.toLatin1().data()<<std::endl;
                    Rxml.readNext();
                }else if(Rxml.name() == "segment"){
                    read_segment();

                }else if(Rxml.name() == "tracker"){

                    read_tracker();
                } else {
                    Rxml.raiseError(QObject::tr("Not a bookindex file"));
                }

            } else {
                Rxml.readNext();
            }
    }
        file.close();

    }
    if (Rxml.hasError()) {
         std::cerr << "Error: Failed to parse file "
                   << qPrintable(filename) << ": "
                   << qPrintable(Rxml.errorString()) << std::endl;

    }else if (file.error() != QFile::NoError){
          std::cerr << "Error: Cannot read file " << qPrintable(filename)
                    << ": " << qPrintable(file.errorString())
                    << std::endl;
   }
  statusBar()->showMessage(tr("Xml Opened"));
}



void MainWindow::read_segment(){

    QString _label, _p1x, _p1y, _p2x, _p2y;
    String label;
    int p1x, p1y, p2x, p2y;


    while(!Rxml.atEnd()){
         if(Rxml.isEndElement()){
            Rxml.readNext();
            break;
         }else if(Rxml.isCharacters()){
            Rxml.readNext();
         }else if(Rxml.isStartElement()) {
            if (Rxml.name() == "segment"){
             _label=Rxml.attributes().value("label").toString();
             //_value=Rxml.attributes().value("value").toString();
            }else if(Rxml.name() == "p1"){
                _p1x=Rxml.attributes().value("x").toString();
                _p1y=Rxml.attributes().value("y").toString();
                Rxml.readNext();
            }else if(Rxml.name() == "p2"){
                _p2x=Rxml.attributes().value("x").toString();
                _p2y=Rxml.attributes().value("y").toString();
            }
            Rxml.readNext();
         }else{
            Rxml.readNext();
         }
    }

    label = _label.toLatin1().data();
    //value = _value.toFloat();
    p1x = _p1x.toInt();
    p1y = _p1y.toInt();
    p2x = _p2x.toInt();
    p2y = _p2y.toInt();


    segments->addSegment(new Segment(label, Point2i(p1x,p1y), Point2i(p2x,p2y), Scalar(0,200,0)));
    std::cout<<"label: "<< _label.toLatin1().data() << std::endl;
    //std::cout<<"value: "<<_value.toLatin1().data()<<std::endl;
    std::cout<<"p1x: "<<_p1x.toLatin1().data()<<std::endl;
    std::cout<<"p1y: "<<_p1y.toLatin1().data()<<std::endl;
    std::cout<<"p2x: "<<_p2x.toLatin1().data()<<std::endl;
    std::cout<<"p2y: "<<_p2y.toLatin1().data()<<std::endl;

}

void MainWindow::read_tracker(){
    QString _label, _centerx, _centery, _framestart;
    String label;
    int centerx, centery, framestart, size;
    vector<Point2i> path_px;
    Track* tmp;

    while(!Rxml.atEnd()){
         if(Rxml.isEndElement()){
            Rxml.readNext();
            break;
         }else if(Rxml.isCharacters()){
            Rxml.readNext();
         }else if(Rxml.isStartElement()) {
            if (Rxml.name() == "tracker"){
             _label=Rxml.attributes().value("label").toString();
             _framestart=Rxml.attributes().value("framestart").toString();
            }else if(Rxml.name() == "center"){
                _centerx=Rxml.attributes().value("x").toString();
                _centery=Rxml.attributes().value("y").toString();
                Rxml.readNext();
            }else if(Rxml.name() == "path"){
                size=(Rxml.attributes().value("size").toString().toInt());
                std::cout<<"size: "<<Rxml.attributes().value("size").toString().toLatin1().data()<<std::endl;
               read_tracker_path(&path_px,size);
            }
            Rxml.readNext();
         }else{
         Rxml.readNext();
         }
   }
    /*
    std::cout<<"label: "<< _label.toLatin1().data() << std::endl;
    std::cout<<"framestart: "<<_framestart.toLatin1().data()<<std::endl;
    std::cout<<"centerx: "<<_centerx.toLatin1().data()<<std::endl;
    std::cout<<"centery: "<<_centery.toLatin1().data()<<std::endl;
    */
     label = _label.toLatin1().data();
     framestart = _framestart.toInt();
     centerx = _centerx.toInt();
     centery = _centery.toInt();
     tmp = new Track(label, Point2i(centerx, centery), framestart, Scalar(0,200,0));
     tmp->path_px = path_px;
     tmp->estimateRelativePosition();
    tmp->estimateMotion();
    trackers->addTrack(tmp);
    //trackers->searchByIndex(trackers->size()-1)->path_px= path_px;
    //trackers->searchByIndex(trackers->size()-1)->estimateRelativePosition();
}

void MainWindow::read_tracker_path(vector<Point2i> *path, int size){
    QString x, y;
    int i=0;
    while(i<size){ //while (!Rxml.atEnd()) {
         Rxml.readNext();
         if (Rxml.isStartElement()) {
            if (Rxml.name() == "p")
                x=(Rxml.attributes().value("x").toString());
                y=(Rxml.attributes().value("y").toString());
                std::cout<<"p->x: "<<x.toLatin1().data()<<" y: "<<y.toLatin1().data()<<std::endl;
                path->push_back(Point2i(x.toInt(), y.toInt()));
                i++;
                Rxml.readNext();
          }
    }
}

void MainWindow::selectNewSegment(bool state){
    if(state){

        cursorAct->setChecked(false);
        addTrackerAct->setChecked(false);
        manualTrackAct->setChecked(false);

        videowin->setTool(TOOL_SEGMENT);
        statusBar()->showMessage(tr("Seleccione un punto en el video y arrastre para crear una linea"));
    } else {
        videowin->setTool(TOOL_NONE);
    }

}

void MainWindow::selectNewTracker(bool state){
    if(state){

        cursorAct->setChecked(false);
        addSegmentAct->setChecked(false);
        manualTrackAct->setChecked(false);

        videowin->setTool(TOOL_TRACKER);

        statusBar()->showMessage(tr("Seleccione una region en la pantalla para rastrearla"));
    } else {
        videowin->setTool(TOOL_NONE);
    }

}

void MainWindow::selectManualTrackAct(bool state){
    if(state){

        cursorAct->setChecked(false);
        addTrackerAct->setChecked(false);
        addSegmentAct->setChecked(false);
        videowin->setTool(TOOL_MANUALPATH);

        statusBar()->showMessage(tr("Seleccione un rastreador y muevalo en la nueva posición"));
    } else {
        videowin->setTool(TOOL_NONE);
    }
}

void MainWindow::selectAutoDetectAct(){
    player->searchCircle();
}

void MainWindow::selectCursor(bool state){
if(state){
    addTrackerAct->setChecked(false);
    addSegmentAct->setChecked(false);
    manualTrackAct->setChecked(false);
     videowin->setTool(TOOL_NONE);
     statusBar()->showMessage(tr("Herramienta Puntero"));
} else {
    videowin->setTool(TOOL_NONE);
}
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("Acerca de Track para raspberry Pi"),
            tr("The <b>Track para raspberry Pi</b> example demonstrates how to "));
}

void MainWindow::createActions()
{


    openAct = new QAction(QIcon("../images/document-open.png"), tr("&Abrir..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Abre un archivo de video"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon("../images/document-save.png"), tr("&Guardar Análisis"),this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Guarda el Análisis del video"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    cursorAct= new QAction(QIcon("../images/cursor.png"), tr("&Cursor..."), this);
    cursorAct->setStatusTip(tr("Herramienta Puntero"));
    cursorAct->setCheckable(true);
    cursorAct->setChecked(true);
    connect(cursorAct, SIGNAL(toggled(bool)), this, SLOT(selectCursor(bool)));

    viewLowRes = new QAction(tr("&Modo baja resolución"), this);
    viewLowRes->setStatusTip(tr("Permite visualizar de manera mas comoda en monitores de baja resolución"));
    viewLowRes->setCheckable(true);
    viewLowRes->setShortcut(QKeySequence::FullScreen);
    connect(viewLowRes, SIGNAL(triggered()), this, SLOT(changeLowResWin()));

    addTrackerAct = new QAction(QIcon("../images/tracker.png"), tr("&Nuevo Rastreador..."), this);
    addTrackerAct->setStatusTip(tr("Agrega un nuevo rastreador al video"));
    addTrackerAct->setCheckable(true);
    connect(addTrackerAct, SIGNAL(toggled(bool)), this, SLOT(selectNewTracker(bool)));

    addSegmentAct = new QAction(QIcon("../images/line.png"), tr("&Nuevo Segmento..."), this);
    addSegmentAct->setStatusTip(tr("Agrega un nuevo segmento al video"));
    addSegmentAct->setCheckable(true);
    connect(addSegmentAct, SIGNAL(toggled(bool)), this, SLOT(selectNewSegment(bool)));

    manualTrackAct = new QAction(QIcon("../images/manual.png"), tr("&Seguimiento Manual..."), this);
    manualTrackAct->setStatusTip(tr("Agrega un nuevo segmento al video"));
    manualTrackAct->setCheckable(true);
    connect(manualTrackAct, SIGNAL(toggled(bool)), this, SLOT(selectManualTrackAct(bool)));

    autoDetectAct = new QAction(QIcon("../images/auto.png"), tr("&Detecta la barra automaticamente..."), this);
    autoDetectAct->setStatusTip(tr("Detecta la barra automaticamente"));
    connect(autoDetectAct, SIGNAL(triggered()), this, SLOT(selectAutoDetectAct()));



    quitAct = new QAction(QIcon("../images/system-log-out.png"), tr("&Salir"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Cierra la aplicación"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&Tutorial"), this);
    aboutAct->setStatusTip(tr("Muestra la documentación disponible"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}
/*
void MainWindow::deshabilitar_herramientas(){
    cursorAct->setChecked(false);
    addTrackerAct->setCheckable(false);
    addSegmentAct->setCheckable(false);
    manualTrackAct->setCheckable(false);

}*/

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&Archivo"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    viewMenu = menuBar()->addMenu(tr("&Ver"));
    viewMenu->addAction(viewLowRes);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Ayuda"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar=new QToolBar("Archivo");

     addToolBar(Qt::LeftToolBarArea,fileToolBar);
    //fileToolBar = addToolBar(tr("Archivo"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(cursorAct);
    fileToolBar->addAction(addTrackerAct);
    fileToolBar->addAction(addSegmentAct);
     fileToolBar->addAction(manualTrackAct);
     fileToolBar->addAction(autoDetectAct);
    fileToolBar->addAction(quitAct);

}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Listo"));
}
void MainWindow::setLowRes(){
    dock1->setAllowedAreas(Qt::LeftDockWidgetArea);
    dock2->setAllowedAreas(Qt::LeftDockWidgetArea);
    //videowin->hideControls();
    //menuBar()->hide();
    statusBar()->hide();
    //fileToolBar->hide();
    tabifyDockWidget(dock1, dock2);
    this->showMaximized();
}

void MainWindow::changeLowResWin()
{

    if(viewLowRes->isChecked()){
        setLowRes();

    }else{

        videowin->showControls();
        //menuBar()->show();
        statusBar()->show();
        //fileToolBar->show();
        removeDockWidget(dock1);
        removeDockWidget(dock2);
        viewMenu->removeAction(dock1->toggleViewAction());
        viewMenu->removeAction(dock2->toggleViewAction());
        createDockWindows();
        this->showNormal();
    }


    cout<<"click en change"<<endl;
}



void MainWindow::createDockWindows()
{

    dock1 = new QDockWidget(tr("Graficas"), this);
    plotwin = new PlotWindow(dock1, player, trackers);
    dock1->setWidget(plotwin);
    addDockWidget(Qt::RightDockWidgetArea, dock1);
    viewMenu->addAction(dock1->toggleViewAction());

    dock2 = new QDockWidget(tr("Video"), this);
    videowin = new VideoWindow(dock2, player, trackers, segments);
    dock2->setWidget(videowin);
    addDockWidget(Qt::LeftDockWidgetArea, dock2);
    viewMenu->addAction(dock2->toggleViewAction());

    //dock1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock2->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

}

MainWindow::~MainWindow(){

    delete fileMenu;
    delete viewMenu;
    delete helpMenu;

    delete viewGraphAct;
    delete viewVideoAct;
    delete aboutAct;
    delete aboutQtAct;
    delete quitAct;

    delete saveAct;
    delete openAct;
    delete cursorAct;
    delete addTrackerAct;
    delete addSegmentAct;
    delete manualTrackAct;

    delete saveAct;
    delete openAct;
    delete cursorAct;
    delete addTrackerAct;
    delete addSegmentAct;
    delete manualTrackAct;

    delete fileToolBar;

    delete  plotwin;
    delete  videowin;

}
