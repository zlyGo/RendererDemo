#ifndef MESH_MODEL_H
#define MESH_MODEL_H
#include <cstring>
#include <memory>
#include "model.h"

namespace RenderTask {
    enum class FileType {
        TGA,
    };

    class MeshModel {
    public:
        MeshModel(const std::string& modelPath);
        ~MeshModel() = default;
        bool WriteMeshIntoFile(const std::string& fileName, FileType type = FileType::TGA);

    private:
        bool LoadModel(const std::string& modelPath);

    private:
        std::string m_modelPath{ "" };
        std::unique_ptr<Model> m_model{ nullptr };
    };
}

#endif
