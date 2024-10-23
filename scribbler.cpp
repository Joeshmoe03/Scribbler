#include "scribbler.h"

#include <QtWidgets>
#include <math.h>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time, float _distance, float _speed)
    : action(_action), pos(_pos), time(_time), distance(_distance), speed(_speed) {}

QDataStream &operator<<(QDataStream &out, const MouseEvent &evt) {
    return out << evt.action << evt.pos << evt.time << evt.distance << evt.speed;
}

QDataStream &operator>>(QDataStream &in, MouseEvent &evt) {
    return in >> evt.action >> evt.pos >> evt.time >> evt.distance >> evt.speed;
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
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);
    QPointF p = mapToScene(evt->pos());

    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(lastPoint, p));
    line->setPen(QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));

    lines.append(line);
    graphicsGroup->addToGroup(line);

    // If we are hiding the lines
    if (isDots) {
        line->setVisible(false);
    }

    QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)));
    dot->setPen(Qt::NoPen);
    dot->setBrush(Qt::black);

    dots.append(dot);
    graphicsGroup->addToGroup(dot);

    distance = sqrt(pow(p.x() - lastPoint.x(), 2.0) + pow(p.y() - lastPoint.y(), 2.0));
    lastPoint = p;
    speed = distance / (float)(evt->timestamp() - prevTimestamp);
    prevTimestamp = evt->timestamp();


    events << MouseEvent(MouseEvent::Move, p, evt->timestamp(), distance, speed);
}

void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt);
    QPointF p = mapToScene(evt->pos());
    lastPoint = p;

    QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)));
    dot->setPen(Qt::NoPen);
    dot->setBrush(Qt::black);

    dots.append(dot);
    graphicsGroup->addToGroup(dot);

    distance = 0.0;
    speed = 0.0;
    prevTimestamp = evt->timestamp();

    events << MouseEvent(MouseEvent::Press, p, evt->timestamp(), distance, speed);
}

void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);
    QPointF p = mapToScene(evt->pos());
    distance = sqrt(pow(p.x() - lastPoint.x(), 2.0) + pow(p.y() - lastPoint.y(), 2.0));
    speed = distance / (float)(evt->timestamp() - prevTimestamp);

    events << MouseEvent(MouseEvent::Release, p, evt->timestamp(), distance, speed);
}

void Scribbler::adjustOpacity(int currentTabIdx) {
    for (int i = 0; i < graphicsGroups.length(); ++i) {
        QGraphicsItemGroup *group = graphicsGroups[i];
        if (currentTabIdx == i) {
            group->setOpacity(1.0);
        } else {
            group->setOpacity(0.25);
        }
    }
}

void Scribbler::resetScribbler() {
    events.clear();
    scene.clear();
    dots.clear();
    lines.clear();
    showLines();
    graphicsGroups.clear();
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
    emit resetFile();
}

void Scribbler::startCapture() {
    events.clear();
}

/* One endCapture, scribbler sends data and clear QList<MouseEvent> */
void Scribbler::endCapture() {
    graphicsGroups.append(graphicsGroup);
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
    emit addTab(events);
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

void Scribbler::drawFromEvents(QList<QList<MouseEvent>*> &storedEvents, int currentTabIdx) {
    // reset before redrawing after loading old file or dealing with opacity
    events.clear();
    scene.clear();
    dots.clear();
    lines.clear();
    graphicsGroups.clear();
    showLines(); //DO I WANT TO RESET VIEW DOTS/LINES MODE WHEN OPENING FILE?

    // Nothing to draw
    if (storedEvents.isEmpty()) return;

    // stored events across multiple tabs
    for (int eventsIdx = 0; eventsIdx < storedEvents.length(); ++eventsIdx) {
        QList<MouseEvent> *events = storedEvents[eventsIdx];
        graphicsGroup = new QGraphicsItemGroup();
        scene.addItem(graphicsGroup);

        // deal with event in given events of a tab
        for (QList<MouseEvent>::Iterator eventIt = events->begin(); eventIt != events->end(); ++eventIt) {
            MouseEvent &event = *eventIt;
            QPointF p = event.pos;

            // Press actions add only dot. Move creates lines too
            switch (event.action) {
                case MouseEvent::Press: {
                    lastPoint = p;

                    // dots of Press
                    QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)));
                    dot->setPen(Qt::NoPen);
                    dot->setBrush(Qt::black);

                    dots.append(dot);
                    graphicsGroup->addToGroup(dot);
                    break;
                }
                case MouseEvent::Move: {
                    // lines of Move
                    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(lastPoint, p));
                    line->setPen(QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));

                    lines.append(line);
                    graphicsGroup->addToGroup(line);

                    // dots of Move
                    QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)));
                    dot->setPen(Qt::NoPen);
                    dot->setBrush(Qt::black);

                    dots.append(dot);
                    graphicsGroup->addToGroup(dot);
                    lastPoint = p;

                    break;
                }
                case MouseEvent::Release: {
                    break;
                }
            }
        }
        graphicsGroups.append(graphicsGroup);
    }
    // new graphicsGroup to prevent new modifications of file from being including in previous modifications
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
    adjustOpacity(currentTabIdx);
}
