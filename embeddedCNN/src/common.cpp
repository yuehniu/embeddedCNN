// Shape of feature map in each layer
extern const int SHAPE[18] = {224, 224, 112, 112, 56, 56, 56, 28, 28, 28, 14, 14, 14, 1, 1, 1, 1, 1};
// Channel number in each layer
extern const int CHNEL[18] = {64, 64, 128, 128, 256, 256, 256, 512, 512, 512, 512, 512, 512, 256, 4096, 256, 4096, 1000};
// Whether pooling
extern const bool POOL[13] = {false, true, false, true, false, false, true, false, false, true, false, false, true};
// Kernel size in each conv layer
extern const int KERNL[13] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
// Stride in each conv layer
extern const int STRID[13] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
