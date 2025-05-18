#pragma once
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/backends/imgui_impl_glfw.h"
#include "../thirdparty/imgui/backends/imgui_impl_opengl3.h"
#include "../thirdparty/imgui/imgui.h"
#include "render.hpp"

void imgui_console(RenderState *renderState)
{
    ImGui::Begin("Settings Menu");

    ImGui::Text("Render Options");
    ImGui::Checkbox("Enable Bloom", &renderState->bloom);
    ImGui::Checkbox("Enable sRGB", &renderState->sRGB);
    ImGui::Checkbox("Enable Normals", &renderState->useNormal);
    ImGui::Checkbox("Enable Shadows", &renderState->useShadows);
    ImGui::Checkbox("Enable hdr", &renderState->hdr);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::SliderFloat("Exposure", &renderState->exposure, 0.0f, 5.0f);

    ImGui::End();
}