
void globalManager() {
    float screenWidth = [UIApplication sharedApplication].keyWindow.frame.size.width;
    float screenHeight = [UIApplication sharedApplication].keyWindow.frame.size.height;

    void* bestPlayer = nullptr;
    float bestFov = circleSize;

    if (renderCirlce && activateAimbot)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(screenWidth / 2, screenHeight / 2), circleSize, ImColor(255, 255, 255, 255), 360, 1.0f);
    }

    monoArray<void**>* players = get_PlayerList();
    for (int i = 0; i < players->getLength(); ++i) {
        void* photonPlayer = players->getPointer()[i];
        if (photonPlayer != nullptr && getCamera(0) != nullptr) {
            void* plr = GetFromActorNumber(*(int*)((uint64_t)photonPlayer + 0x40));
            
            if (plr != nullptr) {
               
                if (plr == get_LocalPlayer() && get_LocalPlayer() != nullptr) {
                    continue;
                }

                void* head = *(void**)((uint64_t)plr + 0xe0);                
                void* body = *(void**)((uint64_t)plr + 0xf8);               
                void* leftHand = *(void**)((uint64_t)plr + 0x100);
                void* rightHand = *(void**)((uint64_t)plr + 0x108);


                if (head != nullptr && body != nullptr && leftHand != nullptr && rightHand != nullptr)
                {
                    Vector3 headPosition = get_position(get_transform(head));
                    Vector3 headScreenPos = WorldToScreenPoint(getCamera(0), headPosition);

                    Vector3 bodyPosition = get_position(get_transform(body));
                    Vector3 bodyScreenPos = WorldToScreenPoint(getCamera(0), bodyPosition);

                    Vector3 leftHandPosition = get_position(get_transform(leftHand));
                    Vector3 leftHandScreenPos = WorldToScreenPoint(getCamera(0), leftHandPosition);

                    Vector3 rightHandPosition = get_position(get_transform(rightHand));
                    Vector3 rightHandScreenPos = WorldToScreenPoint(getCamera(0), rightHandPosition);

                    if (lines && headScreenPos.Z > 1.f) {
                        if (topLines) {
                            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screenWidth / 2, 0.0f), ImVec2(headScreenPos.X, headScreenPos.Y), ImColor(255, 255, 255, 255), 1.0f);
                        }
                        else {
                            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screenWidth / 2, screenHeight / 2), ImVec2(headScreenPos.X, headScreenPos.Y), ImColor(255, 255, 255, 255), 1.0f);
                        }
                    }

                    if (skeletons && headScreenPos.Z > 1.f && bodyScreenPos.Z > 1.f && leftHandScreenPos.Z > 1.f && rightHandScreenPos.Z > 1.f)
                    {
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(headScreenPos.X, headScreenPos.Y), ImVec2(bodyScreenPos.X, bodyScreenPos.Y), ImColor(255, 255, 255, 255), 1.0f);
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bodyScreenPos.X, bodyScreenPos.Y), ImVec2(leftHandScreenPos.X, leftHandScreenPos.Y), ImColor(255, 255, 255, 255), 1.0f);
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(bodyScreenPos.X, bodyScreenPos.Y), ImVec2(rightHandScreenPos.X, rightHandScreenPos.Y), ImColor(255, 255, 255, 255), 1.0f);
                    }

                    if (boxes && headScreenPos.Z > 1.f && bodyScreenPos.Z > 1.f && leftHandScreenPos.Z > 1.f && rightHandScreenPos.Z > 1.f)
                    {
                        float boxHeight = abs(headScreenPos.Y - std::max(bodyScreenPos.Y, std::max(leftHandScreenPos.Y, rightHandScreenPos.Z)));
                        float minX = std::min({headScreenPos.X, bodyScreenPos.X, leftHandScreenPos.X, rightHandScreenPos.X});
                        float maxX = std::max({headScreenPos.X, bodyScreenPos.X, leftHandScreenPos.X, rightHandScreenPos.X});
                        float boxWidth = maxX - minX;
                        Vector2 box = {minX, headScreenPos.Y};

                        DrawBox(box.X, box.Y, boxWidth, boxHeight, ImVec4(1, 1, 1, 1));
                    }

                    Vector2 screenCenter(screenWidth / 2, screenHeight / 2);
                    if (headScreenPos.Z > 1.f)
                    {
                        float crosshairDistance = Vector2::Distance(screenCenter, Vector2(headScreenPos.X, headScreenPos.Y));
                        if (crosshairDistance < bestFov)
                            bestFov = crosshairDistance;
                        if (crosshairDistance == bestFov)
                            bestPlayer = plr;
                    }
                }
            }
        }

        if (activateAimbot)
        {
            void* screenPlayerController = get_ScreenPlayerControllerInstance();
            if (bestPlayer != nullptr && getCamera(0) != nullptr && screenPlayerController != nullptr) {
                void* closestPlayerHead = *(void**)((uint64_t)bestPlayer + 0xe0);
                void* closestPlayerBody = *(void**)((uint64_t)bestPlayer + 0xf8);
                void* cameraSystem = *(void**)((uint64_t)screenPlayerController + 0x198);
                if (closestPlayerHead != nullptr && closestPlayerBody != nullptr && cameraSystem != nullptr) {
                    
                    Vector3 closestPlayerHeadPosition = get_position(get_transform(closestPlayerHead));
                    Vector3 closestPlayerHeadScreenPos = WorldToScreenPoint(getCamera(0), closestPlayerHeadPosition);

                    Vector3 closestPlayerBodyPosition = get_position(get_transform(closestPlayerBody));
                    Vector3 closestPlayerBodyScreenPos = WorldToScreenPoint(getCamera(0), closestPlayerBodyPosition);

                    Vector3 cameraPosition = get_position(get_transform(getCamera(0)));
                    Quaternion currentRotation = *(Quaternion*)((uint64_t)cameraSystem + 0x98);

                    if (headLock) {
                        Quaternion rotToTarget{};
                        Vector3 dirToTarget = Vector3::Normalized(closestPlayerHeadPosition - cameraPosition);
                        if (customStrength) {
                            rotToTarget = Quaternion::Slerp(currentRotation, Quaternion::LookRotation(dirToTarget), ImGui::GetIO().DeltaTime * strengthAmount);
                        }
                        else {
                            rotToTarget = Quaternion::LookRotation(dirToTarget);
                        }
                        *(Quaternion*)((uint64_t)cameraSystem + 0x98) = rotToTarget;
                    }

                    else if (bodyLock) {
                        Quaternion rotToTarget{};
                        Vector3 dirToTarget = Vector3::Normalized(closestPlayerBodyPosition - cameraPosition);
                        if (customStrength) {
                            rotToTarget = Quaternion::Slerp(currentRotation, Quaternion::LookRotation(dirToTarget), ImGui::GetIO().DeltaTime * strengthAmount);
                        }
                        else {
                            rotToTarget = Quaternion::LookRotation(dirToTarget);
                        }
                        *(Quaternion*)((uint64_t)cameraSystem + 0x98) = rotToTarget;
                    }
                }
            }
        }


    }
}