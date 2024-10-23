#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>
#include <QTableWidget>

class MouseEvent {
public:
    enum {
        Press,
        Move,
        Release,
        Distance,
        Speed,
    };
    int action;
    QPointF pos;
    quint64 time;
    float distance;
    float speed;

    MouseEvent(int _action, QPointF _pos, quint64 _time, float _distance, float _speed);

    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, const MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{
    QGraphicsScene scene;
    double lineWidth;
    QPointF lastPoint;
    float distance;
    float speed;
    quint64 prevTimestamp;
    QList<MouseEvent> events;
    bool isDots;

    QList<QGraphicsEllipseItem*> dots;
    QList<QGraphicsLineItem*> lines;
    QList<QGraphicsItemGroup*> graphicsGroups;
    QGraphicsItemGroup *graphicsGroup;

    Q_OBJECT

public:
    Scribbler();

    void resetScribbler();

    void startCapture();
    void endCapture();

    void showLines();
    void showDots();

public slots:
    void drawFromEvents(QList<QList<MouseEvent>*> &storedEvents, int currentTabIdx);
    void adjustOpacity(int currentTabIdx);

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;

signals:
    void addTab(QList<MouseEvent> &events);
    void resetFile();
};

#endif // SCRIBBLER_H
