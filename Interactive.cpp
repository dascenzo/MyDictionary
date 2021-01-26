#include "Interactive.h"
#include <unistd.h>

bool isInteractive(FILE* file) {
  return isatty(fileno(file));
}
