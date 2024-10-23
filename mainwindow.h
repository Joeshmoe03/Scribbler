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
    void changeTab();

public slots:
    void addTab(QList<MouseEvent> &events);
    void resetFile();

signals:
    void adjustOpacity(int currentTabIdx);
    void drawFromEvents(QList<QList<MouseEvent>*> &storedEvents, int currentTabIdx);
};
#endif // MAINWINDOW_H
