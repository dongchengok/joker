#pragma once

#define JCLAMP(v, lo, hi) ((v) < (lo) ? (v) : (v) > (hi) ? hi : v)