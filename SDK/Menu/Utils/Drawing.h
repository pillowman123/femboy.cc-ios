void DrawLine(ImVec2 start, ImVec2 end, ImVec4 color) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        background->AddLine(start, end, ImColor(color.x,color.y,color.z,color.w));
    }
}

void DrawBox(float x, float y, float z, float w, ImVec4 color) {
    ImVec2 v1(x, y);
    ImVec2 v2(x + z, y);
    ImVec2 v3(x + z, y + w);
    ImVec2 v4(x, y + w);

    DrawLine(v1, v2, color);
    DrawLine(v2, v3, color);
    DrawLine(v3, v4, color);
    DrawLine(v4, v1, color);
}

void DrawBoxFilled(float x, float y, float z, float w, ImVec4 color) {
    auto background = ImGui::GetBackgroundDrawList();
    if(background) {
        background->AddRectFilled(ImVec2(x, y), ImVec2(z, w), ImColor(color.x,color.y,color.z,color.w), 0.0f);
    }
}


void DrawFilledRect(int x, int y, int w, int h, ImVec4 color) {
    auto background = ImGui::GetBackgroundDrawList();
    background->AddRectFilled(ImVec2(x, y - 1), ImVec2(x + w, y + h), ImColor(color.x,color.y,color.z, 0.07), 0, 0);
    background->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + w, y + h), ImColor(color.x,color.y,color.z, 0.07), 0, 0);
    background->AddRectFilled(ImVec2(x - 1, y), ImVec2(x + w, y + h), ImColor(color.x,color.y,color.z, 0.07), 0, 0);
    background->AddRectFilled(ImVec2(x + 1, y), ImVec2(x + w, y + h), ImColor(color.x,color.y,color.z, 0.07), 0, 0);
    background->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(color.x,color.y,color.z,0.07), 0, 0);

    background->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(color.x,color.y,color.z, 1.0), 0, 0);
}