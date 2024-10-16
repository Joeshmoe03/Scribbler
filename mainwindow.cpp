#include "mainwindow.h"
#include "scribbler.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    // Our MenuBar consists of several possible open/edit/undo actions
    QAction *openFileAct = new QAction("Open image file");
    QAction *saveFileAct = new QAction("Save image file");
    QAction *resetFileAct = new QAction("Reset file");

    QAction *startCapture = new QAction("Start capture");
    QAction *endCapture = new QAction("End capture");

    QAction *lineViewAct = new QAction("Line View");
    QAction *dotsViewAct = new QAction("Dots only view");

    QMenu *fileBar = new QMenu("&File");
    QMenu *captureBar = new QMenu("&Capture");
    QMenu *viewBar = new QMenu("&View");

    Scribbler *scribbler = new Scribbler();
    tabWidget = new QTabWidget();
    tabWidget->setHidden(true);

    //setCentralWidget(scribbler);
    QWidget *center = new QWidget;
    setCentralWidget(center);

    QVBoxLayout *mainLayout = new QVBoxLayout(center);
    QHBoxLayout *scribblerLayout = new QHBoxLayout();
    QHBoxLayout *tabLayout = new QHBoxLayout();

    scribblerLayout->addWidget(scribbler);
    tabLayout->addWidget(tabWidget);

    mainLayout->addLayout(scribblerLayout);
    mainLayout->addLayout(tabLayout);

    fileBar->addAction(openFileAct);
    fileBar->addAction(saveFileAct);
    fileBar->addAction(resetFileAct);

    captureBar->addAction(startCapture);
    captureBar->addAction(endCapture);

    viewBar->addAction(lineViewAct);
    viewBar->addAction(dotsViewAct);

    menuBar()->addMenu(fileBar);
    menuBar()->addMenu(captureBar);
    menuBar()->addMenu(viewBar);

    connect(openFileAct, &QAction::triggered, scribbler, &Scribbler::openFile);
    connect(saveFileAct, &QAction::triggered, scribbler, &Scribbler::saveFile);
    connect(resetFileAct, &QAction::triggered, scribbler, &Scribbler::resetFile);

    connect(startCapture, &QAction::triggered, scribbler, &Scribbler::startCapture);
    connect(endCapture, &QAction::triggered, scribbler, &Scribbler::endCapture);

    connect(lineViewAct, &QAction::triggered, scribbler, &Scribbler::showLines);
    connect(dotsViewAct, &QAction::triggered, scribbler, &Scribbler::showDots);
}

MainWindow::~MainWindow() {}

void MainWindow::receiveEvents(QList<MouseEvent> &data) {
    storedEvents = data; //FIX?


    QTableWidget *eventsTable = new QTableWidget();
    eventsTable->setRowCount(storedEvents.length());

    for (int i = 0; i < storedEvents.length(); ++i) {
        QTableWidgetItem *eventItem = new QTableWidgetItem();
        eventItem->setData(Qt::DisplayRole, i);
        eventsTable->setItem(i, 0, eventItem);
    }
    tabWidget->setHidden(false);
}






// QGraphicsView *view = new QGraphicsView(&scene);
// view->setSceneRect(QRectF(0.0, 0.0, 800.0, 600.0));
// view->setMinimumSize(QSize(800, 600));
// view->setRenderHint(QPainter::Antialiasing);

// QGraphicsLineItem *line0 = scene.addLine(QLineF(0.0, 0.0, 50.0, 100.0), QPen(Qt::blue, 20.0, Qt::SolidLine, Qt::FlatCap));
// QGraphicsLineItem *line1 = scene.addLine(QLineF(0.0, 0.0, 50.0, 100.0), QPen(Qt::black, 1.0));
//                      // = new QGraphicsLineItem(QLineF(QPointF(0.0, 0.0), QPoint(100.0, 100.0)));
// //scene.addItem(line);

// QGraphicsRectItem *rect = scene.addRect(QRectF(100.0, 100.0, 300.0, 100.0),
//                                         QPen(Qt::black, 5.0, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin),
//                                         Qt::red);
// QGraphicsEllipseItem *dot = scene.addEllipse(100.0 - 3.0, 100.0 - 3.0, 6.0, 6.0, Qt::NoPen, Qt::black);
// rect->setRotation(10.0);
// rect->setScale(0.5);
// rect->setTransformOriginPoint(100.0, 100.0);

// // We can group items together too
// QGraphicsItemGroup *group = scene.createItemGroup(QList<QGraphicsItem *>() << line0 << line1 << rect  << dot);

// scene.addRect(view->sceneRect());

// //rect->setFlag(QGraphicsItem::ItemIsMovable);
// group->setFlag(QGraphicsItem::ItemIsMovable);
// group->setTransformOriginPoint(0.0, 0.0);
// group->setTransform(QTransform().translate(200, 200).scale(0.5, 0.5).rotate(90.0));
