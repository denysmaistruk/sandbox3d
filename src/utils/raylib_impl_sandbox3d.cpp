#include "raylib_impl_sandbox3d.h"

#include "raymath.h"

Mesh genMeshPlaneTiled(float width, float length, int resX, int resZ, int tilesX, int tilesZ)
{

    Mesh mesh = { 0 };

#define CUSTOM_MESH_GEN_PLANE
#if defined(CUSTOM_MESH_GEN_PLANE)
    resX++;
    resZ++;

    // Vertices definition
    int vertexCount = resX * resZ; // vertices get reused for the faces

    Vector3* vertices = (Vector3*)RL_MALLOC(vertexCount * sizeof(Vector3));
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z / (resZ - 1) - 0.5f) * length;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x / (resX - 1) - 0.5f) * width;
            vertices[x + z * resX] = Vector3{ xPos, 0.0f, zPos };
        }
    }

    // Normals definition
    Vector3* normals = (Vector3*)RL_MALLOC(vertexCount * sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++) normals[n] = Vector3{ 0.0f, 1.0f, 0.0f };   // Vector3.up;

    // TexCoords definition
    Vector2* texcoords = (Vector2*)RL_MALLOC(vertexCount * sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v * resX] = Vector2Multiply( 
                Vector2{(float)u/(resX-1), (float)v/(resZ-1)},Vector2{(float)tilesX, (float)tilesZ});
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1) * (resZ - 1);
    int* triangles = (int*)RL_MALLOC(numFaces * 6 * sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face % (resX - 1) + (face / (resZ - 1) * resX);

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resX;
        triangles[t++] = i + resX + 1;
        triangles[t++] = i + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces * 2;
    mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3 * i] = vertices[i].x;
        mesh.vertices[3 * i + 1] = vertices[i].y;
        mesh.vertices[3 * i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2 * i] = texcoords[i].x;
        mesh.texcoords[2 * i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3 * i] = normals[i].x;
        mesh.normals[3 * i + 1] = normals[i].y;
        mesh.normals[3 * i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount * 3; i++) mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

#else       // Use par_shapes library to generate plane mesh

    par_shapes_mesh* plane = par_shapes_create_plane(resX, resZ);   // No normals/texcoords generated!!!
    par_shapes_scale(plane, width, length, 1.0f);
    par_shapes_rotate(plane, -PI / 2.0f, (float[]) { 1, 0, 0 });
    par_shapes_translate(plane, -width / 2, 0.0f, length / 2);

    mesh.vertices = (float*)RL_MALLOC(plane->ntriangles * 3 * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(plane->ntriangles * 3 * 2 * sizeof(float));
    mesh.normals = (float*)RL_MALLOC(plane->ntriangles * 3 * 3 * sizeof(float));

    mesh.vertexCount = plane->ntriangles * 3;
    mesh.triangleCount = plane->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k * 3] = plane->points[plane->triangles[k] * 3];
        mesh.vertices[k * 3 + 1] = plane->points[plane->triangles[k] * 3 + 1];
        mesh.vertices[k * 3 + 2] = plane->points[plane->triangles[k] * 3 + 2];

        mesh.normals[k * 3] = plane->normals[plane->triangles[k] * 3];
        mesh.normals[k * 3 + 1] = plane->normals[plane->triangles[k] * 3 + 1];
        mesh.normals[k * 3 + 2] = plane->normals[plane->triangles[k] * 3 + 2];

        mesh.texcoords[k * 2] = plane->tcoords[plane->triangles[k] * 2];
        mesh.texcoords[k * 2 + 1] = plane->tcoords[plane->triangles[k] * 2 + 1];
    }

    par_shapes_free_mesh(plane);
#endif

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

void drawGuizmo(Matrix transform)
{
    Vector3 origin = Vector3Transform(Vector3Zero(), transform);
    DrawLine3D(origin, Vector3Transform(Vector3{ 1.f, 0.f, 0.f }, transform), RED);
    DrawLine3D(origin, Vector3Transform(Vector3{ 0.f, 1.0f, 0.f }, transform), GREEN);
    DrawLine3D(origin, Vector3Transform(Vector3{ 0.f, 0.f, 1.f }, transform), BLUE);
}

Vector3 Vector3Translate(Vector3 v, Matrix mat)
{
    return Vector3{ v.x + mat.m12, v.y + mat.m13, v.z + mat.m14 };
}