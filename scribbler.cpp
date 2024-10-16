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
    setMinimumSize(QSize(800, 600));
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

void Scribbler::openFile() {
    qDebug() << "open file";
}

void Scribbler::saveFile() {
    qDebug() << "save file";
}

void Scribbler::resetFile() {
//    scene.clear();
}

void Scribbler::startCapture() {
//     emit sendScribblerData(events);
}

void Scribbler::endCapture() {
    emit reportEvents(events);
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
