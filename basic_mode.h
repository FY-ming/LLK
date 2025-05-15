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

class basic_mode : public QWidget
{
    Q_OBJECT

public:
    basic_mode(QWidget *parent = nullptr);
    ~basic_mode();

private slots:
    void on_BTN_START_clicked();
    void on_BTN_PAUSE_clicked();
    void on_BTN_TIP_clicked();
    void on_BTN_REARRANGE_clicked();
    void clearHint();

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::basic_mode *ui;
    QVector<QVector<int>> mapData;
    QPixmap elementPixmap;
    QPixmap maskPixmap;
    QVector<QPixmap> elements;
    QPixmap backgroundPixmap;

    QPair<int, int> selectedPos1;
    QPair<int, int> selectedPos2;
    QPair<int, int> hintPos1;
    QPair<int, int> hintPos2;
    QVector<QPair<int, int>> connectionPath;

    int score;
    int gameTime;
    int timerId;
    bool gameOver;
    bool gamePaused;
    bool isEliminating = false;

    QVector<QVector<bool>> adjMatrix; // 邻接矩阵

    void generateMap();
    void extractElements();
    void shuffleMap();
    void buildAdjMatrix();
    bool canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2);
    bool canEliminate(const QPair<int, int> &pos1, const QPair<int, int> &pos2, QVector<QPair<int, int>> &path);
    void eliminatePatterns(const QPair<int, int> &pos1, const QPair<int, int> &pos2);

    void checkGameStatus();
    void showWinMessage();
    void showLoseMessage();

    void setButtonInteractions(bool enabled);
};
#endif // BASIC_MODE_H
