
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "STAR.h"

#include "Locomotion/Character/BaseCharacter.h"
#include "Locomotion/Character/CharacterController.h"
#include "Locomotion/Camera/PlayerCameraManager.h"

#include <Urho3D/DebugNew.h>


void STAR::Setup()
{
    App::Setup();

    // Engine is not initialized yet. Set up all the parameters now.
    engineParameters_[EP_WINDOW_TITLE] = "Star Game";
    engineParameters_[EP_VSYNC] = true;
    engineParameters_[EP_WINDOW_MAXIMIZE] = true;
    engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_WINDOW_HEIGHT] = 720;
    engineParameters_[EP_WINDOW_WIDTH] = 1280;
    // Resource prefix path is a list of semicolon-separated paths which will be checked for containing resource
    // directories. They are relative to application executable file.
    engineParameters_[EP_RESOURCE_PATHS] = "C:/THEO/Projects/STAR24/";
    engineParameters_[EP_LOG_NAME] = "StarLog.log";
}

void STAR::RegisterObjects(Context* context)
{
    // Register factory and attributes for the Character component so it can be created via CreateComponent, and
    // loaded / saved
    Character::RegisterObject(context);
    CharacterController::RegisterObject(context);
    PlayerCameraManager::RegisterObject(context);
}

void STAR::Start()
{
    // At this point engine is initialized, but first frame was not rendered yet. Further setup should be done here. To
    // make sample a little bit user friendly show mouse cursor here.
    App::Start();

    auto* cache = GetSubsystem<ResourceCache>();
    cache->AddResourceDir("C:\\THEO\\Projects\\STAR24\\Resources");
    cache->AddResourceDir("C:\\THEO\\Projects\\STAR24\\Cache");

    GetSubsystem<Input>()->SetMouseVisible(true);

    // Create static scene content
    CreateScene();

    // Create the controllable character
    CreateCharacter();

    // Create the UI content
    CreateInstructions();

    // Subscribe to necessary events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    InitMouseMode(MM_RELATIVE);
}

void STAR::Stop()
{
    // This step is executed when application is closing. No more frames will be rendered after this method is invoked.
    App::Stop();
}

void STAR::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    cameraNode_ = new Node(context_);
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

    // load scene
    XMLFile* xmlLevel = cache->GetResource<XMLFile>("Scenes/Benchmark.xml");
    scene_->LoadXML(xmlLevel->GetRoot());

    // Add scene debug renderer
    scene_->CreateComponent<DebugRenderer>();
}

void STAR::CreateCharacter()
{
    auto* cache = GetSubsystem<ResourceCache>();

    Node* objectNode = scene_->CreateChild("StarPlayer");
    objectNode->SetPosition(Vector3(0.0f, 1.0f, 0.0f));

    // spin node
    Node* adjustNode = objectNode->CreateChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));

    // Create the rendering component + animation controller
    auto* modelNode = adjustNode->CreateChild("Model");
    modelNode->SetRotation(Quaternion(90, Vector3(1, 0, 0)));
    modelNode->SetScale(Vector3(0.01f, 0.01f, 0.01f));
    auto* object = modelNode->CreateComponent<AnimatedModel>();
    object->SetModel(cache->GetResource<Model>("Models/Locomotion/ALS_N_Pose.mdl"));
    //object->SetModel(cache->GetResource<Model>("Models/Locomotion/Manneqin/Model.mdl"));
    //object->SetMaterial(cache->GetResource<Material>("Models/Locomotion/Manneqin/Materials/M_Male_Body.xml"));
    object->SetCastShadows(false);
    adjustNode->CreateComponent<AnimationController>();

    // Set the head bone for manual control
    object->GetSkeleton().GetBone("head")->animated_ = true;

    // Create rigidbody, and set non-zero mass so that the body becomes dynamic
    auto* body = objectNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(1);
    body->SetMass(1.0f);

    // Set zero angular factor so that physics doesn't turn the character on its own.
    // Instead we will control the character yaw manually
    body->SetAngularFactor(Vector3::ZERO);

    // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
    body->SetCollisionEventMode(COLLISION_ALWAYS);

    // Set a capsule shape for collision
    auto* shape = objectNode->CreateComponent<CollisionShape>();
    shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));

    // Create the character logic component, which takes care of steering the rigidbody
    // Remember it so that we can set the controls. Use a ea::weak_ptr because the scene hierarchy already owns it
    // and keeps it alive as long as it's not removed from the hierarchy
    character_ = objectNode->CreateComponent<Character>();
    objectNode->CreateComponent<CharacterController>();
    auto* cameraManager = objectNode->CreateComponent<PlayerCameraManager>();
    cameraManager->SetUp(scene_, cameraNode_);
    character_->scene_ = scene_;
    // Finish Character Setup
    //character_->Setup();
}

void STAR::CreateInstructions()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    auto* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText(
        "Use WASD keys and mouse to move\n"
        "Space to jump, F to toggle 1st/3rd person\n"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/NotoSans-Regular.ttf"), 12);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 2.5);
}


void STAR::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(STAR, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(STAR, HandlePostUpdate));

    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(STAR, HandlePostRenderUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in
    // this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}


void STAR::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    auto* input = GetSubsystem<Input>();

     // Toggle debug geometry with space
    if (input->GetKeyPress(KEY_M))
    {
        drawDebug_ = !drawDebug_;
        URHO3D_LOGINFO("Draw Debug: " + fmt::format("{}", drawDebug_));
    }

    if (input->GetKeyPress(KEY_ESCAPE))
    {
        if (useMouseMode_ == MouseMode::MM_ABSOLUTE)
        {
            InitMouseMode(MouseMode::MM_FREE);
            URHO3D_LOGINFO("Mouse Mode: Free");
        }
        else
        {
            InitMouseMode(MouseMode::MM_ABSOLUTE);
            input->SetMouseVisible(false);
            URHO3D_LOGINFO("Mouse Mode: Captured");
        }
    }
}

void STAR::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!character_)
        return;
}

void STAR::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    if (drawDebug_)
    {
        auto* physWorld = scene_->GetComponent<PhysicsWorld>();
        auto* dbgRenderer = scene_->GetComponent<DebugRenderer>();

        if (dbgRenderer)
        {
            if (physWorld)
            {
                physWorld->DrawDebugGeometry(dbgRenderer, true);
            }

            if (character_)
            {
                character_.Get()->DrawDebug();
            }
        }
    }
}
