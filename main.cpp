#include <QApplication>
#include <QCommandLineParser>
#include <QProcess>
#include <QSettings>
#include <iostream>
#include <QTimer>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QIcon>
#include <QIntValidator>

// --- GLOBALE HILFSFUNKTIONEN ---

// Diese Funktion wird von beiden Modi genutzt, um Hardware zu finden
QStringList getAudioDevices() {
    QProcess p;
    p.start("bash", QStringList() << "-c" << "pactl list short sources | awk '{print $2}'");
    p.waitForFinished();
    return QString(p.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
}

// Dein exakter Streaming-Befehl
QString buildStreamCmd(QString dev, QString sr, QString br, QString ip, QString port) {
    return QString("parec -d %1 --rate=%2 --format=s16le --channels=2 | "
                   "ffmpeg -f s16le -ar %2 -ac 2 -i - -acodec mp2 -ab %3 -f mpegts udp://%4:%5?pkt_size=1316")
           .arg(dev, sr, br, ip, port);
}

// --- DIE VOLLSTÄNDIGE GUI KLASSE ---

class DartsAudio : public QWidget {
    Q_OBJECT

public:
    DartsAudio(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
        setWindowTitle("Autodarts Audio Pro");

        loadSettings();
        auto *layout = new QVBoxLayout(this);

        layout->addWidget(new QLabel("Ziel-IP (Windows PC):"));
        ipEdit = new QLineEdit(targetIp);
        layout->addWidget(ipEdit);

        layout->addWidget(new QLabel("Port:"));
        portEdit = new QLineEdit(targetPort);
        portEdit->setValidator(new QIntValidator(1, 65535, this));
        layout->addWidget(portEdit);

        layout->addWidget(new QLabel("Audiogerät (Wähle .monitor):"));
        deviceBox = new QComboBox();
        refreshDevices();
        layout->addWidget(deviceBox);

        layout->addWidget(new QLabel("Sampling Rate (Hz):"));
        sampleBox = new QComboBox();
        sampleBox->addItems({"44100", "48000"});
        sampleBox->setCurrentText(targetSample);
        layout->addWidget(sampleBox);

        layout->addWidget(new QLabel("Bitrate (kbps):"));
        bitrateBox = new QComboBox();
        bitrateBox->addItems({"128k", "192k", "256k", "320k"});
        bitrateBox->setCurrentText(targetBitrate);
        layout->addWidget(bitrateBox);

        autoStartCheck = new QCheckBox("Beim Programmstart sofort senden");
        autoStartCheck->setChecked(autoStartEnabled);
        layout->addWidget(autoStartCheck);

        toggleBtn = new QPushButton(" AUDIO STARTEN");
        toggleBtn->setCheckable(true);
        toggleBtn->setMinimumHeight(60);
        layout->addWidget(toggleBtn);

        statusLabel = new QLabel("Bereit");
        statusLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(statusLabel);

        connect(toggleBtn, &QPushButton::toggled, this, &DartsAudio::handleToggle);
        setFixedSize(sizeHint());

        if (autoStartEnabled) {
            statusLabel->setText("Auto-Start aktiv...");
            QTimer::singleShot(1500, this, [this]() {
                if(autoStartCheck->isChecked()) toggleBtn->setChecked(true);
            });
        }
    }

    ~DartsAudio() { stopStream(); }

public slots:
    void handleToggle(bool checked) {
        if (checked) startStream(); else stopStream();
    }

private:
    QLineEdit *ipEdit, *portEdit;
    QComboBox *deviceBox, *sampleBox, *bitrateBox;
    QCheckBox *autoStartCheck;
    QPushButton *toggleBtn;
    QLabel *statusLabel;
    QProcess *streamProcess = nullptr;
    QString targetIp, targetPort, targetSample, targetBitrate, targetDevice;
    bool autoStartEnabled;

    void refreshDevices() {
        QStringList devices = getAudioDevices();
        deviceBox->clear();
        deviceBox->addItems(devices);
        int index = deviceBox->findText(targetDevice);
        if (index != -1) deviceBox->setCurrentIndex(index);
    }

    void loadSettings() {
        QSettings s("Autodarts", "AudioStream");
        targetIp = s.value("ip", "192.168.100.222").toString();
        targetPort = s.value("port", "4010").toString();
        targetSample = s.value("sample", "48000").toString();
        targetBitrate = s.value("bitrate", "320k").toString();
        targetDevice = s.value("device", "").toString();
        autoStartEnabled = s.value("autostart", false).toBool();
    }

    void saveSettings() {
        QSettings s("Autodarts", "AudioStream");
        s.setValue("ip", ipEdit->text());
        s.setValue("port", portEdit->text());
        s.setValue("sample", sampleBox->currentText());
        s.setValue("bitrate", bitrateBox->currentText());
        s.setValue("device", deviceBox->currentText());
        s.setValue("autostart", autoStartCheck->isChecked());
    }

    void startStream() {
        saveSettings();
        QString cmd = buildStreamCmd(deviceBox->currentText(), sampleBox->currentText(), 
                                     bitrateBox->currentText(), ipEdit->text(), portEdit->text());

        streamProcess = new QProcess(this);
        streamProcess->start("bash", QStringList() << "-c" << cmd);

        toggleBtn->setText(" STOP (SENDET)");
        toggleBtn->setStyleSheet("background-color: #cc0000; color: white; font-weight: bold;");
        statusLabel->setText("Streaming aktiv...");
    }

    void stopStream() {
        if (streamProcess) {
            streamProcess->terminate();
            if(!streamProcess->waitForFinished(500)) streamProcess->kill();
            streamProcess->deleteLater();
            streamProcess = nullptr;
        }
        QProcess::execute("pkill", QStringList() << "-9" << "ffmpeg");
        QProcess::execute("pkill", QStringList() << "-9" << "parec");
        toggleBtn->setText(" AUDIO STARTEN");
        toggleBtn->setStyleSheet("");
        statusLabel->setText("Status: Gestoppt");
        saveSettings();
    }
};

// --- DIE MAIN FUNKTION MIT KOPF (HEADLESS-FIX) ---

int main(int argc, char *argv[]) {
    // 1. Vorab-Check für Headless-Betrieb
    bool headless = false;
    for (int i = 0; i < argc; ++i) {
        QString arg = argv[i];
        if (arg == "--cli" || arg == "-c" || arg == "--list" || arg == "-l") {
            headless = true; break;
        }
    }
    if (headless) qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication a(argc, argv);
    a.setApplicationName("AutodartsAudioPro");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Autodarts Audio Streamer Pro");
    parser.addHelpOption();

    QCommandLineOption cliOption(QStringList() << "c" << "cli", "Startet ohne Fenster.");
    QCommandLineOption listOption(QStringList() << "l" << "list", "Zeigt verfuegbare Audio-Geraete.");
    QCommandLineOption ipOption("ip", "Ziel-IP Adresse.", "ip");
    QCommandLineOption portOption("port", "Port (Standard 4010).", "port", "4010");
    QCommandLineOption devOption("dev", "Geraet Name oder Index.", "dev");

    parser.addOption(cliOption);
    parser.addOption(listOption);
    parser.addOption(ipOption);
    parser.addOption(portOption);
    parser.addOption(devOption);
    parser.process(a);

    // LOGIK: LISTE
    if (parser.isSet(listOption)) {
        std::cout << "Geraete-Liste:" << std::endl;
        QStringList devs = getAudioDevices();
        for (int i = 0; i < devs.size(); ++i) std::cout << i << ": " << devs[i].toStdString() << std::endl;
        return 0;
    }

    // LOGIK: CLI START
    if (parser.isSet(cliOption)) {
        QString ip = parser.value(ipOption);
        QString dev = parser.value(devOption);
        QString port = parser.value(portOption);
        
        QStringList devs = getAudioDevices();
        bool isNum; int idx = dev.toInt(&isNum);
        if (isNum && idx >= 0 && idx < devs.size()) dev = devs[idx];

        if (ip.isEmpty() || dev.isEmpty()) {
            std::cout << "Fehler: --ip und --dev (oder Index) fehlen!" << std::endl;
            return -1;
        }

        std::cout << "CLI Stream: " << dev.toStdString() << " -> " << ip.toStdString() << ":" << port.toStdString() << std::endl;
        QProcess *proc = new QProcess();
        // Hier nutzen wir dieselbe Funktion wie die GUI
        proc->start("bash", QStringList() << "-c" << buildStreamCmd(dev, "48000", "320k", ip, port));
        return a.exec();
    }

    // LOGIK: GUI (Standard)
    DartsAudio w;
    w.show();
    return a.exec();
}

#include "main.moc"
