#pragma once

#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/UI/UIEvents.h>

using namespace Urho3D;

namespace AudioDefs
{
enum SOUND_EFFECTS : unsigned int;
enum MUSIC : unsigned int;
enum AMBIENT_SOUNDS : unsigned int;
} // namespace AudioDefs

class AudioManager : public Object
{
    URHO3D_OBJECT(AudioManager, Object);

public:
    AudioManager(Context* context);

    virtual ~AudioManager();

    /**
     * Register Sound Effects assigned to AudioManager
     */
    void RegisterAudio(AudioDefs::SOUND_EFFECTS effect, eastl::string file);

    /**
     * Register Music assigned to AudioManager
     */
    void RegisterAudio(AudioDefs::MUSIC music, eastl::string file);

    /**
     * Register Ambient Sounds to AudioManager
     */
    void RegisterAudio(AudioDefs::AMBIENT_SOUNDS ambient, eastl::string file);

    /**
     * When enabled, multiple music tracks can be played simultaneously
     * If disabled, previous music track will be stopped when new music
     * starts to play
     */
    void AllowMultipleMusicTracks(bool enabled);

    /**
     * When enabled, multiple ambient tracks can be played simultaneously
     * If disabled, previous ambient track will be stopped when new ambient sound
     * starts to play
     */
    void AllowMultipleAmbientTracks(bool enabled);

    /**
     * Add sound effect to specific node
     */
    SoundSource3D* AddEffectToNode(Node* node, unsigned int index);

    /**
     * Add music to specific node
     */
    SoundSource3D* AddMusicToNode(Node* node, unsigned int index);

protected:
    virtual void Init();

private:
    SoundSource3D* CreateNodeSound(Node* node, const eastl::string& filename, const eastl::string& type);

    /**
     * Register to all sound related events
     */
    void SubscribeToEvents();

    /**
     * Subscribe to console commands to allow sound playing via console commands
     */
    void SubscribeConsoleCommands();

    /**
     * Listen for any button clicks
     */
    void HandleButtonClick(StringHash eventType, VariantMap& eventData);

    /**
     * Handle E_PLAY_SOUND event
     */
    void HandlePlaySound(StringHash eventType, VariantMap& eventData);

    /**
     * Handle E_STOP_SOUND event
     */
    void HandleStopSound(StringHash eventType, VariantMap& eventData);

    /**
     * Handle E_STOP_ALL_SOUNDS event
     */
    void HandleStopAllSounds(StringHash eventType, VariantMap& eventData);

    /**
     * Handle "ConsolePlaySound" event
     */
    void HandleConsolePlaySound(StringHash eventType, VariantMap& eventData);

    /**
     * Play the actual sound with configured settings
     */
    void PlaySound(eastl::string filename, eastl::string type = "Effect", int index = -1);

    /**
     * All sound effects map
     */
    eastl::hash_map<unsigned int, eastl::string> soundEffects_;

    /**
     * All music tracks map
     */
    eastl::hash_map<unsigned int, eastl::string> music_;

    /**
     * All active music track nodes
     */
    eastl::hash_map<int, SharedPtr<Node>> musicNodes_;

    /**
     * All ambient sound tracks map
     */
    eastl::hash_map<unsigned int, eastl::string> ambientSounds_;

    /**
     * All active ambient sound nodes
     */
    eastl::hash_map<int, SharedPtr<Node>> ambientNodes_;

    /**
     * Allow multiple music tracks to play
     */
    bool multipleMusicTracks_{true};

    /**
     * Allow multiple ambient tracks to play
     */
    bool multipleAmbientTracks_{true};

    eastl::hash_map<StringHash, Timer> effectsTimer_;
};
