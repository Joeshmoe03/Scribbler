#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scribbler.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QGraphicsScene>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QList<QList<MouseEvent*>*> storedEvents;
    QTabWidget *tabWidget;
    QString dir;
    int tabCount;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void saveFile();
    void openFile();
    void changeTab();
    void itemSelectionChanged();

public slots:
    void addTab(QList<MouseEvent*> &events);
    void resetFile();

signals:
    void adjustOpacity(int currentTabIdx);
    void drawFromEvents(QList<QList<MouseEvent*>*> &storedEvents);
    void highlightScribble(int currentTabIdx, QPair<int, int> rowSlice, QList<QList<MouseEvent*>*> &storedEvents);
    void restoreColor();
};
#endif // MAINWINDOW_H
