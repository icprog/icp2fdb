#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include "autostopthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
public slots:
    void PushButtonRead();
    void TextChanged(int iUzel,QString newText);
    void InsertChanged(QString Text);
};


class ThreadPollObjects : public AutoStopThread
{
   Q_OBJECT
public:
    void run();
    void PollVosn(int i);
    void Poll1Uvr2Nord(int i);
    void Poll4Uvr(int i);
signals:
    void textchange(int iUzel,QString newText);
    void insert(QString Text);
};


#endif // MAINWINDOW_H
