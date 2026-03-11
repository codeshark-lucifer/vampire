#pragma once

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))