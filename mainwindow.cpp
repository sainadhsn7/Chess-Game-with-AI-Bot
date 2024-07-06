#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMessageBox>
#include "gamemanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameManager = new GameManager(this, this);
    gameManager->start();
    // Board board;
    // board.loadStartPosition();
    // boardUI = new BoardUI(this);
    // boardUI->updatePosition(&board);
    // MoveGenerator mg;
    // qDebug() << mg.GenerateMoves(&board).size();
}


MainWindow::~MainWindow()
{
    delete ui;
    // delete gameManager;
}
