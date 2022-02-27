#include "tgaimage.h"

namespace {
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
TGAColor GetRandomColor()
{
    TGAColor color;
    color[0] = rand() % 255;
    color[1] = rand() % 255;
    color[2] = rand() % 255;
    color[4] = 255;
    return color;
}

void DrawLine(int x0, int y0, int x1, int y1, float precision, TGAImage& image)
{
    auto color = GetRandomColor();
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    float x = x0;
    float y = y0;

    while (x < x1 && (y - y1) * (y0 - y1) > 0) {
        image.set(static_cast<int>(x), static_cast<int>(y), color);
        x += (x1 - x0) * precision;
        y += (y1 - y0) * precision;

    }
    //while (x < x1) {
    //    image.set(static_cast<int>(x), static_cast<int>(y), GetRandomColor());
    //    x += (x1 - x0) * xPrecision;
    //}
    //while (y < y1) {
    //    image.set(static_cast<int>(x), static_cast<int>(y), GetRandomColor());
    //    y += (y1 - y0) * yPrecision;
    //}
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
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
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

}

int main()
{
    int width = 100;
    int height = 100;
    float precision = 0.001;
    //float yPrecision = 0.01;
    TGAImage image(width, height, TGAImage::RGB);
    //image.set(52, 41, red);
    //DrawLine(0, 0, width, height, precision, image);
    //DrawLine(0, height, width, 0, precision, image);
    //DrawLine(13, 20, 80, 40, precision, image);
    //DrawLine(20, 13, 40, 80, precision, image);
    //DrawLine(80, 40, 13, 20, precision, image);
    line(13, 20, 80, 40, image, white);
    line(20, 13, 40, 80, image, red);
    line(80, 40, 13, 20, image, red);
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}