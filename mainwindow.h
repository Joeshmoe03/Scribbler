#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scribbler.h"

#include <QMainWindow>
#include <QGraphicsScene>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QList<MouseEvent> storedEvents;
    QTabWidget *tabWidget;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void receiveEvents(QList<MouseEvent> &data);
};
#endif // MAINWINDOW_H
