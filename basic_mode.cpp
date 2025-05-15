#include "basic_mode.h"
#include "ui_basic_mode.h"
#include <queue>
#include <QRegularExpression>

// basic_mode 类的构造函数
basic_mode::basic_mode(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::basic_mode)
{
    ui->setupUi(this);    // 设置用户界面
    this->setAttribute(Qt::WA_DeleteOnClose); // 设置窗口关闭时自动销毁，为了触发MainWindow窗口的showAgain函数
    extractElements(); // 从图片中提取游戏素材
    backgroundPixmap.load(":/resource/fruit_b0g.bmp"); // 加载背景图片，绘制到窗口
    score = 0;    // 初始化积分
    gameTime = 300;    // 初始化游戏剩余时间
    gameOver = true;    // 初始化游戏状态为结束，（为了方便控制按钮状态）
    selectedPos1 = {-1, -1};    // 初始化第一个选中图案的位置为无效位置
    selectedPos2 = {-1, -1};    // 初始化第二个选中图案的位置为无效位置
    // 初始化提示图案的位置为无效位置
    hintPos1 = {-1, -1};
    hintPos2 = {-1, -1};
    // 开始游戏前禁用其他按钮，只启用 “开始游戏” 按钮
    setButtonInteractions(false);
    ui->BTN_START->setEnabled(true);
}

// basic_mode 类的析构函数
basic_mode::~basic_mode()
{
    // 释放用户界面对象的内存
    delete ui;
}

// “开始游戏” 按钮的点击事件处理函数
void basic_mode::on_BTN_START_clicked()
{
    // 当用户点击 “开始游戏” 按钮时，调用 generateMap 函数生成地图数据，并调用 update 函数触发重绘事件。
    ui->BTN_START->setEnabled(false);   // 禁用开始游戏按钮，防止重复点击
    setButtonInteractions(true);        // 启用其他按钮（如提示、重排、暂停等）

    generateMap();    // 生成游戏地图数据
    // extendMap();   // 扩展地图数据,为了通过验收而被注释
    update();      // 触发窗口重绘事件，更新游戏界面显示

    gameTime = 300;    // 初始化倒计时功能，设置游戏剩余时间为 300 秒
    timerId = startTimer(1000);      // 启动计时器，每秒触发一次 timerEvent 函数
    gameOver = false;    // 设置游戏状态为未结束
    gamePaused = false;    // 设置游戏状态为未暂停
}

// 计时器事件处理函数
void basic_mode::timerEvent(QTimerEvent *event)
{
    // 检查当前事件的计时器 ID 是否为游戏倒计时的计时器 ID
    if (event->timerId() == timerId) {
        gameTime--;// 减少游戏剩余时间
        ui->CountDownBar->setValue(gameTime);// 更新进度条显示游戏剩余时间
        // 如果游戏剩余时间小于等于 0
        if (gameTime <= 0) {
            killTimer(timerId);// 停止计时器
            showLoseMessage();// 显示游戏失败提示信息
            // 禁用其他按钮，只启用 “开始游戏” 按钮
            setButtonInteractions(false);
            ui->BTN_START->setEnabled(true);
        } else {
            checkGameStatus();// 检查游戏状态（是否所有图案都已消除）
        }
    }
    // 调用父类的 timerEvent 函数，处理其他计时器事件
    QWidget::timerEvent(event);
}

    // 检查游戏状态，判断是否已经完成游戏
void basic_mode::checkGameStatus()
{
    bool allEliminated = true;    // 假设所有图案都已消除
    // 遍历地图数据的每一个位置
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; ++j) {
            // 如果有任何一个位置的图案未消除
            if (mapData[i][j] != -1) {
                // 设置所有图案都已消除的标志为 false
                allEliminated = false;
                break;
            }
        }
        if (!allEliminated) break;
    }

    // 如果所有图案都已消除
    if (allEliminated) {
        killTimer(timerId);      // 停止计时器
        showWinMessage();        // 显示游戏胜利提示信息
        // 禁用其他按钮，只启用 “开始游戏” 按钮
        setButtonInteractions(false);
        ui->BTN_START->setEnabled(true);
    }
}

// 显示游戏胜利提示信息的函数
void basic_mode::showWinMessage()
{
    QMessageBox::information(this, "游戏胜利", "恭喜你，成功消除所有图案！");
    gameOver = true;
}
// 显示游戏失败提示信息的函数
void basic_mode::showLoseMessage()
{
    QMessageBox::information(this, "游戏失败", "时间已到，未能消除所有图案！");
    gameOver = true;
}

// 绘制事件处理函数，用于绘制游戏界面
void basic_mode::paintEvent(QPaintEvent *event)
{
    // 根据生成的地图数据，将对应的素材图片绘制到游戏窗口中。

    Q_UNUSED(event);// 忽略事件参数
    QPainter painter(this);// 创建 QPainter 对象，用于绘制图形

    // 绘制背景图片
    if (!backgroundPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), backgroundPixmap);
    }

    if (!mapData.isEmpty()) {
        // 偏移量，用于修改地图的显示位置
        int offsetX = 45;  // 水平偏移量
        int offsetY = 70;   // 垂直偏移量
        // 遍历地图数据的每一个位置，用于绘制每一处的图案
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 16; ++j) {
                // 检查位置是否在有效范围内
                if (i >= 0 && i < mapData.size() && j >= 0 && j < mapData[i].size()) {
                    // 获取当前位置的图案索引
                    int elementIndex = mapData[i][j];
                    // 如果图案索引有效且在素材向量范围内
                    if (elementIndex != -1 && elementIndex < elements.size()) {
                        // 绘制图案
                        painter.drawPixmap(j * 40 + offsetX, i * 40 + offsetY, elements[elementIndex]);
                        // 绘制选中图案的边框
                        if ((i == selectedPos1.first && j == selectedPos1.second) ||
                            (i == selectedPos2.first && j == selectedPos2.second)) {
                            painter.setPen(QPen(Qt::blue,3)); // 选中图案的蓝色边框
                            painter.drawRect(j * 40 + offsetX, i * 40 + offsetY, 40, 40);
                            painter.setPen(Qt::black);
                        }
                        // 绘制提示图案的边框
                        if ((i == hintPos1.first && j == hintPos1.second) ||
                            (i == hintPos2.first && j == hintPos2.second)) {
                            painter.setPen(QPen(Qt::red,3)); // 提示的图案用红色边框
                            painter.drawRect(j * 40 + offsetX, i * 40 + offsetY, 40, 40);
                            painter.setPen(Qt::black);
                        }
                    }
                }
            }
        }
    }

    // 绘制连线
    if (connectionPath.size() >= 2) {
        int offsetX = 45;
        int offsetY = 70;
        painter.setPen(QPen(Qt::blue, 3));
        // 遍历连线路径的每一段
        for (int i = 0; i < connectionPath.size() - 1; ++i) {
            // 由于不再扩展地图，不再需要-1
            // int row1 = connectionPath[i].first - 1;
            // int col1 = connectionPath[i].second - 1;
            // int row2 = connectionPath[i + 1].first - 1;
            // int col2 = connectionPath[i + 1].second - 1;
            int row1 = connectionPath[i].first;
            int col1 = connectionPath[i].second;
            int row2 = connectionPath[i + 1].first;
            int col2 = connectionPath[i + 1].second;
            // 绘制连线
            painter.drawLine(col1 * 40 + offsetX + 20, row1 * 40 + offsetY + 20,
                             col2 * 40 + offsetX + 20, row2 * 40 + offsetY + 20);
        }
    }
}

void basic_mode::generateMap()
{
    // 生成一个 10 行 16 列的地图数据，每种素材的数量为偶数。使用 QRandomGenerator 随机选择素材并填充到地图中。
    mapData.clear();// 清空地图数据
    mapData.resize(10, QVector<int>(16, -1));// 调整地图数据的大小为 10 行 16 列，并初始化为 -1（表示空白）

    // 顺序填充地图
    int elementIndex = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; j += 2) {
            // 填充相邻两个位置为相同的图案索引
            mapData[i][j] = elementIndex;
            mapData[i][j + 1] = elementIndex;
            elementIndex = (elementIndex + 1) % elements.size();// 循环选择下一个图案索引
        }
    }

    // 打乱地图
    shuffleMap();
}

// 打乱地图中图案位置的函数，将有序地图变得无序
void basic_mode::shuffleMap()
{
    // 打乱按照规则排序的地图素材
    const int shuffleTimes = 100;  // 打乱次数
    for (int i = 0; i < shuffleTimes; ++i) {
        // 随机选择两个位置
        int row1 = QRandomGenerator::global()->bounded(10);
        int col1 = QRandomGenerator::global()->bounded(16);
        int row2 = QRandomGenerator::global()->bounded(10);
        int col2 = QRandomGenerator::global()->bounded(16);

        // 交换两个位置的素材
        int temp = mapData[row1][col1];
        mapData[row1][col1] = mapData[row2][col2];
        mapData[row2][col2] = temp;
    }
}

// 从图片中提取游戏素材的函数
void basic_mode::extractElements()
{
    // 该函数用于加载 fruit_element.bmp 和 fruit_mask.bmp 图片，并从中提取出每个 40x40 的素材，将掩码应用到原图上，存储到 elements 向量中。
    elementPixmap.load(":/resource/fruit_element.bmp");    // 加载原图素材图片
    maskPixmap.load(":/resource/fruit_mask.bmp");    // 加载掩码素材图片

    // 遍历提取每个 40x40 的素材图片
    for (int i = 0; i < 20; ++i) {
        QPixmap element = elementPixmap.copy(0, i * 40, 40, 40);// 从原图中复制一个 40x40 的区域
        QPixmap mask = maskPixmap.copy(0, i * 40, 40, 40);// 从掩码图中复制一个 40x40 的区域
        element.setMask(mask.createMaskFromColor(Qt::black));// 将掩码应用到素材图片上，设置透明部分
        elements.append(element);// 将处理后的素材图片添加到素材向量中
    }
}

// 鼠标点击事件处理函数
void basic_mode::mousePressEvent(QMouseEvent *event)
{
    // 如果游戏未结束且未暂停
    if (!gameOver && !gamePaused) {
        QPoint position = event->position().toPoint();// 获取鼠标点击的位置
        // 检查当前是否不在进行消除操作
        if (!isEliminating) {
            // 获取鼠标点击位置的 x 和 y 坐标
            int x = position.x();
            int y = position.y();
            // 根据鼠标点击位置计算对应的地图行索引
            int row = (y - 70) / 40;
            // 根据鼠标点击位置计算对应的地图列索引
            int col = (x - 45) / 40;

            // 检查点击位置是否在有效地图范围内，并且该位置有图案（不为 -1）
            if (row >= 0 && row < 10 && col >= 0 && col < 16 && mapData[row][col] != -1) {
                // 如果还没有选中第一个图案
                if (selectedPos1 == QPair<int, int>(-1, -1)) {
                    // 将当前点击位置设为第一个选中图案的位置
                    selectedPos1 = {row, col};
                } else {
                    // 如果已经选中了第一个图案，将当前点击位置设为第二个选中图案的位置
                    selectedPos2 = {row, col};
                    // 用于存储两个选中图案之间的连线路径
                    QVector<QPair<int, int>> path;
                    // 检查两个选中的图案是否可以消除
                    if (canEliminate(selectedPos1, selectedPos2, path)) {
                        // 标记正在进行消除操作，避免用户快速点击导致信息丢失
                        isEliminating  = true;
                        // 保存连线路径
                        connectionPath = path;
                        // 触发重绘事件，绘制连线路径
                        update();
                        // 延迟 300 毫秒后执行消除图案的操作
                        QTimer::singleShot(300, this, [this]() {
                            // singleShot 是异步操作，避免在其他地方修改选中位置信息导致越界
                            // 消除选中的两个图案
                            eliminatePatterns(selectedPos1, selectedPos2);
                            // 清除连线路径
                            connectionPath.clear();
                            // 触发重绘事件，更新界面
                            update();
                            // 重置选中位置信息
                            selectedPos1 = {-1, -1};
                            selectedPos2 = {-1, -1};
                            // 重置提示位置信息
                            hintPos1 = {-1, -1};
                            hintPos2 = {-1, -1};
                            // 标记消除操作结束
                            isEliminating  = false;
                        });
                    } else {
                        // 如果两个图案不能消除，将第二个选中位置设为第一个选中位置
                        selectedPos1 = selectedPos2;
                        // 重置第二个选中位置
                        selectedPos2 = {-1, -1};
                        // 重置提示位置信息
                        hintPos1 = {-1, -1};
                        hintPos2 = {-1, -1};
                    }
                }
                // 触发重绘事件，更新界面显示选中状态
                update();
            }
        }
    }
    // 调用父类的鼠标点击事件处理函数，处理其他可能的事件
    QWidget::mousePressEvent(event);
}

bool basic_mode::canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2)
{
    if (pos1 == pos2) return false;
    int row1 = pos1.first;
    int col1 = pos1.second;
    int row2 = pos2.first;
    int col2 = pos2.second;
    if (mapData[row1][col1] != mapData[row2][col2]) return false;
    struct Node {
        int row;
        int col;
        int turns;
        int direction;
    };
    std::queue<Node> q;
    QVector<QVector<bool>> visited(mapData.size(), QVector<bool>(mapData[0].size(), false));
    QVector<QVector<int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    q.push({row1, col1, 0, -1});
    visited[row1][col1] = true;
    while (!q.empty()) {
        Node current = q.front();
        q.pop();
        if (current.row == row2 && current.col == col2 && current.turns <= 2) {
            return true;
        }
        for (int i = 0; i < 4; ++i) {
            int newRow = current.row + directions[i][0];
            int newCol = current.col + directions[i][1];
            int newTurns = current.turns;
            if (current.direction != -1 && current.direction != i) {
                newTurns++;
            }
            if (newTurns > 2) continue;
            while (newRow >= 0 && newRow < mapData.size() && newCol >= 0 && newCol < mapData[0].size()) {
                if (newRow == row2 && newCol == col2) {
                    if (newTurns <= 2) {
                        return true;
                    }
                    break;
                }
                if (mapData[newRow][newCol] != -1) {
                    break;
                }
                if (!visited[newRow][newCol]) {
                    visited[newRow][newCol] = true;
                    q.push({newRow, newCol, newTurns, i});
                }
                newRow += directions[i][0];
                newCol += directions[i][1];
            }
        }
    }
    return false;


    // 拓展地图版本
    // 如果选择的是同一块区块，不能消除
    // if (pos1 == pos2) return false;
    // // 由于extendmap拓展了一圈空白区域，所以xy的坐标需要+1处理
    // int extendedRow1 = pos1.first + 1;
    // int extendedCol1 = pos1.second + 1;
    // int extendedRow2 = pos2.first + 1;
    // int extendedCol2 = pos2.second + 1;

    // // 如果两个区块的图案不同，不能消除
    // if (extendedMapData[extendedRow1][extendedCol1] != extendedMapData[extendedRow2][extendedCol2]) return false; // 两块区块图像不同

    // // 定义一个结构体 Node，用于表示搜索过程中的节点
    // // 包含当前位置的行、列、转弯次数和移动方向
    // struct Node {
    //     int row;
    //     int col;
    //     int turns;
    //     int direction;
    // };
    // // 创建一个队列 q 用于存储待搜索的节点。
    // // 创建一个二维布尔数组 visited，用于记录每个位置是否已经被访问过，初始化为 false。
    // // 定义一个二维数组 directions，表示四个方向的偏移量，分别是向上 {-1, 0}、向下 {1, 0}、向左 {0, -1}、向右 {0, 1}。
    // std::queue<Node> q;
    // QVector<QVector<bool>> visited(extendedMapData.size(), QVector<bool>(extendedMapData[0].size(), false));
    // QVector<QVector<int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    // // 将起始位置（即第一个选中图案在扩展地图上的位置）封装为一个 Node 并加入队列 q，设置其转弯次数为 0，方向为 -1（表示初始没有方向）。将起始位置标记为已访问。
    // q.push({extendedRow1, extendedCol1, 0, -1});
    // visited[extendedRow1][extendedCol1] = true;

    // // 广度优先搜索：
    // //     进入 while 循环，只要队列不为空就继续搜索。
    // //     取出队列头部的节点作为当前节点 current。
    // //     检查当前节点是否为目标节点（即第二个选中图案在扩展地图上的位置），并且转弯次数是否不超过 2（因为规则是最多三条直线连通，即最多两次转弯）。如果满足条件，则返回 true，表示两个图案可以消除。
    // //     遍历四个方向，计算新位置的坐标 newRow 和 newCol。
    // //     根据当前节点的方向和新方向，更新转弯次数 newTurns。如果当前节点有方向且新方向与当前方向不同，则转弯次数加 1。
    // //     如果转弯次数超过 2，则跳过当前方向的搜索，继续下一个方向。
    // //     进入内层 while 循环，在当前方向上持续移动，直到超出地图边界、遇到非空白图案或者到达目标位置。
    // //     如果到达目标位置且转弯次数不超过 2，则返回 true。
    // //     如果遇到非空白图案，则停止在当前方向上的搜索，跳出内层 while 循环。
    // //     如果当前位置未被访问过，则将其标记为已访问，并将其封装为一个 Node 加入队列 q，继续后续的搜索。
    // while (!q.empty()) {
    //     // 取出队列头部的节点作为当前节点
    //     Node current = q.front();
    //     q.pop();

    //     // 如果当前节点是目标节点（第二个选中图案的位置），且转弯次数不超过 2
    //     if (current.row == extendedRow2 && current.col == extendedCol2 && current.turns <= 2) {
    //         // 说明两个图案可以消除，返回 true
    //         return true;
    //     }

    //     // 遍历四个方向
    //     for (int i = 0; i < 4; ++i) {
    //         // 计算新位置的行和列
    //         int newRow = current.row + directions[i][0];
    //         int newCol = current.col + directions[i][1];
    //         // 初始化新的转弯次数
    //         int newTurns = current.turns;
    //         // 如果当前节点有方向且新方向与当前方向不同，转弯次数加 1
    //         if (current.direction != -1 && current.direction != i) {
    //             newTurns++;
    //         }

    //         // 如果转弯次数超过 2，跳过当前方向的搜索
    //         if (newTurns > 2) continue;

    //         // 在内层 while 循环中，沿着当前方向持续移动
    //         while (newRow >= 0 && newRow < extendedMapData.size() && newCol >= 0 && newCol < extendedMapData[0].size()) {
    //             // 如果到达目标位置且转弯次数不超过 2
    //             if (newRow == extendedRow2 && newCol == extendedCol2) {
    //                 if (newTurns <= 2) {
    //                     // 说明两个图案可以消除，返回 true
    //                     return true;
    //                 }
    //                 // 否则跳出内层循环
    //                 break;
    //             }

    //             // 如果遇到非空白图案，停止在当前方向上的搜索
    //             if (extendedMapData[newRow][newCol] != -1) {
    //                 break;
    //             }

    //             // 如果当前位置未被访问过
    //             if (!visited[newRow][newCol]) {
    //                 // 标记为已访问
    //                 visited[newRow][newCol] = true;
    //                 // 将新位置封装为一个 Node 加入队列
    //                 q.push({newRow, newCol, newTurns, i});
    //             }

    //             // 继续沿着当前方向移动
    //             newRow += directions[i][0];
    //             newCol += directions[i][1];
    //         }
    //     }
    // }
    // // 如果搜索完所有可能路径都未找到符合条件的路径，返回 false
    // return false;
}

// 判断两个图案是否可以消除的函数（返回连线路径）
bool basic_mode::canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2, QVector<QPair<int, int>> &path)
{
    // 如果选择的是同一块区块，不能消除
    if (pos1 == pos2) return false;
    // 由于不扩展地图，不需要对坐标进行 +1 处理
    int row1 = pos1.first;
    int col1 = pos1.second;
    int row2 = pos2.first;
    int col2 = pos2.second;

    // 如果两个区块的图案不同，不能消除
    if (mapData[row1][col1] != mapData[row2][col2]) return false;

    // 定义一个结构体 Node，用于表示搜索过程中的节点
    // 包含当前位置的行、列、转弯次数、移动方向和到达该位置的路径
    struct Node {
        int row;
        int col;
        int turns;
        int direction;
        QVector<QPair<int, int>> path;
    };
    // 创建一个队列，用于存储待搜索的节点
    std::queue<Node> q;
    // 创建一个二维布尔数组，用于记录每个位置是否已经被访问过
    QVector<QVector<bool>> visited(mapData.size(), QVector<bool>(mapData[0].size(), false));
    // 定义四个方向的偏移量，分别是上、下、左、右
    QVector<QVector<int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    // 创建起始节点，将起始位置封装为一个 Node 并加入队列
    Node startNode = {row1, col1, 0, -1, {QPair<int, int>(row1, col1)}};
    q.push(startNode);
    // 标记起始位置为已访问
    visited[row1][col1] = true;

    // 广度优先搜索过程
    while (!q.empty()) {
        // 取出队列头部的节点作为当前节点
        Node current = q.front();
        q.pop();

        // 如果当前节点是目标节点（第二个选中图案的位置），且转弯次数不超过 2
        if (current.row == row2 && current.col == col2 && current.turns <= 2) {
            // 将当前节点的路径赋值给传入的 path 参数
            path = current.path;
            // 将目标位置添加到路径中
            path.append(QPair<int, int>(row2, col2));
            // 说明两个图案可以消除，返回 true
            return true;
        }

        // 遍历四个方向
        for (int i = 0; i < 4; ++i) {
            // 计算新位置的行和列
            int newRow = current.row + directions[i][0];
            int newCol = current.col + directions[i][1];
            // 初始化新的转弯次数
            int newTurns = current.turns;
            // 如果当前节点有方向且新方向与当前方向不同，转弯次数加 1
            if (current.direction != -1 && current.direction != i) {
                newTurns++;
            }

            // 如果转弯次数超过 2，跳过当前方向的搜索
            if (newTurns > 2) continue;

            // 在内层 while 循环中，沿着当前方向持续移动
            while (newRow >= 0 && newRow < mapData.size() && newCol >= 0 && newCol < mapData[0].size()) {
                // 如果到达目标位置且转弯次数不超过 2
                if (newRow == row2 && newCol == col2) {
                    if (newTurns <= 2) {
                        // 将当前节点的路径赋值给传入的 path 参数
                        path = current.path;
                        // 将目标位置添加到路径中
                        path.append(QPair<int, int>(newRow, newCol));
                        // 说明两个图案可以消除，返回 true
                        return true;
                    }
                    // 否则跳出内层循环
                    break;
                }

                // 如果遇到非空白图案，停止在当前方向上的搜索
                if (mapData[newRow][newCol] != -1) {
                    break;
                }

                // 如果当前位置未被访问过
                if (!visited[newRow][newCol]) {
                    // 标记为已访问
                    visited[newRow][newCol] = true;
                    // 创建新节点，继承当前节点的路径
                    Node newNode = {newRow, newCol, newTurns, i, current.path};
                    // 将新位置添加到新节点的路径中
                    newNode.path.append(QPair<int, int>(newRow, newCol));
                    // 将新节点加入队列
                    q.push(newNode);
                }

                // 继续沿着当前方向移动
                newRow += directions[i][0];
                newCol += directions[i][1];
            }
        }
    }

    // 如果搜索完所有可能路径都未找到符合条件的路径，返回 false
    return false;




    // 拓展地图版本
    // // 如果选择的是同一块区块，不能消除
    // if (pos1 == pos2) return false;
    // // 由于扩展地图增加了一圈空白区域，所以需要将坐标 +1 转换到扩展地图上
    // int extendedRow1 = pos1.first + 1;
    // int extendedCol1 = pos1.second + 1;
    // int extendedRow2 = pos2.first + 1;
    // int extendedCol2 = pos2.second + 1;

    // // 如果两个区块的图案不同，不能消除
    // if (extendedMapData[extendedRow1][extendedCol1] != extendedMapData[extendedRow2][extendedCol2]) return false;

    // // 定义一个结构体 Node，用于表示搜索过程中的节点
    // // 包含当前位置的行、列、转弯次数、移动方向和到达该位置的路径
    // struct Node {
    //     int row;
    //     int col;
    //     int turns;
    //     int direction;
    //     QVector<QPair<int, int>> path;
    // };
    // // 创建一个队列，用于存储待搜索的节点
    // std::queue<Node> q;
    // // 创建一个二维布尔数组，用于记录每个位置是否已经被访问过
    // QVector<QVector<bool>> visited(extendedMapData.size(), QVector<bool>(extendedMapData[0].size(), false));
    // // 定义四个方向的偏移量，分别是上、下、左、右
    // QVector<QVector<int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    // // 创建起始节点，将起始位置封装为一个 Node 并加入队列
    // Node startNode = {extendedRow1, extendedCol1, 0, -1, {QPair<int, int>(extendedRow1, extendedCol1)}};
    // q.push(startNode);
    // // 标记起始位置为已访问
    // visited[extendedRow1][extendedCol1] = true;

    // // 广度优先搜索过程
    // while (!q.empty()) {
    //     // 取出队列头部的节点作为当前节点
    //     Node current = q.front();
    //     q.pop();

    //     // 如果当前节点是目标节点（第二个选中图案的位置），且转弯次数不超过 2
    //     if (current.row == extendedRow2 && current.col == extendedCol2 && current.turns <= 2) {
    //         // 将当前节点的路径赋值给传入的 path 参数
    //         path = current.path;
    //         // 将目标位置添加到路径中
    //         path.append(QPair<int, int>(extendedRow2, extendedCol2));
    //         // 说明两个图案可以消除，返回 true
    //         return true;
    //     }

    //     // 遍历四个方向
    //     for (int i = 0; i < 4; ++i) {
    //         // 计算新位置的行和列
    //         int newRow = current.row + directions[i][0];
    //         int newCol = current.col + directions[i][1];
    //         // 初始化新的转弯次数
    //         int newTurns = current.turns;
    //         // 如果当前节点有方向且新方向与当前方向不同，转弯次数加 1
    //         if (current.direction != -1 && current.direction != i) {
    //             newTurns++;
    //         }

    //         // 如果转弯次数超过 2，跳过当前方向的搜索
    //         if (newTurns > 2) continue;

    //         // 在内层 while 循环中，沿着当前方向持续移动
    //         while (newRow >= 0 && newRow < extendedMapData.size() && newCol >= 0 && newCol < extendedMapData[0].size()) {
    //             // 如果到达目标位置且转弯次数不超过 2
    //             if (newRow == extendedRow2 && newCol == extendedCol2) {
    //                 if (newTurns <= 2) {
    //                     // 将当前节点的路径赋值给传入的 path 参数
    //                     path = current.path;
    //                     // 将目标位置添加到路径中
    //                     path.append(QPair<int, int>(newRow, newCol));
    //                     // 说明两个图案可以消除，返回 true
    //                     return true;
    //                 }
    //                 // 否则跳出内层循环
    //                 break;
    //             }

    //             // 如果遇到非空白图案，停止在当前方向上的搜索
    //             if (extendedMapData[newRow][newCol] != -1) {
    //                 break;
    //             }

    //             // 如果当前位置未被访问过
    //             if (!visited[newRow][newCol]) {
    //                 // 标记为已访问
    //                 visited[newRow][newCol] = true;
    //                 // 创建新节点，继承当前节点的路径
    //                 Node newNode = {newRow, newCol, newTurns, i, current.path};
    //                 // 将新位置添加到新节点的路径中
    //                 newNode.path.append(QPair<int, int>(newRow, newCol));
    //                 // 将新节点加入队列
    //                 q.push(newNode);
    //             }

    //             // 继续沿着当前方向移动
    //             newRow += directions[i][0];
    //             newCol += directions[i][1];
    //         }
    //     }
    // }

    // // 如果搜索完所有可能路径都未找到符合条件的路径，返回 false
    // return false;
}

// 消除两个指定位置图案的函数
void basic_mode::eliminatePatterns(const QPair<int, int> &pos1, const QPair<int, int> &pos2)
{
    // 检查第一个位置是否在有效地图范围内
    if (pos1.first >= 0 && pos1.first < mapData.size() && pos1.second >= 0 && pos1.second < mapData[0].size()) {
        // 将该位置的图案标记为 -1，表示已消除
        mapData[pos1.first][pos1.second] = -1;
    }
    // 检查第二个位置是否在有效地图范围内
    if (pos2.first >= 0 && pos2.first < mapData.size() && pos2.second >= 0 && pos2.second < mapData[0].size()) {
        // 将该位置的图案标记为 -1，表示已消除
        mapData[pos2.first][pos2.second] = -1;
    }
    // 消除图案后，增加积分
    score += 10;

    // 因为要取消边缘相消而被注释
    // // 检查第一个位置在扩展地图上的位置是否有效
    // if (pos1.first + 1 >= 0 && pos1.first + 1 < extendedMapData.size() && pos1.second + 1 >= 0 && pos1.second + 1 < extendedMapData[0].size()) {
    //     // 将扩展地图上该位置的图案标记为 -1，表示已消除
    //     extendedMapData[pos1.first + 1][pos1.second + 1] = -1;
    // }
    // // 检查第二个位置在扩展地图上的位置是否有效
    // if (pos2.first + 1 >= 0 && pos2.first + 1 < extendedMapData.size() && pos2.second + 1 >= 0 && pos2.second + 1 < extendedMapData[0].size()) {
    //     // 将扩展地图上该位置的图案标记为 -1，表示已消除
    //     extendedMapData[pos2.first + 1][pos2.second + 1] = -1;
    // }
}

// 扩展地图数据的函数
void basic_mode::extendMap()
{
    // 清空扩展地图数据
    extendedMapData.clear();
    // 调整扩展地图数据的大小为 12 行 18 列，初始值为 -1（表示空白）
    extendedMapData.resize(12, QVector<int>(18, -1));
    // 将原始地图数据复制到扩展地图的中间部分
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; ++j) {
            extendedMapData[i + 1][j + 1] = mapData[i][j];
        }
    }
}

// 提示按钮的槽函数
void basic_mode::on_BTN_TIP_clicked()
{
    // 用于存储所有可消除的图案对
    QVector<QPair<QPair<int, int>, QPair<int, int>>> eliminablePairs;
    // 遍历地图上的所有位置
    for (int i1 = 0; i1 < 10; ++i1) {
        for (int j1 = 0; j1 < 16; ++j1) {
            // 如果该位置有图案
            if (mapData[i1][j1] != -1) {
                // 再次遍历地图上的所有位置
                for (int i2 = 0; i2 < 10; ++i2) {
                    for (int j2 = 0; j2 < 16; ++j2) {
                        // 排除自身位置
                        if (i1 != i2 || j1 != j2) {
                            // 如果该位置有图案
                            if (mapData[i2][j2] != -1) {
                                // 创建两个位置的坐标对
                                QPair<int, int> pos1 = {i1, j1};
                                QPair<int, int> pos2 = {i2, j2};
                                // 检查这两个位置的图案是否可以消除
                                if (canEliminate(pos1, pos2)) {
                                    // 如果可以消除，将这对位置添加到可消除图案对列表中
                                    eliminablePairs.append({{pos1, pos2}});
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 如果存在可消除的图案对
    if (!eliminablePairs.isEmpty()) {
        // 随机选择一个可消除的图案对
        int randomIndex = QRandomGenerator::global()->bounded(eliminablePairs.size());
        // 记录提示的第一个图案位置
        hintPos1 = eliminablePairs[randomIndex].first;
        // 记录提示的第二个图案位置
        hintPos2 = eliminablePairs[randomIndex].second;
        // 触发重绘事件，更新界面显示提示
        update();
        // 3 秒后清除提示
        QTimer::singleShot(3000, this, &basic_mode::clearHint);
    } else {
        // 如果没有可消除的图案对，弹出提示框
        QMessageBox::information(this, "提示", "当前没有可以消除的图案对！请进行重排！");
    }
}


// 重排按钮的槽函数
void basic_mode::on_BTN_REARRANGE_clicked()
{
    // 用于存储所有有图案的位置
    QVector<QPair<int, int>> occupiedPositions;
    // 遍历地图上的所有位置
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; ++j) {
            // 如果该位置有图案
            if (mapData[i][j] != -1) {
                // 将该位置添加到有图案的位置列表中
                occupiedPositions.append({i, j});
            }
        }
    }

    // 重排次数（可修改）
    const int shuffleTimes = 50;
    for (int i = 0; i < shuffleTimes; ++i) {
        // 随机选择两个有图案的位置
        int index1 = QRandomGenerator::global()->bounded(occupiedPositions.size());
        int index2 = QRandomGenerator::global()->bounded(occupiedPositions.size());

        // 获取这两个位置的坐标对
        QPair<int, int> pos1 = occupiedPositions[index1];
        QPair<int, int> pos2 = occupiedPositions[index2];

        // 交换这两个位置的图案
        int temp = mapData[pos1.first][pos1.second];
        mapData[pos1.first][pos1.second] = mapData[pos2.first][pos2.second];
        mapData[pos2.first][pos2.second] = temp;

        // // 更新扩展地图上对应位置的图案，因为取消拓展地图而取消
        // extendedMapData[pos1.first + 1][pos1.second + 1] = mapData[pos1.first][pos1.second];
        // extendedMapData[pos2.first + 1][pos2.second + 1] = mapData[pos2.first][pos2.second];
    }

    // 触发重绘事件，更新界面显示重排后的地图
    update();
}

// 暂停按钮的槽函数
void basic_mode::on_BTN_PAUSE_clicked()
{
    // 切换游戏暂停状态
    gamePaused = !gamePaused;

    // 如果游戏暂停
    if (gamePaused) {
        // 停止计时器
        killTimer(timerId);
        // 禁用其他按钮
        setButtonInteractions(false);
        // 启用暂停按钮，并将文本改为 “继续游戏”
        ui->BTN_PAUSE->setEnabled(true);
        ui->BTN_PAUSE->setText("继续游戏");
    } else {
        // 重新启动计时器
        timerId = startTimer(1000);
        // 启用其他按钮
        setButtonInteractions(true);
        // 将暂停按钮文本改为 “暂停游戏”
        ui->BTN_PAUSE->setText("暂停游戏");
    }
}

// 清除提示的函数
void basic_mode::clearHint()
{
    // 重置提示位置信息
    hintPos1 = {-1, -1};
    hintPos2 = {-1, -1};
    // 触发重绘事件，更新界面清除提示显示
    update();
}

// 重置按钮可交互状态的函数
void basic_mode::setButtonInteractions(bool enabled)
{
    // 设置提示按钮的可交互状态
    ui->BTN_TIP->setEnabled(enabled);
    // 设置重排按钮的可交互状态
    ui->BTN_REARRANGE->setEnabled(enabled);
    // 设置暂停按钮的可交互状态
    ui->BTN_PAUSE->setEnabled(enabled);

    // 以下按钮可根据需要启用或禁用
    // ui->BTN_HELP->setEnabled(enabled);
    // ui->BTN_SETTING->setEnabled(enabled);
}
