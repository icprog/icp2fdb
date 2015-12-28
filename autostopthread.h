
/*
���� ��� ������������ ����������������
�������������� ������� ��� �������� ���������
(��� ������ ������������ ��������)
�������������:
1. ����������� �� ����
2. � ���������������� ������ run() � ���������� ����� ����� ��������� ��������
if (CheckThreadStop()) return;

�������� ��� ���:
1. ��� ������ ����������� ��������� ���� ������������� ��������� ������ �
���������� wait() - ������� �������� ���������� ������.
2. ����� ������� �� ����������� ����� �������� CheckThreadStop() �
����������� return-��.
3. ���������� ����� �� wait() � ����������� � �� ��������� ������.
*/

#ifndef AUTOSTOPTHREAD_H
#define AUTOSTOPTHREAD_H

#include <QThread>

class AutoStopThread: public QThread
{
    Q_OBJECT
 private:
      volatile bool stopping;
 public:
    AutoStopThread(){stopping=false;}
    ~AutoStopThread(){stopping=true;wait();}
    bool CheckThreadStop(){return stopping;}
};
#endif // AUTOSTOPTHREAD_H
