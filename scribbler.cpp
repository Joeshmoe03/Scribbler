#include "scribbler.h"

#include <QtWidgets>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time)
    : action(_action), pos(_pos), time(_time) {}

QDataStream &operator<<(QDataStream &out, const MouseEvent &evt) {
    return out << evt.action << evt.pos << evt.time;
}

QDataStream &operator>>(QDataStream &in, MouseEvent &evt) {
    return in >> evt.action >> evt.pos >> evt.time;
}

/* ============================= SCRIBBLER ================================ */
Scribbler::Scribbler()
    :lineWidth(4.0), isDots(false) {

    setScene(&scene);
    setSceneRect(QRectF(0.0, 0.0, 800.0, 600.0));
    setMinimumSize(QSize(600, 600));
    setRenderHint(QPainter::Antialiasing, true);
    setBackgroundBrush(Qt::white);

    scene.addRect(sceneRect());
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);
    QPointF p = mapToScene(evt->pos());
    QGraphicsLineItem *line = scene.addLine(QLineF(lastPoint, p), QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    lines.append(line);

    // If we are hiding the lines
    if (isDots) {
        line->setVisible(false);
    }

    QGraphicsEllipseItem *dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);
    dots.append(dot);
    lastPoint = p;

    events << MouseEvent(MouseEvent::Move, p, evt->timestamp());
}

void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt);
    QPointF p = mapToScene(evt->pos());
    lastPoint = p;
    QGraphicsEllipseItem *dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);
    dots.append(dot);

    events << MouseEvent(MouseEvent::Press, p, evt->timestamp());
}

void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);
    QPointF p = mapToScene(evt->pos());

    events << MouseEvent(MouseEvent::Release, p, evt->timestamp());
}

void Scribbler::resetScribbler() {
    events.clear();
    scene.clear();
    dots.clear();
    lines.clear();
    showLines();
    emit resetFile();
}

void Scribbler::startCapture() {
    events.clear();
}

/* One endCapture, scribbler sends data and clear QList<MouseEvent> */
void Scribbler::endCapture() {
    emit sendMouseEvents(events);
    events.clear();
}

void Scribbler::showDots() {
    isDots = true;

    for (int i = 0; i < dots.length(); ++i) {
        QGraphicsEllipseItem *dot = dots[i];
        dot->setVisible(true);
    }

    for (int i = 0; i < lines.length(); ++i) {
        QGraphicsLineItem *line = lines[i];
        line->setVisible(false);
    }
}

void Scribbler::showLines() {
    isDots = false;

    for (int i = 0; i < dots.length(); ++i) {
        QGraphicsEllipseItem *dot = dots[i];
        dot->setVisible(true);
    }

    for (int i = 0; i < lines.length(); ++i) {
        QGraphicsLineItem *line = lines[i];
        line->setVisible(true);
    }
}

void Scribbler::drawFromEvents(QList<QList<MouseEvent>*> &storedEvents) {
    // reset before redrawing after loading old file
    events.clear();
    scene.clear();
    dots.clear();
    lines.clear();
    showLines();

    // stored events across multiple tabs
    for (QList<MouseEvent> *events : storedEvents) {
        QPointF lastPoint;

        // deal with event in given events of a tab
        for (MouseEvent &event : *events) {
            QPointF p = event.pos;

            // Press actions add only dot. Move creates lines too
            switch (event.action) {
                case MouseEvent::Press: {
                    lastPoint = p;

                    // dots of Press
                    QGraphicsEllipseItem *dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);
                    dots.append(dot);
                    break;
                }
                case MouseEvent::Move: {
                    // lines of Move
                    QGraphicsLineItem *line = scene.addLine(QLineF(lastPoint, p), QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
                    lines.append(line);

                    // dots of Move
                    QGraphicsEllipseItem *dot = scene.addEllipse(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);
                    dots.append(dot);
                    lastPoint = p;
                    break;
                }
                case MouseEvent::Release: {
                    break;
                }
            }
        }
    }
}
