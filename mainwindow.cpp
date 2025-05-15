#include <QCloseEvent>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

// 主窗口类的构造函数
// parent 是父窗口指针，默认为 nullptr
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 设置用户界面
    ui->setupUi(this);
    // 初始化基础模式窗口指针为 nullptr
    basic_modeWindow = nullptr;
}

// 主窗口类的析构函数
MainWindow::~MainWindow()
{
    // 释放用户界面对象的内存
    delete ui;
    // 如果基础模式窗口指针不为空，释放其内存
    if (basic_modeWindow) {
        delete basic_modeWindow;
    }
}

// 基础模式按钮点击事件处理函数
void MainWindow::on_IDC_BTN_BASIC_clicked()
{
    // 基础模式按钮点击事件
    // 如果基础模式窗口指针为空，说明还未创建基础模式窗口
    if (!basic_modeWindow) {
        // 创建一个新的基础模式窗口对象
        basic_modeWindow = new basic_mode(nullptr);
        // 连接 basic_mode 窗口的关闭信号到槽函数 showAgain
        // 当基础模式窗口被销毁时，会触发 showAgain 函数
        connect(basic_modeWindow, &basic_mode::destroyed, this, &MainWindow::showAgain);
        // 设置基础模式窗口的位置为 (500, 200)，可自行调整
        basic_modeWindow->move(500, 200);
    }
    // 显示基础模式窗口
    basic_modeWindow->show();
    // 隐藏主窗口
    this->hide();
}

// 重新显示主窗口的函数
// 当用户关闭了游戏窗口（基础模式窗口）时，会调用此函数返回主菜单
void MainWindow::showAgain()
{
    // 确保主窗口可见
    this->setVisible(true);
    // 确保主窗口可用
    this->setEnabled(true);
    // 显示主窗口
    this->show();
    // 将基础模式窗口指针置为 nullptr，表示窗口已关闭
    basic_modeWindow = nullptr;
}

// 重写的关闭事件处理函数
// 当用户尝试关闭主窗口时，会弹出确认对话框
void MainWindow::closeEvent(QCloseEvent *event)
{
    // MainWindow窗口关闭函数（弹出确认对话框）
    // 这里可以添加确认退出的逻辑，弹出确认对话框
    // 定义一个标准按钮类型的变量，用于存储用户的选择
    QMessageBox::StandardButton reply;
    // 弹出一个确认对话框，询问用户是否确定要退出应用程序
    reply = QMessageBox::question(this, "确认退出", "你确定要退出应用程序吗？",
                                  QMessageBox::Yes | QMessageBox::No);
    // 如果用户选择了 Yes
    if (reply == QMessageBox::Yes) {
        // 接受关闭事件，退出应用程序
        event->accept();
    } else {
        // 忽略关闭事件，不退出应用程序
        event->ignore();
    }
}
