#include "util.h"

bool file_exists(const char *filename) {
    struct stat buffer;
    return stat(filename, &buffer) == 0;
}
