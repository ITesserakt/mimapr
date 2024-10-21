#include <iomanip>

#include "ProgressBar.h"

ProgressBar::ProgressBar(const float size, const int barWidth) : _barWidth(barWidth), _progress(0.0f), _size(size) {}

std::ostream &operator<<(std::ostream &os, const ProgressBar &bar) {
    os << std::fixed << std::setprecision(2);

    os << "[";

    const int pos = int((float)bar._barWidth * bar._progress);
    for (int i = 0; i < bar._barWidth; i++) {
        if (i < pos)
            os << "=";
        else if (i == pos)
            os << ">";
        else
            os << " ";
    }

    os << "] " << bar._progress * 100.0f << "%\r";
    os.flush();

    return os;
}

void ProgressBar::operator++(int) { _progress += 1.0f / _size; }
