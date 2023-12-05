#include "gol_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <qlayout.h>
#include <qmouseevent>
#include <tuple>
#include <algorithm>

#include "gol_rle.h"

GoL_Widget::GoL_Widget(QWidget* parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);
}

void GoL_Widget::setGame(GoL_Game* gm) {
    if (game != nullptr) {
        delete game;
    }

    this->game = gm;

    // i dont like this; why do i have to make a useless variable
    auto gameSz = game->GetSize();
    auto& [w, h] = gameSz;

    zoom = 2;
    camX = cellSize * w / 2;
    camY = cellSize * h / 2;
}

void GoL_Widget::paintEvent(QPaintEvent* event) {
    QSize sz = this->size();

    int cSz = cellSize * zoom;

    auto gameSz = game->GetSize();
    auto& [w, h] = gameSz;

    int boundCamX = std::clamp(camX, -sz.width() / 2, (int)w * cSz - sz.width() / 2);
    int boundCamY = std::clamp(camY, -sz.height() / 2, (int)h * cSz - sz.width() / 2);

    int x_lCell = std::max<int>(0, floor(boundCamX / cSz));
    int x_rCell = std::min<int>(w, x_lCell + ceil(sz.width() / cSz) + 2);

    int y_tCell = std::max<int>(0, floor(boundCamY / cSz));
    int y_bCell = std::min<int>(h, y_tCell + ceil(sz.height() / cSz) + 2);

    QPainter painter(this);
    painter.setPen(pen);

    brush.setColor(emptyColor);
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    painter.setBrush(brush);

    bool lastState = false;

    // draw line-by-line
    for (int yc = y_tCell; yc < y_bCell; yc++) {
        int y = yc * cSz - boundCamY;

        for (int xc = x_lCell; xc < x_rCell; xc++) {
            int x = xc * cSz - boundCamX;

            bool curState = game->GetCellState(xc, yc);

            QColor& filledCol = filledColor;
            brush.setColor(curState ? filledCol : emptyColor);
            painter.setBrush(brush);
            painter.drawRect(x, y, cSz, cSz);
        }
    }

}

void GoL_Widget::CenterCamera() {
    auto gameSz = game->GetSize();
    auto& [w, h] = gameSz;

    int pxW = w * cellSize;
    int pxH = h * cellSize;

    camX = pxW - size().width() / 2;
    camY = pxH - size().height() / 2;
    zoom = 2;
}

void GoL_Widget::BoundCamera() {
    auto gameSz = game->GetSize();
    auto& [w, h] = gameSz;

    int cSz = cellSize * zoom;
    QSize sz = size();

    int boundCamX = std::clamp(camX, -sz.width() / 2, (int)w * cSz - sz.width() / 2);
    int boundCamY = std::clamp(camY, -sz.height() / 2, (int)h * cSz - sz.width() / 2);

    camX = boundCamX;
    camY = boundCamY;
}

std::tuple<int, int> GoL_Widget::posToCell(int x, int y) {
    int loc_x = x + camX;
    int loc_y = y + camY;
    int cSz = cellSize * zoom;

    return std::tuple<int, int>({ floor(loc_x / cSz), floor(loc_y / cSz) });
}

void GoL_Widget::setCell(int mx, int my, bool st) {
    auto gameSz = game->GetSize();
    auto& [w, h] = gameSz;

    auto unfunny = posToCell(mx, my);

    if (std::get<0>(unfunny) < 0 || std::get<1>(unfunny) < 0) {
        return;
    }
    
    if (std::get<0>(unfunny) >= w || std::get<1>(unfunny) >= h) {
        return;
    }

    qDebug() << "setting cell @" << mx << ", " << my <<
        "or (" << std::get<0>(unfunny) << ", " << std::get<1>(unfunny) << ")";

    game->SetState(std::get<0>(unfunny), std::get<1>(unfunny), st);
    repaint(); // changed a state; need ta repaint
}

void GoL_Widget::mousePressEvent(QMouseEvent* ev) {
    QPointF pt = ev->localPos();
    Qt::MouseButton mb = ev->button();

    if (mb == Qt::LeftButton) {
        setCell(pt.x(), pt.y(), true);
    } else if (mb == Qt::RightButton) {
        // setCell(pt.x(), pt.y(), false);
        mouseMoveDist = 0;
        lastMouse.setX(pt.x());
        lastMouse.setY(pt.y());
    }
}

/*

*/
void GoL_Widget::LoadRLE(std::string rle) {
    RLEToGame(rle, *game);

    CenterCamera();
    repaint();
}

void GoL_Widget::NextGeneration() {
    game->NextGeneration();
    repaint();
}

void GoL_Widget::mouseReleaseEvent(QMouseEvent* ev) {
    QPointF pt = ev->localPos();
    Qt::MouseButton mb = ev->button();

    if (mb == Qt::LeftButton) {
        // setCell(pt.x(), pt.y(), true);
    } else if (mb == Qt::RightButton) {
        // clamp only on release; c o o l    u x
        BoundCamera();

        if (mouseMoveDist < cellSize) {
            setCell(pt.x(), pt.y(), false);
        }
    }
}

void GoL_Widget::mouseMoveEvent(QMouseEvent* ev) {
    QPointF pt = ev->localPos();
    Qt::MouseButtons mb = ev->buttons();

    if (mb & Qt::LeftButton) {
        setCell(pt.x(), pt.y(), true);
    } else if (mb & Qt::RightButton) {
        camX -= ev->x() - lastMouse.x();
        camY -= ev->y() - lastMouse.y();

        mouseMoveDist += std::abs(ev->x() - lastMouse.x()) + std::abs(ev->y() - lastMouse.y());
        lastMouse.setX(ev->x());
        lastMouse.setY(ev->y());
        repaint();
    } else if (mb & Qt::MiddleButton) {
        setCell(pt.x(), pt.y(), false);
    }
}

void GoL_Widget::wheelEvent(QWheelEvent* ev) {
    int changeZoom = ev->angleDelta().y() / 8;
    QPointF pos = ev->position();

    double newZoom = zoom;
    if (changeZoom < 0) {
        newZoom -= .5;
    } else {
        newZoom += .5;
    }

    if (newZoom <= 1) {
        newZoom = 1;
    } else if (newZoom >= 5) {
        newZoom = 5;
    }

    double deltaScale = newZoom - zoom;

    double cx = (pos.x() + camX) / zoom;
    double cy = (pos.y() + camY) / zoom;

    int dcx = (cx * deltaScale);
    int dcy = (cy * deltaScale);

    camX += dcx;
    camY += dcy;

    BoundCamera();

    zoom = newZoom;
    
    repaint();
}