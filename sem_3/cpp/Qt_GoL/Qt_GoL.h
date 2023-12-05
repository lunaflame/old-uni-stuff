#pragma once

#include <QtWidgets/QMainWindow>
#include <qtimer.h>
#include <qcheckbox.h>

#include "ui_Qt_GoL.h"
#include "gol_widget.h"
#include "gol_game.h"

class Qt_GoL : public QMainWindow
{
    Q_OBJECT

public:
    Qt_GoL(QWidget *parent = Q_NULLPTR);
    void StartGame();
    void StopGame();
    void FlipGame();

    void ResetGame();

    void LoadRLE();
    void LoadFileRLE();

    void update();

    void SyncData(); // syncs rules and field size data using the game's data

    double TPS = 5;
private:
    Ui::Qt_GoLClass ui;
    GoL_Widget* fieldWt;
    GoL_Game* game;
    QTimer timer;

    QCheckBox* survCboxes[10] = { nullptr };
    QCheckBox* reviveCboxes[10] = { nullptr }; // WTF

    bool playing = false;
    bool syncing = false; // giant hack tbh

    void createRuleCheckboxes(int num);
    void ruleChanged();
    void gameSizeChanged();

    void tryLoadRLE(QString rle);
};
