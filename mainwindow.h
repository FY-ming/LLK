#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "basic_mode.h"

// 开始 Qt 命名空间
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
// 结束 Qt 命名空间
QT_END_NAMESPACE

// 主窗口类的定义，继承自 QMainWindow
class MainWindow : public QMainWindow
{
    // 声明该类使用 Qt 的信号和槽机制
    Q_OBJECT

public:
    // 主窗口类的构造函数，父窗口指针默认为 nullptr
    MainWindow(QWidget *parent = nullptr);
    // 主窗口类的析构函数
    ~MainWindow();

protected:
    // 重写 QMainWindow 的 closeEvent 函数
    // 用于处理主窗口关闭事件
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 基础模式按钮点击事件处理函数
    void on_IDC_BTN_BASIC_clicked();
    // 重新显示主窗口的函数
    void showAgain();

private:
    // 指向用户界面对象的指针
    Ui::MainWindow *ui;
    // 指向基础模式窗口对象的指针
    basic_mode *basic_modeWindow;
};

#endif // MAINWINDOW_H
