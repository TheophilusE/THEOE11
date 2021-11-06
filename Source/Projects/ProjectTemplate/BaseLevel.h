#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Scene/Scene.h>

using namespace Urho3D;

class BaseLevel : public Object
{
    URHO3D_OBJECT(BaseLevel, Object);

public:
    BaseLevel(Context* context);

    virtual ~BaseLevel();

private:
    void SubscribeToBaseEvents();

    void HandleStart(StringHash eventType, VariantMap& eventData);

protected:
    /**
     * Initialize the level
     */
    virtual void Init(){};

    /**
     * Start scene updates
     */
    virtual void Run();

    /**
     * Pause scene updates
     */
    virtual void Pause();

    void SubscribeToEvents();

    /**
     * Handle FOV change for all cameras
     */
    void HandleFovChange(StringHash eventType, VariantMap& eventData);

    /**
     * Get rid of this level
     */
    virtual void Dispose();

    /**
     * Define rects for splitscreen mode
     */
    eastl::vector<IntRect> InitRects(int count);

    /**
     * Create viewports and cameras based on controller count
     */
    void InitViewports(eastl::vector<int> playerIndexes);

    void CreateSingleCamera(int index = 0, int totalCount = 1, int controllerIndex = 0);

    void ApplyPostProcessEffects();

    /**
     * Level scene
     */
    SharedPtr<Scene> scene_;

    /**
     * Data which was passed trough LevelManager
     */
    VariantMap data_;

    /**
     * All available viewports in the scene
     * mapped against specific controller
     */
    eastl::hash_map<int, SharedPtr<Viewport>> viewports_;

    /**
     * All available cameras in the scene
     * mapped against specific controller
     */
    eastl::hash_map<int, SharedPtr<Node>> cameras_;

    SharedPtr<Zone> defaultZone_;
};
