#include "basic_mode.h"
#include "ui_basic_mode.h"
#include <queue>

basic_mode::basic_mode(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::basic_mode)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    extractElements();
    backgroundPixmap.load(":/resource/fruit_b0g.bmp");
    score = 0;
    gameTime = 300;
    gameOver = true;
    selectedPos1 = {-1, -1};
    selectedPos2 = {-1, -1};
    hintPos1 = {-1, -1};
    hintPos2 = {-1, -1};
    setButtonInteractions(false);
    ui->BTN_START->setEnabled(true);
}

basic_mode::~basic_mode()
{
    delete ui;
}

void basic_mode::on_BTN_START_clicked()
{
    ui->BTN_START->setEnabled(false);
    setButtonInteractions(true);

    generateMap();
    buildAdjMatrix();
    update();

    gameTime = 300;
    timerId = startTimer(1000);
    gameOver = false;
    gamePaused = false;
}

void basic_mode::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId) {
        gameTime--;
        ui->CountDownBar->setValue(gameTime);
        if (gameTime <= 0) {
            killTimer(timerId);
            showLoseMessage();
            setButtonInteractions(false);
            ui->BTN_START->setEnabled(true);
        } else {
            checkGameStatus();
        }
    }
    QWidget::timerEvent(event);
}

void basic_mode::checkGameStatus()
{
    bool allEliminated = true;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (mapData[i][j] != -1) {
                allEliminated = false;
                break;
            }
        }
        if (!allEliminated) break;
    }

    if (allEliminated) {
        killTimer(timerId);
        showWinMessage();
        setButtonInteractions(false);
        ui->BTN_START->setEnabled(true);
    }
}

void basic_mode::showWinMessage()
{
    QMessageBox::information(this, "游戏胜利", "恭喜你，成功消除所有图案！");
    gameOver = true;
}

void basic_mode::showLoseMessage()
{
    QMessageBox::information(this, "游戏失败", "时间已到，未能消除所有图案！");
    gameOver = true;
}

void basic_mode::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (!backgroundPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), backgroundPixmap);
    }

    if (!mapData.isEmpty()) {
        int offsetX = 45;
        int offsetY = 70;
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 16; ++j) {
                if (i >= 0 && i < mapData.size() && j >= 0 && j < mapData[i].size()) {
                    int elementIndex = mapData[i][j];
                    if (elementIndex != -1 && elementIndex < elements.size()) {
                        painter.drawPixmap(j * 40 + offsetX, i * 40 + offsetY, elements[elementIndex]);
                        if ((i == selectedPos1.first && j == selectedPos1.second) ||
                            (i == selectedPos2.first && j == selectedPos2.second)) {
                            painter.setPen(QPen(Qt::blue, 3));
                            painter.drawRect(j * 40 + offsetX, i * 40 + offsetY, 40, 40);
                            painter.setPen(Qt::black);
                        }
                        if ((i == hintPos1.first && j == hintPos1.second) ||
                            (i == hintPos2.first && j == hintPos2.second)) {
                            painter.setPen(QPen(Qt::red, 3));
                            painter.drawRect(j * 40 + offsetX, i * 40 + offsetY, 40, 40);
                            painter.setPen(Qt::black);
                        }
                    }
                }
            }
        }
    }

    if (connectionPath.size() >= 2) {
        int offsetX = 45;
        int offsetY = 70;
        painter.setPen(QPen(Qt::blue, 3));
        for (int i = 0; i < connectionPath.size() - 1; ++i) {
            int row1 = connectionPath[i].first;
            int col1 = connectionPath[i].second;
            int row2 = connectionPath[i + 1].first;
            int col2 = connectionPath[i + 1].second;
            painter.drawLine(col1 * 40 + offsetX + 20, row1 * 40 + offsetY + 20,
                             col2 * 40 + offsetX + 20, row2 * 40 + offsetY + 20);
        }
    }
}

void basic_mode::generateMap()
{
    mapData.clear();
    mapData.resize(10, QVector<int>(16, -1));

    int elementIndex = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; j += 2) {
            mapData[i][j] = elementIndex;
            mapData[i][j + 1] = elementIndex;
            elementIndex = (elementIndex + 1) % elements.size();
        }
    }

    shuffleMap();
}

void basic_mode::shuffleMap()
{
    const int shuffleTimes = 100;
    for (int i = 0; i < shuffleTimes; ++i) {
        int row1 = QRandomGenerator::global()->bounded(10);
        int col1 = QRandomGenerator::global()->bounded(16);
        int row2 = QRandomGenerator::global()->bounded(10);
        int col2 = QRandomGenerator::global()->bounded(16);

        int temp = mapData[row1][col1];
        mapData[row1][col1] = mapData[row2][col2];
        mapData[row2][col2] = temp;
    }
}

void basic_mode::extractElements()
{
    elementPixmap.load(":/resource/fruit_element.bmp");
    maskPixmap.load(":/resource/fruit_mask.bmp");

    for (int i = 0; i < 20; ++i) {
        QPixmap element = elementPixmap.copy(0, i * 40, 40, 40);
        QPixmap mask = maskPixmap.copy(0, i * 40, 40, 40);
        element.setMask(mask.createMaskFromColor(Qt::black));
        elements.append(element);
    }
}

void basic_mode::buildAdjMatrix()
{
    int numNodes = 10 * 16;
    adjMatrix.clear();
    adjMatrix.resize(numNodes, QVector<bool>(numNodes, false));

    for (int i1 = 0; i1 < 10; ++i1) {
        for (int j1 = 0; j1 < 16; ++j1) {
            if (mapData[i1][j1] != -1) {
                for (int i2 = 0; i2 < 10; ++i2) {
                    for (int j2 = 0; j2 < 16; ++j2) {
                        if (i1 != i2 || j1 != j2) {
                            if (mapData[i2][j2] != -1) {
                                QPair<int, int> pos1 = {i1, j1};
                                QPair<int, int> pos2 = {i2, j2};
                                if (canEliminate(pos1, pos2)) {
                                    int index1 = i1 * 16 + j1;
                                    int index2 = i2 * 16 + j2;
                                    adjMatrix[index1][index2] = true;
                                    adjMatrix[index2][index1] = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
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
        QVector<QPair<int, int>> path;
    };

    std::queue<Node> q;
    QVector<QVector<bool>> visited(mapData.size(), QVector<bool>(mapData[0].size(), false));
    QVector<QVector<int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    q.push({row1, col1, 0, {pos1}});
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
            if (current.path.size() > 1) {
                QPair<int, int> prev = current.path[current.path.size() - 2];
                if (prev.first != newRow && prev.second != newCol) {
                    newTurns++;
                }
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
                    Node newNode = {newRow, newCol, newTurns, current.path};
                    newNode.path.emplace_back(newRow, newCol);
                    q.push(newNode);
                }

                newRow += directions[i][0];
                newCol += directions[i][1];
            }
        }
    }

    return false;
}

bool basic_mode::canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2, QVector<QPair<int, int>> &path)
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
        QVector<QPair<int, int>> path;
    };

    std::queue<Node> q;
    QVector<QVector<bool>> visited(mapData.size(), QVector<bool>(mapData[0].size(), false));
    QVector<QVector<int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    q.push({row1, col1, 0, {pos1}});
    visited[row1][col1] = true;

    while (!q.empty()) {
        Node current = q.front();
        q.pop();

        if (current.row == row2 && current.col == col2 && current.turns <= 2) {
            path = current.path;
            path.emplace_back(row2, col2);
            return true;
        }

        for (int i = 0; i < 4; ++i) {
            int newRow = current.row + directions[i][0];
            int newCol = current.col + directions[i][1];
            int newTurns = current.turns;
            if (current.path.size() > 1) {
                QPair<int, int> prev = current.path[current.path.size() - 2];
                if (prev.first != newRow && prev.second != newCol) {
                    newTurns++;
                }
            }

            if (newTurns > 2) continue;

            while (newRow >= 0 && newRow < mapData.size() && newCol >= 0 && newCol < mapData[0].size()) {
                if (newRow == row2 && newCol == col2) {
                    if (newTurns <= 2) {
                        path = current.path;
                        path.emplace_back(newRow, newCol);
                        return true;
                    }
                    break;
                }

                if (mapData[newRow][newCol] != -1) {
                    break;
                }

                if (!visited[newRow][newCol]) {
                    visited[newRow][newCol] = true;
                    Node newNode = {newRow, newCol, newTurns, current.path};
                    newNode.path.emplace_back(newRow, newCol);
                    q.push(newNode);
                }

                newRow += directions[i][0];
                newCol += directions[i][1];
            }
        }
    }

    return false;
}

void basic_mode::eliminatePatterns(const QPair<int, int> &pos1, const QPair<int, int> &pos2)
{
    if (pos1.first >= 0 && pos1.first < mapData.size() && pos1.second >= 0 && pos1.second < mapData[0].size()) {
        mapData[pos1.first][pos1.second] = -1;
    }
    if (pos2.first >= 0 && pos2.first < mapData.size() && pos2.second >= 0 && pos2.second < mapData[0].size()) {
        mapData[pos2.first][pos2.second] = -1;
    }
    score += 10;
    buildAdjMatrix();
}

void basic_mode::mousePressEvent(QMouseEvent *event)
{
    if (!gameOver && !gamePaused) {
        QPoint position = event->position().toPoint();
        if (!isEliminating) {
            int x = position.x();
            int y = position.y();
            int row = (y - 70) / 40;
            int col = (x - 45) / 40;

            if (row >= 0 && row < 10 && col >= 0 && col < 16 && mapData[row][col] != -1) {
                if (selectedPos1 == QPair<int, int>(-1, -1)) {
                    selectedPos1 = {row, col};
                } else {
                    selectedPos2 = {row, col};
                    QVector<QPair<int, int>> path;
                    if (canEliminate(selectedPos1, selectedPos2, path)) {
                        isEliminating = true;
                        connectionPath = path;
                        update();
                        QTimer::singleShot(300, this, [this]() {
                            eliminatePatterns(selectedPos1, selectedPos2);
                            connectionPath.clear();
                            update();
                            selectedPos1 = {-1, -1};
                            selectedPos2 = {-1, -1};
                            hintPos1 = {-1, -1};
                            hintPos2 = {-1, -1};
                            isEliminating = false;
                        });
                    } else {
                        selectedPos1 = selectedPos2;
                        selectedPos2 = {-1, -1};
                        hintPos1 = {-1, -1};
                        hintPos2 = {-1, -1};
                    }
                }
                update();
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void basic_mode::on_BTN_TIP_clicked()
{
    QVector<QPair<QPair<int, int>, QPair<int, int>>> eliminablePairs;
    for (int i1 = 0; i1 < 10; ++i1) {
        for (int j1 = 0; j1 < 16; ++j1) {
            if (mapData[i1][j1] != -1) {
                for (int i2 = 0; i2 < 10; ++i2) {
                    for (int j2 = 0; j2 < 16; ++j2) {
                        if (i1 != i2 || j1 != j2) {
                            if (mapData[i2][j2] != -1) {
                                QPair<int, int> pos1 = {i1, j1};
                                QPair<int, int> pos2 = {i2, j2};
                                int index1 = i1 * 16 + j1;
                                int index2 = i2 * 16 + j2;
                                if (adjMatrix[index1][index2]) {
                                    eliminablePairs.emplace_back(pos1, pos2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!eliminablePairs.isEmpty()) {
        int randomIndex = QRandomGenerator::global()->bounded(eliminablePairs.size());
        hintPos1 = eliminablePairs[randomIndex].first;
        hintPos2 = eliminablePairs[randomIndex].second;
        update();
        QTimer::singleShot(3000, this, &basic_mode::clearHint);
    } else {
        QMessageBox::information(this, "提示", "当前没有可以消除的图案对！请进行重排！");
    }
}

void basic_mode::on_BTN_REARRANGE_clicked()
{
    QVector<QPair<int, int>> occupiedPositions;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (mapData[i][j] != -1) {
                occupiedPositions.emplace_back(i, j);
            }
        }
    }

    const int shuffleTimes = 50;
    for (int i = 0; i < shuffleTimes; ++i) {
        int index1 = QRandomGenerator::global()->bounded(occupiedPositions.size());
        int index2 = QRandomGenerator::global()->bounded(occupiedPositions.size());

        QPair<int, int> pos1 = occupiedPositions[index1];
        QPair<int, int> pos2 = occupiedPositions[index2];

        int temp = mapData[pos1.first][pos1.second];
        mapData[pos1.first][pos1.second] = mapData[pos2.first][pos2.second];
        mapData[pos2.first][pos2.second] = temp;
    }

    buildAdjMatrix();
    update();
}

void basic_mode::on_BTN_PAUSE_clicked()
{
    gamePaused = !gamePaused;

    if (gamePaused) {
        killTimer(timerId);
        setButtonInteractions(false);
        ui->BTN_PAUSE->setEnabled(true);
        ui->BTN_PAUSE->setText("继续游戏");
    } else {
        timerId = startTimer(1000);
        setButtonInteractions(true);
        ui->BTN_PAUSE->setText("暂停游戏");
    }
}

void basic_mode::clearHint()
{
    hintPos1 = {-1, -1};
    hintPos2 = {-1, -1};
    update();
}

void basic_mode::setButtonInteractions(bool enabled)
{
    ui->BTN_TIP->setEnabled(enabled);
    ui->BTN_REARRANGE->setEnabled(enabled);
    ui->BTN_PAUSE->setEnabled(enabled);
}
