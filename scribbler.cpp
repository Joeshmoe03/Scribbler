#include "scribbler.h"

#include <QtWidgets>
#include <math.h>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time, float _distance, float _speed, QList<QGraphicsItem*> _graphicsItems)
    : action(_action), pos(_pos), time(_time), distance(_distance), speed(_speed), graphicsItems(_graphicsItems) {}

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

    // We store dots and lines in graphicsGroup in a list for corresponding capture.
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
    float timeDiff = (float)(evt->timestamp() - prevTimestamp);
    if (timeDiff == 0) timeDiff = 1; //prevent zero division
    speed = distance / timeDiff;
    prevTimestamp = evt->timestamp();

    events << new MouseEvent(MouseEvent::Move, p, evt->timestamp(), distance, speed, QList<QGraphicsItem*>{dot, line});
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

    events << new MouseEvent(MouseEvent::Press, p, evt->timestamp(), distance, speed, QList<QGraphicsItem*>{dot});
}

void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);
    QPointF p = mapToScene(evt->pos());
    distance = sqrt(pow(p.x() - lastPoint.x(), 2.0) + pow(p.y() - lastPoint.y(), 2.0));
    float timeDiff = (float)(evt->timestamp() - prevTimestamp);
    if (timeDiff == 0) timeDiff = 1; //prevent zero division
    speed = distance / timeDiff;

    events << new MouseEvent(MouseEvent::Release, p, evt->timestamp(), distance, speed, QList<QGraphicsItem*>{});
}

void Scribbler::restoreColor() {
    for (QGraphicsLineItem *line : lines) {
        line->setPen(QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    }

    for (QGraphicsEllipseItem *dot : dots) {
        dot->setPen(QPen(Qt::black));
        dot->setBrush(QBrush(Qt::black));
    }
}

void Scribbler::highlightScribble(int currentTabIdx, QPair<int, int> rowSlice, QList<QList<MouseEvent*>*> &storedEvents) {
    for (int eventsIdx = 0; eventsIdx < storedEvents.length(); ++eventsIdx) {
        QList<MouseEvent*> *events = storedEvents[eventsIdx];

        for (int eventIdx = 0; eventIdx < events->length(); ++eventIdx) {
            MouseEvent *event = events->at(eventIdx);
            QBrush color;
            QPen pen;

            if (currentTabIdx == eventsIdx && eventIdx >= rowSlice.first && eventIdx <= rowSlice.second) {
                color = Qt::red;
                pen = QPen(Qt::red);
            } else {
                color = Qt::black;
                pen = QPen(Qt::black);
            }

            QList<QGraphicsItem*> graphicsItems;
            graphicsItems << event->graphicsItems;

            for (QGraphicsItem* graphicsItem : graphicsItems) {
                if (graphicsItem->type() == QGraphicsEllipseItem::Type) {
                    QGraphicsEllipseItem *dot = (QGraphicsEllipseItem*)graphicsItem;
                    dot->setPen(pen);
                    dot->setBrush(QBrush(color));
                }
                else if (graphicsItem->type() == QGraphicsLineItem::Type) {
                    QGraphicsLineItem *line = (QGraphicsLineItem*)graphicsItem;
                    line->setPen(QPen(color, lineWidth, Qt::SolidLine, Qt::FlatCap));
                }
            }
        }
    }
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

void Scribbler::resetCapture() {
    QList<QGraphicsItem*> groupItems;
    groupItems << graphicsGroup->childItems();

    // Clear the corresponding item from lines and dots to prevent conflicts when removing those dots and lines on resetCapture
    for (QGraphicsItem *item : groupItems) {
        if (lines.contains(item)) {
            lines.removeAll(item);
        }
        if (dots.contains(item)) {
            dots.removeAll(item);
        }
    }

    delete graphicsGroup; // discard the graphics Group not captured?
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
    events.clear();
}

/* One endCapture, scribbler sends data and clear QList<MouseEvent> */
void Scribbler::endCapture() {
    // don't capture for empty graphicsGroup
    if (events.isEmpty()) return;

    // reset graphicsGroup for new capture
    graphicsGroups.append(graphicsGroup);
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
    emit addTab(events);
    events.clear();
}

void Scribbler::showDots() {
    isDots = true;

    for (int i = 0; i < lines.length(); ++i) {
        QGraphicsLineItem *line = lines[i];
        line->setVisible(false);
    }
}

void Scribbler::showLines() {
    isDots = false;

    for (int i = 0; i < lines.length(); ++i) {
        QGraphicsLineItem *line = lines[i];
        line->setVisible(true);
    }
}

void Scribbler::drawFromEvents(QList<QList<MouseEvent*>*> &storedEvents) {
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
        QList<MouseEvent*>*events = storedEvents[eventsIdx];

        // We store dots and lines in graphicsGroup in a list for corresponding capture. We must iterate thru captures and reform these QItemGroups from storedEvents
        graphicsGroup = new QGraphicsItemGroup();
        scene.addItem(graphicsGroup);

        // deal with event in given events of a tab
        for (int eventIdx = 0; eventIdx < events->length(); ++eventIdx) {
            MouseEvent *event = events->at(eventIdx);
            QPointF p = event->pos;

            // Press actions add only dot. Move creates lines too
            switch (event->action) {
                case MouseEvent::Press: {
                    lastPoint = p;

                    // dots of Press
                    QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth)));
                    dot->setPen(Qt::NoPen);
                    dot->setBrush(Qt::black);

                    dots.append(dot);
                    graphicsGroup->addToGroup(dot);
                    event->graphicsItems.append(dot);
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
                    event->graphicsItems.append(dot);
                    event->graphicsItems.append(line);
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
    // new graphicsGroup to prevent new modifications of file from being included in previous modifications
    graphicsGroup = new QGraphicsItemGroup();
    scene.addItem(graphicsGroup);
}
