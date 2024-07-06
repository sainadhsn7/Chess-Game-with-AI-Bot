#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <board.h>
#include <fennotation.h>
#include "gamemanager.h"
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

class BoardUI;
class GameManager;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Board *board;
    Ui::MainWindow *ui;
    GameManager *gameManager;
    BoardUI *boardUI;

};
#endif // MAINWINDOW_H
