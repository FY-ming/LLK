#ifndef BASIC_MODE_H
#define BASIC_MODE_H

#include <QWidget>
#include <QPushButton>
#include <QPixmap>
#include <QVector>
#include <QPainter>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTimer>

// 开始 Qt 命名空间
QT_BEGIN_NAMESPACE
namespace Ui { class basic_mode; }
// 结束 Qt 命名空间
QT_END_NAMESPACE

// basic_mode 类的定义，继承自 QWidget
class basic_mode : public QWidget
{
    // 声明该类使用 Qt 的信号和槽机制
    Q_OBJECT

public:
    basic_mode(QWidget *parent = nullptr);    // 构造函数，parent 为父窗口指针，默认为 nullptr
    ~basic_mode();    // 析构函数

private slots:
    void on_BTN_START_clicked();    // “开始游戏” 按钮的点击事件处理函数
    void on_BTN_PAUSE_clicked();    // “暂停” 按钮的点击事件处理函数
    void on_BTN_TIP_clicked();      // “提示” 按钮的点击事件处理函数
    void on_BTN_REARRANGE_clicked();// “重排” 按钮的点击事件处理函数
    void clearHint();               // 清除提示的函数

protected:
    void paintEvent(QPaintEvent *event) override;         // 重写 QWidget 的 paintEvent 函数，用于绘制窗口内容
    void timerEvent(QTimerEvent *event) override;         // 重写 QWidget 的 timerEvent 函数，用于处理计时器事件
    void mousePressEvent(QMouseEvent *event) override;    // 重写 QWidget 的 mousePressEvent 函数，用于处理鼠标点击事件

private:
    Ui::basic_mode *ui;                     // 指向 UI 设计器生成的类的指针
    QVector<QVector<int>> mapData;          // 存储地图数据的二维向量，用于表示游戏中各个位置的图案信息
    QVector<QVector<int>> extendedMapData;  // 扩展后的地图数据，用于更方便地处理边界和连通性判断
    QPixmap elementPixmap;                  // 原图素材，存储游戏中使用的原始图片
    QPixmap maskPixmap;                     // 掩码素材，用于处理图片的透明部分
    QVector<QPixmap> elements;              // 素材库，存储提取的素材图片的向量，每个元素对应一个图案
    QPixmap backgroundPixmap;               // 背景图片，用于显示游戏的背景

    QPair<int, int> selectedPos1;           // 记录第一个选中图案的位置 (行, 列)
    QPair<int, int> selectedPos2;           // 记录第二个选中图案的位置 (行, 列)
    QPair<int, int> hintPos1;               // 用于存储提示的图案位置的第一个点
    QPair<int, int> hintPos2;               // 用于存储提示的图案位置的第二个点
    QVector<QPair<int, int>> connectionPath;// 记录连线路径的向量，用于存储两个可消除图案之间的连线点

    int score;                              // 记录游戏积分
    int gameTime;                           // 记录游戏剩余时间
    int timerId;                            // 计时器 ID，用于标识游戏倒计时的计时器
    bool gameOver;                          // 游戏是否结束的标志
    bool gamePaused;                        // 游戏是否暂停的标志
    bool isEliminating = false;             // 是否正在进行消除操作的标志

    void generateMap();             // 生成地图数据的函数
    void extractElements();         // 从图片中提取素材的函数
    void shuffleMap();              // 打乱地图中图案位置的函数
    void extendMap();               // 扩展地图数据的函数，增加边界空白区域

    bool canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2);                                    // 判断两个图案是否可以消除的函数（重载 1）
    bool canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2, QVector<QPair<int, int>> &path);    // 判断两个图案是否可以消除的函数（重载 2），同时返回连线路径
    void eliminatePatterns(const QPair<int, int> &pos1, const QPair<int, int> &pos2);    // 消除两个指定位置图案的函数

    void checkGameStatus();     // 检查游戏状态（是否所有图案都已消除）的函数
    void showWinMessage();      // 显示游戏胜利提示信息的函数
    void showLoseMessage();     // 显示游戏失败提示信息的函数

    void setButtonInteractions(bool enabled);   // 设置按钮交互状态（启用或禁用）的函数
};
#endif // BASIC_MODE_H
