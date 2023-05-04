#pragma once

#include <Eigen/Core>
#include <heatmap.h>
#include <ostream>

class ImageHandle {
  private:
    heatmap_t *_heatmap;
    heatmap_colorscheme_t *_colorscheme;

    [[nodiscard]] Eigen::Vector2i size() const;

  public:
    ImageHandle(heatmap_t *heatmap, heatmap_colorscheme_t *colorscheme);
    ImageHandle(const ImageHandle &) = delete;
    ImageHandle &operator=(const ImageHandle &) = delete;
    ImageHandle(ImageHandle &&) = default;
    ImageHandle &operator=(ImageHandle &&) = default;

    friend std::ostream &operator<<(std::ostream &os, const ImageHandle &handle);
};

class ImageWriter {
  private:
    heatmap_t *_heatmap;

  public:
    explicit ImageWriter(Eigen::Vector2i size, const Eigen::MatrixXf &points = {});
    ImageWriter(const ImageWriter &) = delete;
    ImageWriter &operator=(const ImageWriter &) = delete;
    ImageWriter(ImageWriter &&) = default;
    ImageWriter &operator=(ImageWriter &&) = default;

    ImageWriter &addPoint(int x, int y, float weight = 1);

    ImageHandle write(heatmap_colorscheme_t *colorscheme = (heatmap_colorscheme_t*)heatmap_cs_default);

    ~ImageWriter();
};