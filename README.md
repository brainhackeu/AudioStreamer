Autodarts Remote Audio Streamer 🎯🔊
🇩🇪 DEUTSCH
📖 Die Geschichte (Hintergrund)
Wer Autodarts nutzt, kennt das Problem: Der PC am Dartboard (oft ein kleiner Kiosk-PC oder Raspberry Pi) ist perfekt für die Bildverarbeitung der Kameras, steht aber meistens direkt an der Scheibe – dort, wo man keine klobigen Lautsprecher haben möchte. Der "Caller"-Sound (die Ansage der Punkte) sollte aber eigentlich aus der Soundanlage am anderen Ende des Raums oder am PC des Schreibers kommen.
Ich habe nach einer einfachen Lösung gesucht, um den Systemsound ohne Verzögerung und ohne kilometerlange Kabel quer durch den Raum zu schicken. Da vorhandene Lösungen oft zu komplex, instabil oder nur für Windows-zu-Windows gedacht waren, habe ich dieses Tool geschrieben. Es "beamt" den Sound einfach per Netzwerk an einen beliebigen anderen Rechner.

💡 Wie es funktioniert
Das Programm fungiert als "Audio-Brücke":

Es scannt dein System nach Monitor-Outputs (das Signal, das normalerweise aus deinen Boxen kommt).
Es greift diesen digitalen Datenstrom mit parec (PulseAudio/PipeWire) ab.
Dieser Strom wird in Echtzeit an ffmpeg übergeben, dort in einen MPEG-TS Container verpackt und als UDP-Stream ins Netzwerk geschickt.
Jedes Gerät im selben Netzwerk (Windows, Linux, Mac) can diesen Stream mit einem Player wie VLC oder ffplay empfangen und abspielen.
🛠 Technische Details (Full Stack)
System-Voraussetzungen (Sender/Ubuntu):
OS: Ubuntu 22.04 / 24.04+ (x86_64 oder ARM/Raspberry Pi)
Audio-Server: PipeWire oder PulseAudio
Tools: ffmpeg, pulseaudio-utils (für parec)
Framework: Qt6 Widgets
Installation & Kompilierung:
# 1. Abhängigkeiten installieren
sudo apt update
sudo apt install build-essential qt6-base-dev ffmpeg pipewire-bin pulseaudio-utils git -y

# 2. Projekt klonen
git clone https://github.com/brainhackeu/AudioStreamer.git
cd AudioStreamer

# 3. Kompilieren
qmake6 AutodartsAudio.pro
make

Konfiguration (Schritt für Schritt):
GUI Modus: ./AutodartsAudio starten, Ziel-IP eingeben, .monitor Gerät wählen und Start drücken.
CLI Modus: ./AutodartsAudio --cli --ip [ZIEL-IP] --dev [INDEX]
Geräte Liste: ./AutodartsAudio --list (zeigt alle verfügbaren Audio-Indizes an).

Ziel-IP: IP des Empfänger-PCs eingeben.
Audiogerät: Wähle den Eintrag, der auf .monitor endet.
Qualität: 48000 Hz und 320k Bitrate empfohlen.
Auto-Start: Setze den Haken für automatischen Stream-Start beim nächsten Mal.

🇺🇸 ENGLISH
📖 The Backstory
When using Autodarts, the PC at the board (often a mini-PC or Raspberry Pi) is great for camera processing but is usually located right at the dartboard—not exactly where you want your speakers. The "Caller" sound (the score announcements) should ideally come from a sound system across the room or the computer where the scorer is sitting.
I was looking for a simple way to transmit system audio with zero latency and without running long cables across the floor. Since existing solutions were often too complex or unstable, I created this tool. It simply "beams" the audio over the network to any other computer, making it perfect for remote dart setups.

💡 How it Works
The application acts as an "Audio Bridge":

It scans your system for Monitor Outputs (the signal that usually goes to your speakers).
It captures this digital stream using parec (PulseAudio/PipeWire).
The stream is passed to ffmpeg in real-time, wrapped into an MPEG-TS container, and sent as a UDP stream over the network.
Any device on the same network (Windows, Linux, Mac) can receive and play this stream using VLC or ffplay.
🛠 Technical Details (Full Stack)
Requirements (Sender/Ubuntu):
OS: Ubuntu 22.04 / 24.04+ (x86_64 or ARM/Raspberry Pi)
Audio Server: PipeWire or PulseAudio
Tools: ffmpeg, pulseaudio-utils (for parec)
Framework: Qt6 Widgets
Installation & Compilation:
# 1. Install dependencies
sudo apt update
sudo apt install build-essential qt6-base-dev ffmpeg pipewire-bin pulseaudio-utils git -y

# 2. Clone project
git clone https://github.com/brainhackeu/AudioStreamer.git
cd AudioStreamer

# 3. Compile
qmake6 AutodartsAudio.pro
make

Configuration (Step-by-Step):
GUI Mode: Launch ./AutodartsAudio, enter IP, select .monitor device.
CLI Mode: ./AutodartsAudio --cli --ip [TARGET-IP] --dev [INDEX]
List Devices: ./AutodartsAudio --list

Target IP: Enter the IP of the receiver PC.
Audio Device: Select the entry ending in .monitor.
Quality: 48000 Hz and 320k bitrate are recommended for maximum clarity.
Auto-Start: Enable the checkbox to start streaming automatically on next launch.

Receiver Command (Client):
For the lowest latency, use ffplay on the target machine:
ffplay -nodisp -fflags nobuffer -flags low_delay udp://0.0.0.0:4010

.gitignore (Kopieren & Speichern als .gitignore)
# Build artifacts
Makefile*
*.o
*.moc
moc_*.cpp
*.qrc.cpp
ui_*.h
.qmake.stash
AutodartsAudio

# Local config
*.conf
