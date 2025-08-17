
bool ChildCheckbox(const char* title, bool* v)
{
    ImGui::Spacing();
    ImGui::SameLine();
    return ImGui::Checkbox(title, v);
}

std::vector<std::string> logs;

void Log(const char* fmt, ...) {
    char buffer[512]; // Adjust size if needed
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    logs.push_back(std::string(buffer));
}

void RenderMenu()
{

    ImGui::Begin("Log");
    if (ImGui::Button("Clear")) {
        logs.clear();
    }
    ImGui::Separator();
    for (const auto& log : logs) {
        ImGui::Text("[Log] - %s", log.c_str());
    }
    ImGui::End();

    ImGui::Begin("femboy.cc - [IOS] | from the love of: pillow <3");

    if (ImGui::Button("Dump Clothing")) {
        void* avatarItemConfig = get_Config();
        if (avatarItemConfig != nullptr) {
            monoDictionary<void**, void**>* caidToAvatarItemSelections = *(monoDictionary<void**, void**>**)((uint64_t)avatarItemConfig + 0x50);
            Log("size: %i", caidToAvatarItemSelections->getSize());
            void* serializedData = *(void**)((uint64_t)caidToAvatarItemSelections + 0x0);
            if (serializedData != nullptr) {
                monoArray<void**>* items = *(monoArray<void**>**)((uint64_t)serializedData + 0x10);
                Log("size: %i", *(int*)((uint64_t)serializedData + 0x18));
                for (int i = 0; i < items->getLength(); ++i) {
                    void* avatarItem = items->getPointer()[i];
                    if (avatarItem != nullptr) {
                    }
                }
            }
        }
    }


    ImGui::SeparatorText("ESP");
    ImGui::Checkbox("Enable ESP", &activateEsp);
    if (activateEsp) {
        ChildCheckbox("Lines", &lines);
        if (lines) {
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Checkbox("Top Screen", &topLines);
        }
        ChildCheckbox("Skeletons", &skeletons);
        ChildCheckbox("Boxes", &boxes);
    }

    ImGui::SeparatorText("Aimbot");
    ImGui::Checkbox("Enable Aimbot", &activateAimbot);
    if (activateAimbot) {
        ChildCheckbox("Render Circle", &renderCirlce);
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::SliderFloat("Circle Size", &circleSize, 0.0f, 360.0f);
        ChildCheckbox("Custom Strength", &customStrength);
        if (customStrength) {
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::SliderFloat("Strength", &strengthAmount, 0.0f, 100.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox("Head Lock", &headLock);
        ImGui::Checkbox("Body Lock", &bodyLock);
    }
    ImGui::End();
}