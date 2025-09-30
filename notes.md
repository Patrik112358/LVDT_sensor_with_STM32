# Performance comparison
| ConfigPreset   | Optimization level | Has debug symbols? | Buffers skipped per second | RAM used (B/%)   | FLASH used (B/%) |
|----------------|--------------------|--------------------|----------------------------|------------------|------------------|
| Debug          | -O0                | -g3                | ~55                        | 17512 B / 13.36% | 59672 B / 11.38% |
| Release        | -O3                | -g0                | ~17.54                     | 17512 B / 13.36% | 43376 B /  8.27% |
| MinSizeRel     | -Os                | xxx                | ~17.4                      | 17512 B / 13.36% | 43376 B /  8.27% |
| RelWithDebInfo | -O2                | -g                 | < 1.0 / < 3                | 17512 B / 13.36% | 43376 B /  8.27% |
