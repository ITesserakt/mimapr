#include <lodepng.h>
#include <iostream>

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

ImageHandle ImageWriter::write(heatmap_colorscheme_t *colorscheme) { return {_heatmap, colorscheme}; }

ImageHandle::ImageHandle(heatmap_t *heatmap, heatmap_colorscheme_t *colorscheme)
    : _heatmap(heatmap), _colorscheme(colorscheme) {}

std::ostream &operator<<(std::ostream &os, const ImageHandle &handle) {
    auto renderedData = heatmap_render_to(handle._heatmap, handle._colorscheme, nullptr);

    std::vector<unsigned char> encodedData;
    if (auto result = lodepng::encode(encodedData, renderedData, handle._heatmap->w, handle._heatmap->h) != 0) {
        std::cerr << "Exception occurred while encoding png: " << lodepng_error_text(result) << std::endl;
        throw std::exception{lodepng_error_text(result)};
    }

    for (const auto& item: encodedData)
        os << item;
    return os;
}

Eigen::Vector2i ImageHandle::size() const { return {_heatmap->w, _heatmap->h}; }
