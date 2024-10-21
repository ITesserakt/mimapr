// Original source: https://github.com/tsoding/rendering-video-in-c-with-ffmpeg/blob/master/ffmpeg.h

#pragma once

#include <cstddef>

struct FFMPEG_IMPL;

struct FFMPEG {
    std::size_t width, height;
    FFMPEG_IMPL *impl;

    FFMPEG(std::size_t width, std::size_t height, std::size_t fps);

    void send_frame(const std::uint32_t *data) const;

    ~FFMPEG();
};
