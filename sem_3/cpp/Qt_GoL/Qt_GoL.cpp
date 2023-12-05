#include "Qt_GoL.h"

#include <qtimer.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qspinbox.h>

#include <exception>
#include <assert.h>

#include "gol_widget.h"
#include "gol_game.h"

Qt_GoL::Qt_GoL(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    const int defaultSize = 64;
    game = new GoL_Game(defaultSize, defaultSize);
    ui.gol_field->setGame(game);

    for (int i = 1; i <= 9; i++) {
        createRuleCheckboxes(i);
    }

    connect(ui.startBtn, &QPushButton::released, this, &Qt_GoL::FlipGame);
    connect(ui.resetBtn, &QPushButton::released, this, &Qt_GoL::ResetGame);

    connect(ui.importRLEBtn, &QPushButton::released, this, &Qt_GoL::LoadRLE);
    connect(ui.openFileBtn, &QPushButton::released, this, &Qt_GoL::LoadFileRLE);
    
    connect(ui.widthSpin, &QSpinBox::valueChanged, this, &Qt_GoL::gameSizeChanged);
    ui.widthSpin->setMaximum(GoL_Game::MaxWidth);

    connect(ui.heightSpin, &QSpinBox::valueChanged, this, &Qt_GoL::gameSizeChanged);
    ui.heightSpin->setMaximum(GoL_Game::MaxHeight);

    connect(&timer, &QTimer::timeout, this, QOverload<>::of(&Qt_GoL::update));
    timer.start(1000 / TPS);

    SyncData();
}

void Qt_GoL::createRuleCheckboxes(int num) {
    assert(game != nullptr);

    QCheckBox* checkBox = new QCheckBox(ui.rs_checkboxes);
    checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    checkBox->setFixedSize(16, 16);
    ui.rs_layout_checkboxes->addWidget(checkBox);

    QLabel* label = new QLabel(ui.rs_labels);
    label->setText(QString::fromUtf8(std::to_string(num)));
    label->setObjectName(QString::fromUtf8("unpog")); //std::to_string(num)));
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    label->setFixedSize(16, 16);

    ui.rs_layout_labels->addWidget(label);

    // for revive rule
    QCheckBox* r_checkBox = new QCheckBox(ui.rr_checkboxes);
    r_checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    r_checkBox->setFixedSize(16, 16);

    ui.rr_layout_checkboxes->addWidget(r_checkBox);

    QLabel* r_label = new QLabel(ui.rr_labels);
    r_label->setText(QString::fromUtf8(std::to_string(num)));
    r_label->setObjectName(QString::fromUtf8("unpog")); //std::to_string(num)));
    r_label->setAlignment(Qt::AlignCenter);
    r_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    r_label->setFixedSize(16, 16);

    ui.rr_layout_labels->addWidget(r_label);

    survCboxes[num] = checkBox;
    reviveCboxes[num] = r_checkBox;

    checkBox->setChecked(game->GetSurvivalRule(num));
    r_checkBox->setChecked(game->GetRevivalRule(num));

    // i don't think you can carry data over (wtf?) via the signal
    // so instead, ruleChanged will recalculate all rules
    connect(checkBox, &QCheckBox::stateChanged, this, &Qt_GoL::ruleChanged);
    connect(r_checkBox, &QCheckBox::stateChanged, this, &Qt_GoL::ruleChanged);
}

void Qt_GoL::SyncData() {
    syncing = true;

    for (int num = 1; num <= 9; num++) {
        survCboxes[num]->setChecked(game->GetSurvivalRule(num));
        reviveCboxes[num]->setChecked(game->GetRevivalRule(num));
    }

    auto gSz = game->GetSize();
    auto& [w, h] = gSz;

    ui.widthSpin->setValue(w);
    ui.heightSpin->setValue(h);
    gameSizeChanged();

    syncing = false;
}

void Qt_GoL::gameSizeChanged() {
    size_t want_w = ui.widthSpin->value();
    size_t want_h = ui.heightSpin->value();

    auto gSz = game->GetSize();
    auto& [w, h] = gSz;

    if (want_w != w || want_h != h) {
        ui.resetBtn->setText("Reset and Resize");
    } else {
        ui.resetBtn->setText("Reset");
    }
}

void Qt_GoL::ruleChanged() {
    // this sucks
    if (syncing) return;

    for (int i = 1; i <= 9; i++) {
        if (survCboxes[i] == nullptr) continue;
        QCheckBox* cbox = survCboxes[i];

        game->SetSurvivalRule(i, cbox->isChecked());
    }

    for (int i = 1; i <= 9; i++) {
        if (reviveCboxes[i] == nullptr) continue;
        QCheckBox* cbox = reviveCboxes[i];

        game->SetRevivalRule(i, cbox->isChecked());
    }

}

void Qt_GoL::ResetGame() {
    game->ResetSize(ui.widthSpin->value(), ui.heightSpin->value());
    ui.gol_field->CenterCamera();
    ui.gol_field->repaint();
    SyncData();
}

void Qt_GoL::StartGame() {
    playing = true;
    ui.startBtn->setText("Stop");
}

void Qt_GoL::StopGame() {
    playing = false;
    ui.startBtn->setText("Start");
}

void Qt_GoL::tryLoadRLE(QString rle) {
    try {
        ui.gol_field->LoadRLE(rle.toStdString());
        SyncData();
    }
    catch (std::exception ex) {
        qDebug() << "Got exception: " << ex.what();
        QString err = "Error while reading RLE: ";
        err += ex.what();

        QMessageBox messageBox;
        messageBox.critical(0, "Error", err);
        messageBox.setFixedSize(600, 200);

    }
}

void Qt_GoL::LoadRLE() {
    bool ok;
    QString text = QInputDialog::getMultiLineText(this, tr("QInputDialog::getText()"),
        tr("Input RLE:"),
        "#C Example glider.\n"
        "x = 7, y = 7\n"
        "$$3bo$4bo$2b3o!", &ok);

    if (ok && !text.isEmpty()) {
        tryLoadRLE(text);
    }
}

void Qt_GoL::LoadFileRLE() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open RLE file"), "",
        tr("All Files (*);;RLE format (*.rle)"));

    if (!fileName.isEmpty()) {

        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }

        QString text(file.readAll());
        
        if (text.isEmpty()) {
            QMessageBox::information(this, tr("File was empty."),
                file.errorString());
            return;
        }

        tryLoadRLE(text);
    }
}

void Qt_GoL::FlipGame() {
    if (playing) {
        StopGame();
    } else {
        StartGame();
    }
}

void Qt_GoL::update() {
    // qDebug() << "update: " << playing;
    if (playing) {
        ui.gol_field->NextGeneration();
    }
}