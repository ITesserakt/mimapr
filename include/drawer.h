#pragma once

#include <Eigen/Core>
#include <heatmap.h>
#include <ostream>

struct ImageHandle {
    const unsigned char *Data;
    unsigned int Width, Height;

    ImageHandle(heatmap_t *heatmap, const heatmap_colorscheme_t *colorscheme);
    ImageHandle(const unsigned char *data, unsigned int width, unsigned int height);
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
    ImageWriter &addPoint(int x, int y, float weight, heatmap_stamp_t* stamp);

    ImageHandle write(const heatmap_colorscheme_t *colorscheme = heatmap_cs_default) const;

    ~ImageWriter();
};

class GifImageWriter {
  private:
    std::vector<ImageHandle> _frames;
    const heatmap_colorscheme_t *_colors;

  public:
    explicit GifImageWriter(const heatmap_colorscheme_t *colors = heatmap_cs_default);

    void addFrame(ImageWriter&& writer);

    void saveToFile(const std::string& filename) const;
};