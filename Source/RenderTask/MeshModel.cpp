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

        void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color)
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

        void DrawTriangle(vec2 t0, vec2 t1, vec2 t2, TGAImage& image, TGAColor color)
        {
            if (t0.y > t1.y) {
                std::swap(t0, t1);
            }

            if (t0.y > t2.y) {
                std::swap(t0, t2);
            }

            if (t1.y > t2.y) {
                std::swap(t1, t2);
            }

            for (int y = t0.y; y < t2.y; ++y) {
                int x0 = t2.x - (t2.x - t0.x) * (t2.y - y) * 1.0 / (t2.y - t0.y);
                int x1 = 0;
                if (y < t1.y) {
                    x1 = t1.x - (t1.x - t0.x) * (t1.y - y) * 1.0 / (t1.y - t0.y);
                }
                else {
                    x1 = t2.x - (t2.x - t1.x) * (t2.y - y) * 1.0 / (t2.y - t1.y);
                }

                DrawLine(x0, y, x1, y, image, color);
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
                DrawLine(x0, y0, x1, y1, image, white);
            }
        }

        image.write_tga_file(fileName);
    }

    bool MeshModel::WriteRasterizedMeshIntoFile(const std::string& fileName, FileType type)
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
                vec3 vert2 = m_model->vert(i, (j + 2) % 3);
                vec2 v0;
                vec2 v1;
                vec2 v2;
                // NOTE: static_cast to int have to be done, otherwise a precision problem may happen
                v0.x = static_cast<int>((vert0.x + 1.0) * width / 2.0);
                v0.y = static_cast<int>((vert0.y + 1.0) * height / 2.0);
                v1.x = static_cast<int>((vert1.x + 1.0) * width / 2.0);
                v1.y = static_cast<int>((vert1.y + 1.0) * height / 2.0);
                v2.x = static_cast<int>((vert2.x + 1.0) * width / 2.0);
                v2.y = static_cast<int>((vert2.y + 1.0) * height / 2.0);
                DrawTriangle(v0, v1, v2, image, GetRandomColor());
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