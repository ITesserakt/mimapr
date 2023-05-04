#include <lodepng.h>
#include <iostream>

#include "gif.h"
#include "drawer.h"

ImageWriter::~ImageWriter() { heatmap_free(_heatmap); }

ImageWriter::ImageWriter(Eigen::Vector2i size, const Eigen::MatrixXf &points) {
    _heatmap = heatmap_new(size.x(), size.y());

    for (int i = 0; i < points.rows(); i++)
        for (int j = 0; j < points.cols(); j++)
            heatmap_add_weighted_point(_heatmap, i, j, points(i, j));
}

ImageWriter &ImageWriter::addPoint(int x, int y, float weight) {
    heatmap_add_weighted_point(_heatmap, x, y, weight);
    return *this;
}

ImageHandle ImageWriter::write(const heatmap_colorscheme_t *colorscheme) { return {_heatmap, colorscheme}; }

ImageHandle::ImageHandle(heatmap_t *heatmap, const heatmap_colorscheme_t *colorscheme) {
    Data = heatmap_render_to(heatmap, colorscheme, nullptr);
    Width = heatmap->w;
    Height = heatmap->h;
}

std::ostream &operator<<(std::ostream &os, const ImageHandle &handle) {
    std::vector<unsigned char> encodedData;
    if (auto result = lodepng::encode(encodedData, handle.Data, handle.Width, handle.Height) != 0) {
        std::cerr << "Exception occurred while encoding png: " << lodepng_error_text(result) << std::endl;
        throw std::exception{lodepng_error_text(result)};
    }

    for (const auto &item : encodedData)
        os << item;
    return os;
}

void GifImageWriter::addFrame(ImageWriter &&writer) { _frames.push_back(writer.write(_colors)); }

GifImageWriter::GifImageWriter(const heatmap_colorscheme_t *colors) : _colors(colors) {}

void GifImageWriter::saveToFile(const std::string& filename) const {
    GifWriter gif;
    auto width = _frames[0].Width;
    auto height = _frames[0].Height;
    auto delay = 10;

    GifBegin(&gif, "image.gif", width, height, delay);
    for (const auto& frame: _frames)
        GifWriteFrame(&gif, frame.Data, width, height, delay);
    GifEnd(&gif);
}
