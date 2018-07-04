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
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QString>
#include <QtNetwork>
#include <QMessageBox>
#include <QInputDialog>
#include <QHash>
#include <QDebug>
#include <QKeyEvent>
#include <QList>
#include <QPair>

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
    void setGUIColorScheme();

    QString getMediaTitle(const QString& path);

    void searchVideo(const QString& keyword);
    void playMedia(QString path);
    void setMediaPlayer();
    void setWindowProperties();
    void setButtonsHeight();
    void connectSignals();
    void fullscreenToggle();
    void playingStateToggle();
    void cancelFocusOnWidgets();
    void decreaseVolume();
    void increaseVolume();

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private slots:
    void openFileFromDisk();
    void playVideo();
    void pauseVideo();
    void searchOnYouTube();
    void processYoutubeReply(QNetworkReply *reply);
    void processMP4Reply(QNetworkReply *reply);

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

    QNetworkAccessManager *m_networkYoutubeManager;
    QNetworkAccessManager *m_networkMP4Manager;
    QNetworkReply *m_networkYoutubeReply;
    QNetworkReply *m_networkMP4Reply;
    bool m_isPlayingFromYoutube;

    QList <QPair <QNetworkRequest, QString>> m_requestsList;
};

#endif // MAINWINDOW_H
