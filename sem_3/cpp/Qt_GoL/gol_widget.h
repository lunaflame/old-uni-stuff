#pragma once

#include <QBrush>
#include <QPen>
#include <QWidget>
#include <tuple>
#include <vector>

#include "gol_game.h"

class GoL_Widget : public QWidget
{
    Q_OBJECT

public:

    explicit GoL_Widget(QWidget* parent = nullptr);
    
    // setup
    void setGame(GoL_Game* game);
    void LoadRLE(std::string rle);

    // operation
    void NextGeneration();

    // display
    QColor emptyColor = QColor(50, 50, 50);
    QColor filledColor = QColor(50, 150, 250);

    // This was a bad idea
    // std::vector<QColor> colors = { QColor(50, 210, 50), QColor(210, 50, 50), QColor(20, 20, 210) };
    // std::vector<QColor> colors = { QColor(30, 190, 230), filledColor };

    void CenterCamera();
    void BoundCamera();
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

private:
    std::tuple<int, int> posToCell(int x, int y);
    void setCell(int mx, int my, bool st);

    int camX = 0;
    int camY = 0;
    double zoom = 1;

    const int cellSize = 8;

    GoL_Game* game = nullptr; // pointer because it's not set up (need w/h)

    QPen pen;
    QBrush brush;
    QPoint lastMouse;
    int mouseMoveDist = 0;
    // QPixmap pixmap;
};