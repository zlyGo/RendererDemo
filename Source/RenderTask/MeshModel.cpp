#include "MeshModel.h"
#include "tgaimage.h"

namespace RenderTask {
    namespace {
        static const TGAColor white = { 255, 255, 255, 255 };
        static const TGAColor red = { 255, 0, 0, 255 };
        static constexpr int width = 800;
        static constexpr int height = 800;

        TGAColor GetRandomColor()
        {
            TGAColor color;
            color[0] = rand() % 255;
            color[1] = rand() % 255;
            color[2] = rand() % 255;
            color[4] = 255;
            return color;
        }

        void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image)
        {
            // TODO: why cannot exchange the order of checking steep and comparing x0 with x1?
            bool steep = false;
            if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
                std::swap(x0, y0);
                std::swap(x1, y1);
                steep = true;
            }

            if (x0 > x1) {
                std::swap(x0, x1);
                std::swap(y0, y1);
            }

            int dx = x1 - x0;
            int dy = y1 - y0;
            int dError2 = std::abs(dy) * 2;
            int error2 = 0;
            int x = x0;
            int y = y0;
            TGAColor color = white;

            while (x <= x1) {
                if (steep) {
                    image.set(y, x, color);
                } else {
                    image.set(x, y, color);
                }
                error2 += dError2;
                if (error2 > dx) { // no obvious change if using >= here
                    y += (y1 > y0 ? 1 : -1);
                    error2 -= dx * 2;
                }
                ++x;
            }
        }
    };

    MeshModel::MeshModel(const std::string& modelPath)
        :m_modelPath(modelPath)
    {
        LoadModel(modelPath);
    }

    bool MeshModel::WriteMeshIntoFile(const std::string& fileName, FileType type)
    {
        if (fileName.empty()) {
            return false;
        }

        if (!m_model && !LoadModel(m_modelPath)) {
            return false;
        }

        TGAImage image(width, height, TGAImage::Format::RGB);
        int faces = m_model->nfaces();
        for (int i = 0; i < faces; ++i) {
            for (int j = 0; j < 3; ++j) {
                vec3 vert0 = m_model->vert(i, j);
                vec3 vert1 = m_model->vert(i, (j + 1) % 3);
                auto x0 = static_cast<int>((vert0.x + 1.0) * width / 2.0);
                auto y0 = static_cast<int>((vert0.y + 1.0) * height / 2.0);
                auto x1 = static_cast<int>((vert1.x +1.0) * width / 2.0);
                auto y1 = static_cast<int>((vert1.y + 1.0) * height / 2.0);
                DrawLine(x0, y0, x1, y1, image);
            }
        }

        image.write_tga_file(fileName);
    }

    bool MeshModel::LoadModel(const std::string& modelPath)
    {
        if (modelPath.empty()) {
            return false;
        }

        try {
            m_model = std::make_unique<Model>(modelPath);
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }

        return true;
    }
}