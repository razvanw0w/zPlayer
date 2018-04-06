#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSlider>
#include <QFileDialog>
#include <QDir>
#include <QUrl>
#include <QString>
#include <QtNetwork>
#include <QMessageBox>
#include <QInputDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setSliderProperties();
    void setHorizontalLayout();
    void setVerticalLayout();
    QString getMediaTitle(const QString& path);
    void searchVideo(const QString& keyword);

    void playMedia(QString path);

private slots:
    void openFileFromDisk();
    void playVideo();
    void pauseVideo();
    void searchOnYouTube();
    void processReply(QNetworkReply *reply);

private:
    bool isConnectedToInternet();

    const QString apiKEY = "nope";
    Ui::MainWindow *ui;
    QVBoxLayout *m_vLayout;
    QHBoxLayout *m_hLayout;

    QPushButton *m_openButton;
    QPushButton *m_playButton;
    QPushButton *m_pauseButton;
    QPushButton *m_youtubeSearchButton;

    QLabel *m_volumeLabel;
    QSlider *m_volumeSlider;

    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;
    QString m_currentMediaTitle;
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_networkReply;
};

#endif // MAINWINDOW_H
