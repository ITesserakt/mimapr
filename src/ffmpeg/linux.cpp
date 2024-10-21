// Original code: https://github.com/tsoding/rendering-video-in-c-with-ffmpeg/blob/master/ffmpeg_linux.c

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ffmpeg.h"

#include <stdexcept>

#define READ_END 0
#define WRITE_END 1

struct FFMPEG_IMPL {
    int pid;
    int pipe;
};

FFMPEG::FFMPEG(std::size_t width, std::size_t height, std::size_t fps) : width(width), height(height) {
    int pipefd[2];

    if (pipe(pipefd) < 0)
        throw std::runtime_error(std::format("ERROR: could not create a pipe: {}\n", strerror(errno)));
    const pid_t child = fork();
    if (child < 0)
        throw std::runtime_error(std::format("ERROR: could not fork child process: {}\n", strerror(errno)));

    if (child == 0) {
        if (dup2(pipefd[READ_END], STDIN_FILENO) < 0) {
            throw std::runtime_error(std::format("ERROR: could not redirect stdin: {}\n", strerror(errno)));
        }
        close(pipefd[WRITE_END]);

        const auto resolution = std::format("{}x{}", width, height);
        const auto framerate = std::format("{}", fps);

        const int ret =
            execlp("ffmpeg", "ffmpeg", "-loglevel", "info", "-y", "-f", "rawvideo", "-pix_fmt", "rgba", "-s",
                   resolution.c_str(), "-r", framerate.c_str(), "-i", "-", "-pix_fmt", "yuva420p", "output.webm", NULL);
        if (ret < 0)
            throw std::runtime_error(std::format("ERROR: could not exec ffmpeg: {}\n", strerror(errno)));
    }

    close(pipefd[READ_END]);
    impl = new FFMPEG_IMPL();
    impl->pid = child;
    impl->pipe = pipefd[WRITE_END];
}

void FFMPEG::send_frame(const std::uint32_t *data) const { write(impl->pipe, data, sizeof(uint32_t) * width * height); }

FFMPEG::~FFMPEG() {
    close(impl->pipe);
    waitpid(impl->pid, nullptr, 0);
}
