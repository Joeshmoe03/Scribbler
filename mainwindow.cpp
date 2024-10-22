#include "mainwindow.h"
#include "scribbler.h"

#include <QtWidgets>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tabCount(0) {

    // Our MenuBar consists of several possible actions
    // Our file actions
    QAction *openFileAct = new QAction("Open image file");
    QAction *saveFileAct = new QAction("Save image file");
    QAction *resetFileAct = new QAction("Reset file");

    // Our capture actions
    QAction *startCapture = new QAction("Start capture");
    QAction *endCapture = new QAction("End capture");

    // Our view mode actions
    QAction *lineViewAct = new QAction("Line View");
    QAction *dotsViewAct = new QAction("Dots only view");

    // The menus for these actions
    QMenu *fileBar = new QMenu("&File");
    QMenu *captureBar = new QMenu("&Capture");
    QMenu *viewBar = new QMenu("&View");

    // Our screen consists of scribbler on left and tabWidget on right (hidden)
    Scribbler *scribbler = new Scribbler();
    tabWidget = new QTabWidget();
    tabWidget->setHidden(true);

    //setCentralWidget(scribbler);
    QWidget *center = new QWidget;
    setCentralWidget(center);

    // Our layout reflects the above widgets w/ scribbler and QTabWidget
    QHBoxLayout *mainLayout = new QHBoxLayout(center);
    mainLayout->addWidget(scribbler);
    mainLayout->addWidget(tabWidget);

    // occupy equal space on scren
    mainLayout->setStretchFactor(scribbler, 1);
    mainLayout->setStretchFactor(tabWidget, 1);

    // Add our actions to menubar
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

    // Connect for signals and slots for actions w.r.t. Scribbler
    connect(openFileAct, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveFileAct, &QAction::triggered, this, &MainWindow::saveFile);
    connect(resetFileAct, &QAction::triggered, scribbler, &Scribbler::resetScribbler);
    connect(scribbler, &Scribbler::resetFile, this, &MainWindow::resetFile);

    // deal with start/end captures and redrawing upon openFile
    connect(startCapture, &QAction::triggered, scribbler, &Scribbler::startCapture);
    connect(endCapture, &QAction::triggered, scribbler, &Scribbler::endCapture);
    connect(this, &MainWindow::drawFromEvents, scribbler, &Scribbler::drawFromEvents);

    // When Scribbler::endCapture is triggured by menuBar action, scribbler responds with the events data.
    // With events data, process MouseEvents into QTableWidget
    connect(scribbler, &Scribbler::updateTabs, this, &MainWindow::updateTabs);

    // view modes
    connect(lineViewAct, &QAction::triggered, scribbler, &Scribbler::showLines);
    connect(dotsViewAct, &QAction::triggered, scribbler, &Scribbler::showDots);

    // directory persistence
    QSettings settings("JKW Systems", "Graphics1");
    dir = settings.value("dir", "").toString();
}

MainWindow::~MainWindow() {
    QSettings settings("JKW Systems", "Graphics1");
    settings.setValue("dir", dir);
}

void MainWindow::updateTabs(QList<MouseEvent> &events) {
    // NO drawings means NO table!
    if (events.isEmpty()) return;

    // events may be cleared by scribbler. Keep a copy of it to refer to in storedEvents. storedEvents to refer to events of other tabs later.
    QList<MouseEvent> *eventsCopy = new QList<MouseEvent>(events);
    storedEvents.append(eventsCopy);

    // Our table has as many rows as there are MouseEvents. There are 3 things: pos, action, time in MouseEvents to display
    QTableWidget *eventsTable = new QTableWidget();

    // Stretching automatically
    eventsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // https://stackoverflow.com/questions/3862900/how-to-disable-edit-mode-in-the-qtableview
    eventsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Sizeof table
    int colNum = 5;
    eventsTable->setRowCount(events.length());
    eventsTable->setColumnCount(colNum);

    for (int i = 0; i < events.length(); ++i) {

        // Table entry stuff
        QTableWidgetItem *posItem = new QTableWidgetItem();
        QTableWidgetItem *actItem = new QTableWidgetItem();
        QTableWidgetItem *timeItem = new QTableWidgetItem();
        QTableWidgetItem *distItem = new QTableWidgetItem();
        QTableWidgetItem *speedItem = new QTableWidgetItem();

        // Table entry formatting of strings from raw events
        QString posText = QString("(%1, %2)").arg(events[i].pos.x()).arg(events[i].pos.y());
        QString actionText;
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(events[i].time); //FROM: https://forum.qt.io/topic/77685/qtime-formatting-hh-mm-ss-s
        QString timeText = dateTime.toString("hh:mm:ss"); //FROM: https://forum.qt.io/topic/77685/qtime-formatting-hh-mm-ss-s
        QString distText = QString("%1").arg(events[i].distance);
        QString speedText = QString::number(events[i].speed, 'f', 2);

        // int to string mapping for actions
        switch (events[i].action) {
            case 0:
                actionText = "Click";
                break;
            case 1:
                actionText = "Move";
                break;
            case 2:
                actionText = "Release";
                break;
        }

        posItem->setData(Qt::DisplayRole, posText);
        actItem->setData(Qt::DisplayRole, actionText);
        timeItem->setData(Qt::DisplayRole, timeText);
        distItem->setData(Qt::DisplayRole, distText);
        speedItem->setData(Qt::DisplayRole, speedText);

        eventsTable->setItem(i, 0, posItem);
        eventsTable->setItem(i, 1, actItem);
        eventsTable->setItem(i, 2, timeItem);
        eventsTable->setItem(i, 3, distItem);
        eventsTable->setItem(i, 4, speedItem);
    }
    // Table headers
    QList<QString> tableLabels = {"Position", "Action", "Time", "Distance", "Speed"};
    eventsTable->setHorizontalHeaderLabels(tableLabels);
    eventsTable->setMinimumSize(400, 600);

    // updating adding label, etc... TabWidget is newly generated -> make visible.
    QString tabName = QString::number(tabCount);
    tabWidget->addTab(eventsTable, tabName);
    tabWidget->setHidden(false);
    tabWidget->show();
    ++tabCount;
}

void MainWindow::resetFile() {
    tabCount = 0;
    storedEvents.clear();
    tabWidget->clear();
    tabWidget->setHidden(true);
}

void MainWindow::saveFile() {
    // Outfile stuff for saving
    QString outFName = QFileDialog::getSaveFileName(this, "Save scribble file", dir);
    if (outFName.isEmpty()) return;

    // Error handling for bad file
    QFile outFile(outFName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "Error", QString("Can't write to file \"%1\"").arg(outFName));
        return;
    }

    // Save the number of tabs to file
    QDataStream saveOut(&outFile);
    int numTabs = tabWidget->count();
    saveOut << numTabs;

    // Iterate over all events in the list of copied *events
    for (QList<QList<MouseEvent>*>::Iterator eventsIt = storedEvents.begin(); eventsIt != storedEvents.end(); ++eventsIt) {
        QList<MouseEvent> *events = *eventsIt;

        // Save size of events to file
        int eventsCount = events->length();
        saveOut << eventsCount;

        // Save individual events to file
        for (MouseEvent &event : *events) {
            saveOut << event.pos;
            saveOut << event.action;
            saveOut << event.time;
            saveOut << event.distance;
            saveOut << event.speed;
        }
    }
    outFile.close();
}

void MainWindow::openFile() {
    // inFile stuff
    QString inFName = QFileDialog::getOpenFileName(this, "Load scribble file", dir);
    if (inFName.isEmpty()) {
        return;
    }

    // persistent dir stuff
    dir = QFileInfo(inFName).absolutePath();

    // Error checks
    QFile inFile(inFName);
    if (!inFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Failed to load file", inFName);
        return;
    }

    // Reset the file and window before opening the new file
    resetFile();

    QDataStream openIn(&inFile);

    int numTabs;
    int eventsCount;

    // set numTabs from file
    openIn >> numTabs;

    for (int tabIdx = 0; tabIdx < numTabs; ++tabIdx) {

        // set number of events in events list
        openIn >> eventsCount;

        QTableWidget *eventsTable = new QTableWidget();

        // Stretching automatically
        eventsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // https://stackoverflow.com/questions/3862900/how-to-disable-edit-mode-in-the-qtableview
        eventsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // Set size of table
        int colCount = 5;
        eventsTable->setColumnCount(colCount);
        eventsTable->setRowCount(eventsCount);

        QList<MouseEvent> *events = new QList<MouseEvent>();
        for (int i = 0; i < eventsCount; ++i) {
            QPointF pos;
            int action;
            quint64 time;
            float distance;
            float speed;

            // extract from stream data on event
            openIn >> pos;
            openIn >> action;
            openIn >> time;
            openIn >> distance;
            openIn >> speed;

            // create new event corresponding in saved events
            events->append(MouseEvent(action, pos, time, distance, speed));

            // tableWidget entry stuff
            QTableWidgetItem *posItem = new QTableWidgetItem();
            QTableWidgetItem *actItem = new QTableWidgetItem();
            QTableWidgetItem *timeItem = new QTableWidgetItem();
            QTableWidgetItem *distItem = new QTableWidgetItem();
            QTableWidgetItem *speedItem = new QTableWidgetItem();

            // Formatting to QString for text in table entry
            QString posText = QString("(%1, %2)").arg(events->at(i).pos.x()).arg(events->at(i).pos.y());
            QString actionText;
            QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(events->at(i).time); //FROM: https://forum.qt.io/topic/77685/qtime-formatting-hh-mm-ss-s
            QString timeText = dateTime.toString("hh:mm:ss"); //FROM: https://forum.qt.io/topic/77685/qtime-formatting-hh-mm-ss-s
            QString distText = QString("%1").arg(distance);
            QString speedText = QString::number(speed, 'f', 2);

            // int to string mapping for actions
            switch (events->at(i).action) {
            case 0:
                actionText = "Click";
                break;
            case 1:
                actionText = "Move";
                break;
            case 2:
                actionText = "Release";
                break;
            }

            posItem->setData(Qt::DisplayRole, posText);
            actItem->setData(Qt::DisplayRole, actionText);
            timeItem->setData(Qt::DisplayRole, timeText);
            distItem->setData(Qt::DisplayRole, distText);
            speedItem->setData(Qt::DisplayRole, speedText);

            eventsTable->setItem(i, 0, posItem);
            eventsTable->setItem(i, 1, actItem);
            eventsTable->setItem(i, 2, timeItem);
            eventsTable->setItem(i, 3, distItem);
            eventsTable->setItem(i, 4, speedItem);
        }
        // Table headers
        QList<QString> tableLabels = {"Position", "Action", "Time", "Distance", "Speed"};
        eventsTable->setHorizontalHeaderLabels(tableLabels);
        eventsTable->setMinimumSize(400, 600);

        // updating storedEvents to reflect most recent loaded events tab + adding label, etc...
        storedEvents.append(events);
        QString tabName = QString::number(tabIdx);
        tabWidget->addTab(eventsTable, tabName);
        tabWidget->setHidden(false);
        tabWidget->show();
        ++tabCount;
    }
    inFile.close();
    // send signal to scribbler so that it canr redraw with tabs info
    emit drawFromEvents(storedEvents);
}
