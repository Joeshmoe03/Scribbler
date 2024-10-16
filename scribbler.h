#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>

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
    void startCapture();
    void endCapture();
    void openFile();
    void saveFile();
    void resetFile();
    void showLines();
    void showDots();

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;

signals:
    void reportEvents(QList<MouseEvent> &data);
};

#endif // SCRIBBLER_H
