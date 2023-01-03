#include "raylib_impl_sandbox3d.h"

Mesh GenMeshPlaneTiled(float width, float length, int resX, int resZ, int tilesX, int tilesZ)
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

void DrawGuizmo(Matrix transform)
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


BoundingBox GetMeshBoundingBoxTransfomed(Mesh mesh, Matrix transform)
{
    // Get min and max vertex to construct bounds (AABB)
    Vector3 minVertex = { 0 };
    Vector3 maxVertex = { 0 };

    if (mesh.vertices != nullptr)
    {
        Vector3 vertexInit = { mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
        vertexInit = Vector3Transform(vertexInit, transform);
        
        minVertex = vertexInit;
        maxVertex = vertexInit;

        for (int i = 1; i < mesh.vertexCount; i++)
        {
            Vector3 vertex{ mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2] };
            vertex = Vector3Transform(vertex, transform);

            minVertex = Vector3Min(minVertex, vertex);
            maxVertex = Vector3Max(maxVertex, vertex);
        }
    }

    // Create the bounding box
    BoundingBox box = { 0 };
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}

BoundingBox GetModelBoundingBoxTransformed(Model model)
{
    BoundingBox bounds = { 0 };

    if (model.meshCount > 0)
    {
        Vector3 temp = { 0 };
        bounds = GetMeshBoundingBoxTransfomed(model.meshes[0], model.transform);

        for (int i = 1; i < model.meshCount; i++)
        {
            BoundingBox tempBounds = GetMeshBoundingBox(model.meshes[i]);

            temp.x = (bounds.min.x < tempBounds.min.x) ? bounds.min.x : tempBounds.min.x;
            temp.y = (bounds.min.y < tempBounds.min.y) ? bounds.min.y : tempBounds.min.y;
            temp.z = (bounds.min.z < tempBounds.min.z) ? bounds.min.z : tempBounds.min.z;
            bounds.min = temp;

            temp.x = (bounds.max.x > tempBounds.max.x) ? bounds.max.x : tempBounds.max.x;
            temp.y = (bounds.max.y > tempBounds.max.y) ? bounds.max.y : tempBounds.max.y;
            temp.z = (bounds.max.z > tempBounds.max.z) ? bounds.max.z : tempBounds.max.z;
            bounds.max = temp;
        }
    }

    return bounds;
}

Matrix MatrixFromFloat(float* v)
{
    Matrix mat = { 0 };

    mat.m0 = v[0];
    mat.m1 = v[1];
    mat.m2 = v[2];
    mat.m3 = v[3];
    mat.m4 = v[4];
    mat.m5 = v[5];
    mat.m6 = v[6];
    mat.m7 = v[7];
    mat.m8 = v[8];
    mat.m9 = v[9];
    mat.m10 = v[10];
    mat.m11 = v[11];
    mat.m12 = v[12];
    mat.m13 = v[13];
    mat.m14 = v[14];
    mat.m15 = v[15];

    return mat;
}

Matrix MatrixFromFloat(float16 vec)
{
    return MatrixFromFloat(vec.v);
}