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

        vec3 Barycentric(vec3 v0, vec3 v1, vec3 v2, vec3 p)
        {
            vec3 u = cross({ v2.x - v0.x, v1.x - v0.x, v0.x - p.x }, { v2.y - v0.y, v1.y - v0.y, v0.y - p.y });
            if (std::abs(u.z) < 1e-2) {
                return vec3(-1, -1, -1);
            }

            return { 1.0 - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z };
        }

        void DrawTriangleWithDepth(vec3 v0, vec3 v1, vec3 v2, std::vector<float>& zBuffer, TGAImage& image, TGAColor color)
        {
            vec2 bBoxMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            vec2 bBoxMax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

            vec2 clamp((double)image.width() - 1, (double)image.height() - 1);
            // i = 0 for x coordinate, i = 1 for y coordinate.
            for (int i = 0; i < 2; ++i) {
                bBoxMin[i] = std::min(bBoxMin[i], v0[i]);
                bBoxMin[i] = std::min(bBoxMin[i], v1[i]);
                bBoxMin[i] = std::max(0.0, std::min(bBoxMin[i], v2[i]));
                bBoxMax[i] = std::max(bBoxMax[i], v0[i]);
                bBoxMax[i] = std::max(bBoxMax[i], v1[i]);
                bBoxMax[i] = std::min(clamp[i], std::max(bBoxMax[i], v2[i]));
            }

            vec3 p;
            for (p.x = bBoxMin.x; p.x < bBoxMax.x; ++p.x) {
                for (p.y = bBoxMin.y; p.y < bBoxMax.y; ++p.y) {
                    auto screenBC = Barycentric(v0, v1, v2, p);
                    if (screenBC.x < 0 || screenBC.y < 0 || screenBC.z < 0) {
                        continue;
                    }

                    p.z = float(v0.z * screenBC.x + v1.z * screenBC.y + v2.z * screenBC.z);

                    if (zBuffer[int(p.x + p.y * width)] < p.z) {
                        zBuffer[int(p.x + p.y * width)] = p.z;
                        image.set(p.x, p.y, color);
                    }
                }
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
        // NOTE: FLT_MIN is a positive value!
        std::vector<float> zBuffer(width * height, -std::numeric_limits<float>::max());

        vec3 lightDir(0, 0, -1);
        int faces = m_model->nfaces();
        for (int i = 0; i < faces; ++i) {
            vec3 vert0 = m_model->vert(i, 0);
            vec3 vert1 = m_model->vert(i, 1);
            vec3 vert2 = m_model->vert(i, 2);

            vec3 v0;
            vec3 v1;
            vec3 v2;
            // NOTE: static_cast to int have to be done, otherwise a precision problem may happen
            v0.x = static_cast<int>((vert0.x + 1.0) * width / 2.0);
            v0.y = static_cast<int>((vert0.y + 1.0) * height / 2.0);
            v1.x = static_cast<int>((vert1.x + 1.0) * width / 2.0);
            v1.y = static_cast<int>((vert1.y + 1.0) * height / 2.0);
            v2.x = static_cast<int>((vert2.x + 1.0) * width / 2.0);
            v2.y = static_cast<int>((vert2.y + 1.0) * height / 2.0);
            v0.z = vert0.z;
            v1.z = vert1.z;
            v2.z = vert2.z;

            vec3 n = cross(vert2 - vert0, vert1 - vert0);
            n.normalize();
            float intensity = n * lightDir;
            if (intensity > 0) {
                DrawTriangleWithDepth(v0, v1, v2, zBuffer, image,
                    TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
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