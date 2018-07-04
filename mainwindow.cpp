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

void MainWindow::setGUIColorScheme()
{
    QFile styleSheetFile(":/Files/style.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString style = QLatin1String(styleSheetFile.readAll());
    qApp->setStyleSheet(style);

    m_volumeLabel->setStyleSheet("color: white; padding: 6px;");
}

QString MainWindow::getMediaTitle(const QString &path)
{
    int counter = 0;
    int position = path.size() - 1;
    while (path[position] != '/') {
        --position;
        ++counter;
    }
    return path.mid(position + 1, counter);
}

void MainWindow::openFileFromDisk()
{
    QString path = QFileDialog::getOpenFileName(this, "Open a video file!",
                                                QDir::homePath(),
                                                "Video files (*.mp4 *.avi *.wmv, *.mkv)");
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

void MainWindow::searchVideo(const QString &keyword)
{
    QString youtubeURL = "https://www.googleapis.com/youtube/v3/search?part=snippet,id&order=viewCount&type=video&q=" +
                         keyword +
                         "&key=" +
                         apiKEY +
                         "&videoEmbeddable=true&maxResults=20";

    QUrl realURL(youtubeURL);
    QNetworkRequest networkRequest(realURL);
    m_networkYoutubeReply = m_networkYoutubeManager->get(networkRequest);
}

void MainWindow::processYoutubeReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QRegExp regex("\\\"videoId\\\": \\\"([^\\\"]*)", Qt::CaseInsensitive, QRegExp::RegExp2);
        QRegExp regexTitle("\\\"title\\\": \\\"([^\\\"]*)", Qt::CaseInsensitive, QRegExp::RegExp2);
        QString realMediaLink = "";
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
        QString jsonString = jsonDocument.toJson();

        if (regex.indexIn(responseData) == -1 || regexTitle.indexIn(responseData) == -1) {
            QMessageBox::critical(this, "Error!", "No such video found, try another keyword.");
        }
        else {
            QString result, MP4MediaLink;
            QString title;
            QUrl MP4MediaURL;
            QNetworkRequest networkRequest;
            m_isPlayingFromYoutube = false;

            int currentPosition = regex.indexIn(jsonString, 0);
            int currentPositionTitle = regexTitle.indexIn(jsonString, 0);

            m_requestsList.clear();
            while (currentPosition != -1) {
                currentPosition += regex.matchedLength();
                currentPositionTitle += regexTitle.matchedLength();

                result = regex.cap(1);
                realMediaLink = "https://www.youtube.com/watch?v=" + result;
                title = "[YouTube] " + regexTitle.cap(1);
                MP4MediaLink = "http://you-link.herokuapp.com/?url=" + realMediaLink;
                MP4MediaURL = QUrl(MP4MediaLink);

                currentPosition = regex.indexIn(jsonString, currentPosition);
                currentPositionTitle = regexTitle.indexIn(jsonString, currentPositionTitle);

                networkRequest = QNetworkRequest(MP4MediaURL);
                m_requestsList.push_back({networkRequest, title});
            }

            QPair <QNetworkRequest, QString> firstRequest = m_requestsList.takeFirst();
            m_currentMediaTitle = firstRequest.second;
            m_networkMP4Reply = m_networkMP4Manager->get(firstRequest.first);
        }
    }
    else {
        QMessageBox::critical(this, "Error!", QString(reply->errorString()));
    }
}

void MainWindow::processMP4Reply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QRegExp regex("\"url\": \"([^\"]*)", Qt::CaseInsensitive, QRegExp::RegExp2);
        QString realMP4Link = "";
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
        QString jsonString = jsonDocument.toJson();

        qDebug() << m_currentMediaTitle;
        if (regex.indexIn(jsonString) != -1) {
            realMP4Link = regex.cap(1);
            QUrl realMP4URL(realMP4Link);
            m_mediaPlayer->setMedia(realMP4URL);
            m_mediaPlayer->setVolume(50);
            m_mediaPlayer->play();
            qDebug() << realMP4Link;
            this->setWindowTitle(m_currentMediaTitle + " - zPlayer [PLAYING]");
            m_isPlayingFromYoutube = true;
        }
        else {
            if (!m_requestsList.empty()) {
                QPair <QNetworkRequest, QString> currentRequest = m_requestsList.takeFirst();
                m_currentMediaTitle = currentRequest.second;
                m_networkMP4Reply = m_networkMP4Manager->get(currentRequest.first);
            }
        }
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

void MainWindow::setMediaPlayer()
{
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    m_mediaPlayer->setAudioRole(QAudio::VideoRole);
}

void MainWindow::setWindowProperties()
{
    this->setWindowIcon(QIcon(":/Icons/playBlack.ico"));
}

void MainWindow::connectSignals()
{
    connect(m_openButton, SIGNAL(clicked(bool)), this, SLOT(openFileFromDisk()));
    connect(m_pauseButton, SIGNAL(clicked(bool)), this, SLOT(pauseVideo()));
    connect(m_playButton, SIGNAL(clicked(bool)), this, SLOT(playVideo()));
    connect(m_youtubeSearchButton, SIGNAL(clicked(bool)), this, SLOT(searchOnYouTube()));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), m_mediaPlayer, SLOT(setVolume(int)));
    connect(m_networkYoutubeManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processYoutubeReply(QNetworkReply*)));
    connect(m_networkMP4Manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processMP4Reply(QNetworkReply*)));
}

void MainWindow::fullscreenToggle()
{
    if (this->isFullScreen())
        this->showNormal();
    else
        this->showFullScreen();
}

void MainWindow::playingStateToggle()
{
    if (m_mediaPlayer->state() == QMediaPlayer::State::PlayingState) {
        m_mediaPlayer->pause();
        this->setWindowTitle(m_currentMediaTitle + " - zPlayer [PAUSED]");
    }
    else {
        m_mediaPlayer->play();
        this->setWindowTitle(m_currentMediaTitle + " - zPlayer [PLAYING]");
    }
}

void MainWindow::decreaseVolume()
{
    int volume = m_volumeSlider->value();
    m_volumeSlider->setValue(qMax(volume - 5, 0));
}

void MainWindow::increaseVolume()
{
    int volume = m_volumeSlider->value();
    m_volumeSlider->setValue(qMin(volume + 5, 100));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key::Key_F: fullscreenToggle(); break;
        case Qt::Key::Key_Space: playingStateToggle(); break;
        case Qt::Key::Key_Down: decreaseVolume(); break;
        case Qt::Key::Key_Up: increaseVolume(); break;
        case Qt::Key::Key_O: openFileFromDisk(); break;
        case Qt::Key::Key_Y: searchOnYouTube(); break;
        case Qt::Key::Key_Escape: this->showNormal(); break;
    }
}

void MainWindow::setButtonsHeight()
{
    m_openButton->setFixedHeight(m_pauseButton->height());
    m_youtubeSearchButton->setFixedHeight(m_pauseButton->height());
    m_playButton->setFixedHeight(m_pauseButton->height());
    m_pauseButton->setFixedHeight(m_openButton->height());
}

void MainWindow::cancelFocusOnWidgets()
{
    m_openButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_pauseButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_playButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_youtubeSearchButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_volumeSlider->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow {parent},
    ui {new Ui::MainWindow},
    m_vLayout {new QVBoxLayout(this)},
    m_hLayout {new QHBoxLayout(this)},
    m_openButton {new QPushButton("Open local file", this)},
    m_playButton {new QPushButton(QIcon(":/Icons/play.ico"), "", this)},
    m_pauseButton {new QPushButton(QIcon(":/Icons/pause.ico"), "", this)},
    m_youtubeSearchButton {new QPushButton("Search on YouTube!", this)},
    m_volumeLabel {new QLabel("Volume: ", this)},
    m_volumeSlider {new QSlider(Qt::Orientation::Horizontal, this)},
    m_mediaPlayer {new QMediaPlayer(this)},
    m_videoWidget {new QVideoWidget(this)},
    m_currentMediaTitle {""},
    m_networkYoutubeManager {new QNetworkAccessManager(this)},
    m_networkMP4Manager {new QNetworkAccessManager(this)},
    m_networkYoutubeReply {nullptr},
    m_networkMP4Reply {nullptr}
{
    ui->setupUi(this);

    setSliderProperties();
    setHorizontalLayout();
    setVerticalLayout();
    setWindowProperties();

    setGUIColorScheme();
    setButtonsHeight();
    cancelFocusOnWidgets();

    setMediaPlayer();
    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
}
