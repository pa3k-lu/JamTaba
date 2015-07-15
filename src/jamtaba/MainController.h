#pragma once

#include <QApplication>
#include <memory>
#include <QMutex>
#include "audio/core/AudioPeak.h"

#include "geo/IpToLocationResolver.h"
#include "../ninjam/Server.h"
#include "../loginserver/LoginService.h"
#include "../audio/core/AudioDriver.h"
#include "../audio/core/plugins.h"
#include "../audio/vst/PluginFinder.h"

class MainFrame;

namespace Ninjam{
    class Service;
    //class Server;
}

namespace Audio {
    class AudioDriver;
    class AudioDriverListener;
    class AbstractMp3Streamer;
    class Plugin;
    class AudioMixer;
    class SamplesBuffer;
    class AudioNode;
    class PluginDescriptor;
    class LocalInputAudioNode;
}

namespace Midi{
    class MidiDriver;
    class MidiBuffer;
}

namespace Login {
    class LoginService;
    class LoginServiceParser;
    class LoginServiceListener;
}

namespace Vst {
    class VstHost;
    class PluginFinder;
}

class JamtabaFactory;

//+++++++++++++

namespace Controller {

class AudioListener;
class NinjamJamRoomController;



//++++++++++++++++++++++++++++
class MainController : public QApplication, public Audio::AudioDriverListener
{
    Q_OBJECT

    friend class Controller::AudioListener;
    friend class Controller::NinjamJamRoomController;

public:
    MainController(JamtabaFactory *factory, int& argc, char** argv);
    ~MainController();

    void start();
    void stop();

    virtual void process(Audio::SamplesBuffer& in, Audio::SamplesBuffer& out);

    bool addTrack(long trackID, Audio::AudioNode* trackNode);
    void removeTrack(long trackID);

    void playRoomStream(Login::RoomInfo roomInfo);
    bool isPlayingRoomStream() const;

    bool isPlayingInNinjamRoom() const;

    void stopRoomStream();//stop currentRoom stream
    inline long long getCurrentStreamingRoomID() const{return currentStreamingRoomID;}

    void enterInRoom(Login::RoomInfo room);

    Audio::AudioDriver* getAudioDriver() const;
    Midi::MidiDriver* getMidiDriver() const;
    Login::LoginService* getLoginService() const;


    inline Controller::NinjamJamRoomController* getNinjamController() const{return ninjamController;}

    QList<Audio::PluginDescriptor> getPluginsDescriptors();

    Audio::Plugin* addPlugin(int inputTrackIndex, const Audio::PluginDescriptor& descriptor);
    void removePlugin(int inputTrackIndex, Audio::Plugin* plugin);

    QStringList getBotNames() const;

    //tracks
    void setTrackMute(int trackID, bool muteStatus);
    bool trackIsMuted(int trackID) const;
    void setTrackSolo(int trackID, bool soloStatus);
    bool trackIsSoloed(int trackID) const;
    void setTrackLevel(int trackID, float level);
    void setTrackPan(int trackID, float pan);

    Audio::AudioPeak getRoomStreamPeak();
    Audio::AudioPeak getTrackPeak(int trackID);

    const Vst::PluginFinder& getPluginFinder() const{return pluginFinder;}
    void scanPlugins();
    void initializePluginsList(QStringList paths);

    Audio::AudioNode* getTrackNode(long ID);

    void updateInputTracksRange();//called when input range or method (audio or midi) are changed in preferences
    void setInputTrackToMono(int localChannelIndex, int inputIndexInAudioDevice);
    void setInputTrackToStereo(int localChannelIndex, int firstInputIndex);
    void setInputTrackToMIDI(int localChannelIndex, int midiDevice);
    void setInputTrackToNoInput(int localChannelIndex);

    inline bool isStarted() const{return started;}

    Geo::Location getLocation(QString ip) ;

    //static const int INPUT_TRACK_ID = 1;
    Audio::LocalInputAudioNode* getInputTrack(int localInputIndex);
    int addInputTrackNode(Audio::LocalInputAudioNode* inputTrackNode);
    QList<Audio::LocalInputAudioNode*> inputTracks;

    inline int getAudioDriverSampleRate() const{return audioDriver->getSampleRate();}

    static QByteArray newGUID();

signals:
    void enteredInRoom(Login::RoomInfo room);
    void exitedFromRoom(bool error);

    //input selection
    void inputSelectionChanged();

private:

    void doAudioProcess(Audio::SamplesBuffer& in, Audio::SamplesBuffer& out);
    Audio::Plugin* createPluginInstance(const Audio::PluginDescriptor &descriptor);

    Audio::AudioDriver* audioDriver;
    Midi::MidiDriver* midiDriver;//TODO use unique_ptr

    std::unique_ptr<Login::LoginService> loginService;

    Audio::AudioMixer* audioMixer;

    Audio::AbstractMp3Streamer* roomStreamer;
    long long currentStreamingRoomID;

    //ninjam
    Ninjam::Service* ninjamService;
    Controller::NinjamJamRoomController* ninjamController;

    QMap<long, Audio::AudioNode*> tracksNodes;
    mutable QMutex mutex;

    bool started;

    //VST
    Vst::VstHost* vstHost;
    QList<Audio::PluginDescriptor> pluginsDescriptors;
    Vst::PluginFinder pluginFinder;
    //+++++++++++++++++++++++++
    void configureStyleSheet();

    void tryConnectInNinjamServer(Login::RoomInfo ninjamRoom);

    Geo::IpToLocationResolver ipToLocationResolver;

    QByteArray currentGUID;

private slots:
    //Login server
    void on_disconnectedFromLoginServer();

    //VST
    void on_VSTPluginFounded(QString name, QString group, QString path);

    //ninjam
    void on_connectedInNinjamServer(Ninjam::Server server);
    void on_disconnectedFromNinjamServer(const Ninjam::Server& server);
    void on_errorInNinjamServer(QString error);
    void on_ninjamAudioAvailableToSend(QByteArray encodedAudio, quint8 channelIndex, bool isFirstPart, bool isLastPart);

    //audio driver
    void on_audioDriverSampleRateChanged(int newSampleRate);
    void on_audioDriverStopped();
};

}
