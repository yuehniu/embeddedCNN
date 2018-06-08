// Shape of feature map in each layer
extern const int SHAPE[] = {224, 224, 112, 112, 56, 56, 56, 28, 28, 28, 14, 14, 14, 1, 1, 1, 1, 1};
// Channel number in each layer
extern const int CHNEL[] = {64, 64, 128, 128, 256, 256, 256, 512, 512, 512, 512, 512, 512, 256, 4096, 256, 4096, 1000};
// Kernel size in each conv layer
extern const int KERNL[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
//Stride in each conv layer
extern const int STRID[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};