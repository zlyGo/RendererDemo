#include "MeshModel.h"

using namespace RenderTask;

int main(int argc, char** argv)
{
    MeshModel meshTask("african_head.obj");
    meshTask.WriteMeshIntoFile("frameBuffer.tga");

    return 0;
}