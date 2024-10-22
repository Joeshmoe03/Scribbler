#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>
#include <QTableWidget>

class MouseEvent {
public:
    enum {
        Press,
        Move,
        Release
    };
    int action;
    QPointF pos;
    quint64 time;

    MouseEvent(int _action, QPointF _pos, quint64 _time);

    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, const MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{
    QGraphicsScene scene;
    double lineWidth;
    QPointF lastPoint;
    QList<MouseEvent> events;
    bool isDots;

    QList<QGraphicsEllipseItem*> dots;
    QList<QGraphicsLineItem*> lines;

    Q_OBJECT

public:
    Scribbler();

    void resetScribbler();

    void startCapture();
    void endCapture();

    void showLines();
    void showDots();

public slots:
    void drawFromEvents(QList<QList<MouseEvent>*> &storedEvents);

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;

signals:
    void sendMouseEvents(QList<MouseEvent> &events);
    void resetFile();
};

#endif // SCRIBBLER_H
