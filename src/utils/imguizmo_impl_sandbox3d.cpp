#include "imguizmo_impl_sandbox3d.h"

#include "imgui.h"
#include "imguizmo.h"
#include "raymath.h"

#include "graphics/lights.h"

static Matrix GetCameraView(const Camera& camera)
{
    assert(camera.projection == CAMERA_PERSPECTIVE);
    return MatrixLookAt(camera.position, camera.target, camera.up);
}

static Matrix GetCameraProjection(const Camera& camera)
{
    assert(camera.projection == CAMERA_PERSPECTIVE);
    return CameraFrustum(camera);
}

void ImGuizmo_ImplSandbox3d_EditTransform(const Camera& camera, Matrix& matrix)
{
    static ImGuizmo::OPERATION operation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mode(ImGuizmo::MODE::WORLD);

    if (ImGui::RadioButton("Translate", operation == ImGuizmo::TRANSLATE))
    {
        operation = ImGuizmo::TRANSLATE;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", operation == ImGuizmo::ROTATE))
    {
        operation = ImGuizmo::ROTATE;
    }
    
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", operation == ImGuizmo::SCALE))
    {
        operation = ImGuizmo::SCALE;
    }
    
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    //ImGuizmo::DecomposeMatrixToComponents(matrix.m16, matrixTranslation, matrixRotation, matrixScale);
    ImGuizmo::DecomposeMatrixToComponents(MatrixToFloat(matrix), matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Tr", matrixTranslation);
    ImGui::InputFloat3("Rt", matrixRotation);
    ImGui::InputFloat3("Sc", matrixScale);
    
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, MatrixToFloat(matrix));

    if (operation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mode == ImGuizmo::LOCAL))
        { 
            mode = ImGuizmo::LOCAL;
        }
            
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mode == ImGuizmo::WORLD))
        {
            mode = ImGuizmo::WORLD;
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    //ImGuizmo::Manipulate(camera.mView.m16, camera.mProjection.m16, currentGizmoOperation, currentGizmoMode, MatrixToFloat(matrix), NULL, useSnap ? &snap.x : NULL);
    
    if (ImGuizmo::Manipulate(MatrixToFloat(GetCameraView(camera)), MatrixToFloat(GetCameraProjection(camera)), operation, mode, MatrixToFloat(matrix), nullptr, nullptr))
    {
        int i = 42;
        int j = i;
    }
}