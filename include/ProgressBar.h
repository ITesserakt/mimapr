#pragma once

#include <ostream>
class ProgressBar {
  private:
    int _barWidth;
    float _progress;
    float _size;

  public:
    explicit ProgressBar(float size, int barWidth = 50);

    friend std::ostream &operator<<(std::ostream &os, const ProgressBar &bar);

    void operator++(int);
};