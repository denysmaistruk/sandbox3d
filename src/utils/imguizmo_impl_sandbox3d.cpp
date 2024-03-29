#include "imguizmo_impl_sandbox3d.h"

#include "imgui.h"
#include "imguizmo.h"
#include "raymath.h"
#include "rlgl.h"

#include "core/system/render/utils.h"

#include "utils/raylib_impl_sandbox3d.h"

static Matrix GetCameraView(const Camera& camera)
{
    assert(camera.projection == CAMERA_PERSPECTIVE);
    return MatrixLookAt(camera.position, camera.target, camera.up);
}

static Matrix GetCameraProjection(const Camera& camera) {
    double const zfar = RL_CULL_DISTANCE_FAR;
    double const znear = RL_CULL_DISTANCE_NEAR;
    double const top = znear * tan(camera.fovy * 0.5 * DEG2RAD);
    double const right = top * SANDBOX3D_WINDOW_ASPECT;
    double const left = -right;
    double const bottom = -top;

    return MatrixFrustum(left, right, bottom, top, znear, zfar);
}

void ImGuizmo_ImplSandbox3d_EditTransform(const Camera& camera, Matrix& matrix)
{
    float* matrix16 = MatrixToFloat(matrix);

    static ImGuizmo::OPERATION operation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE mode(ImGuizmo::MODE::WORLD);

    bool open = true;
    

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
    ImGuizmo::DecomposeMatrixToComponents(matrix16, matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Tr", matrixTranslation);
    ImGui::InputFloat3("Rt", matrixRotation);
    ImGui::InputFloat3("Sc", matrixScale);

    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix16);

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
    
    ImGuizmo::Manipulate(MatrixToFloat(GetCameraView(camera)), MatrixToFloat(GetCameraProjection(camera)), operation, mode, matrix16, nullptr, nullptr);
    matrix = matrixFromFloat(matrix16);    
}