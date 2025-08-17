monoArray<void**>* (*get_PlayerList)();
void* (*GetFromActorNumber)(int actorNumber);
void* (*get_LocalPlayer)();

void* (*get_transform)(void* component);
Vector3 (*get_position)(void* transform);


monoArray<void**>* (*get_allCameras)();
Vector3 (*WorldToViewportPoint)(void* camera, Vector3 position);

void* (*get_ScreenPlayerControllerInstance)();
void* (*get_Config)();

monoString* (*GUID_ToString)(void* GUID);

void loadPointers() {
    get_PlayerList = (monoArray<void**>*(*)()) getRealOffset(0xAAB9390);
    GetFromActorNumber = (void*(*)(int)) getRealOffset(0x12B79E0);
    get_LocalPlayer = (void*(*)()) getRealOffset(0x12A7020);

    get_transform = (void*(*)(void*)) getRealOffset(0xCA4442C);
    get_position = (Vector3(*)(void*)) getRealOffset(0xCA52F1C);

    get_allCameras = (monoArray<void**>*(*)()) getRealOffset(0xC9F8C74);
    WorldToViewportPoint = (Vector3(*)(void*, Vector3)) getRealOffset(0xC9F7990);

    get_ScreenPlayerControllerInstance = (void*(*)()) getRealOffset(0x221B60);
    get_Config = (void*(*)()) getRealOffset(0x7D9154);

    GUID_ToString = (monoString*(*)(void*)) getRealOffset(0xA8DD0D4);
}


void* getCamera(int index)
{
    if (index >= 0 && index < get_allCameras()->getLength())
    {
        return get_allCameras()->getPointer()[index];
    }
    return nullptr;
}

Vector3 WorldToScreenPoint(void* camera, Vector3 position)
{
    float screenWidth = [UIApplication sharedApplication].keyWindow.frame.size.width;
    float screenHeight = [UIApplication sharedApplication].keyWindow.frame.size.height;

    if (camera != nullptr) {
        Vector3 fixedPosition = WorldToViewportPoint(camera, position);
        if (fixedPosition.Z > 0.0f) {
            fixedPosition.X *= screenWidth;
            fixedPosition.Y = screenHeight * (1 - fixedPosition.Y);
        }
        return fixedPosition;
    }
    return Vector3(0,0,0);
}