#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::setSliderProperties()
{
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(50);
}

void MainWindow::setHorizontalLayout()
{
    m_hLayout->addWidget(m_openButton);
    m_hLayout->addWidget(m_playButton);
    m_hLayout->addWidget(m_pauseButton);
    m_hLayout->addWidget(m_youtubeSearchButton);
    m_hLayout->addWidget(m_volumeLabel);
    m_hLayout->addWidget(m_volumeSlider);
}

void MainWindow::setVerticalLayout()
{
    m_vLayout->addWidget(m_videoWidget);
    m_vLayout->addLayout(m_hLayout);

    ui->centralWidget->setLayout(m_vLayout);
    m_videoWidget->show();
}

QString MainWindow::getMediaTitle(const QString &path)
{
    int cnt = 0;
    int pos = path.size() - 1;
    while (path[pos] != '/')
        --pos, ++cnt;
    return path.mid(pos + 1, cnt);
}

void MainWindow::searchVideo(const QString &keyword)
{
    QString youtubeURL = "https://www.googleapis.com/youtube/v3/search?part=snippet&type=video&q=" +
                         keyword +
                         "%203%27%203&key=" +
                         apiKEY +
                         "&videoEmbeddable=true&maxResults=1";

    QUrl realURL(youtubeURL);
    if (m_networkManager == nullptr) {
        m_networkManager = new QNetworkAccessManager(this);
        connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processReply(QNetworkReply*)));
    }

    QNetworkRequest networkRequest(realURL);
    m_networkReply = m_networkManager->get(networkRequest);
}

void MainWindow::openFileFromDisk()
{
    QString path = QFileDialog::getOpenFileName(this, "Open a video file!", QDir::homePath(), "Video files (*.mp4 *.avi *.wmv, *.mkv)");
    if (path != "") {
        m_currentMediaTitle = getMediaTitle(path);
        this->setWindowTitle(m_currentMediaTitle + " - zPlayer [PLAYING]");
        m_mediaPlayer->setMedia(QUrl::fromLocalFile(path));
        m_mediaPlayer->setVolume(50);
        m_mediaPlayer->play();
    }
}

void MainWindow::playVideo()
{
    if (m_mediaPlayer->state() == QMediaPlayer::State::PausedState) {
        m_mediaPlayer->play();
        this->setWindowTitle(m_currentMediaTitle + " - zPlayer [PLAYING]");
    }
}

void MainWindow::pauseVideo()
{
    if (m_mediaPlayer->state() != QMediaPlayer::State::PausedState) {
        m_mediaPlayer->pause();
        this->setWindowTitle(m_currentMediaTitle + " - zPlayer [PAUSED]");
    }
}

void MainWindow::searchOnYouTube()
{
    if (!isConnectedToInternet()) {
        QMessageBox::critical(this, "Error!", "You are not connected to the Internet!");
        return;
    }

    bool success;
    QString response = QInputDialog::getText(this, "Search on YouTube!",
                                             "Keyword:",
                                             QLineEdit::Normal,
                                             "",
                                             &success);
    if (success && !response.isEmpty()) {
        searchVideo(response);
    }
}

void MainWindow::processReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray message = reply->readAll();

        QJsonDocument jsonResponse = QJsonDocument::fromJson(message);
        QJsonObject results = jsonResponse.object();
        /// TO BE WRITTEN
    }
    else {
        QMessageBox::critical(this, "Error!", QString(reply->errorString()));
    }
}

bool MainWindow::isConnectedToInternet()
{
    QNetworkAccessManager accessManager;
    QNetworkRequest request(QUrl("http://google.ro"));

    QNetworkReply *reply = accessManager.get(request);
    QEventLoop loop;

    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return reply->bytesAvailable();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow {parent},
    ui {new Ui::MainWindow},
    m_vLayout {new QVBoxLayout(this)},
    m_hLayout {new QHBoxLayout(this)},
    m_openButton {new QPushButton("Open file", this)},
    m_playButton {new QPushButton(QIcon(":/Icons/play.ico"), "", this)},
    m_pauseButton {new QPushButton(QIcon(":/Icons/pause.ico"), "", this)},
    m_youtubeSearchButton {new QPushButton("Search on YouTube!", this)},
    m_volumeLabel {new QLabel("Volume: ", this)},
    m_volumeSlider {new QSlider(Qt::Orientation::Horizontal, this)},
    m_mediaPlayer {new QMediaPlayer(this)},
    m_videoWidget {new QVideoWidget(this)},
    m_currentMediaTitle {""},
    m_networkManager {nullptr},
    m_networkReply {nullptr}
{
    ui->setupUi(this);

    setSliderProperties();
    setHorizontalLayout();
    setVerticalLayout();
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    m_mediaPlayer->setAudioRole(QAudio::VideoRole);
    this->setWindowIcon(QIcon(":/Icons/play.ico"));

    connect(m_openButton, SIGNAL(clicked(bool)), this, SLOT(openFileFromDisk()));
    connect(m_pauseButton, SIGNAL(clicked(bool)), this, SLOT(pauseVideo()));
    connect(m_playButton, SIGNAL(clicked(bool)), this, SLOT(playVideo()));
    connect(m_youtubeSearchButton, SIGNAL(clicked(bool)), this, SLOT(searchOnYouTube()));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), m_mediaPlayer, SLOT(setVolume(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
