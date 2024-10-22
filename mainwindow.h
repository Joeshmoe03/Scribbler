#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scribbler.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QGraphicsScene>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QList<QList<MouseEvent>*> storedEvents;
    QTabWidget *tabWidget;
    QString dir;
    int tabCount;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void saveFile();
    void openFile();

public slots:
    void receiveMouseEvents(QList<MouseEvent> &events);
    void resetFile();

signals:
    void drawFromEvents(QList<QList<MouseEvent>*> &storedEvents);
};
#endif // MAINWINDOW_H
