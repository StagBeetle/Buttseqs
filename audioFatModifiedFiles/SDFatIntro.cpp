#include "SDFatIntro.h"
#include "AudioFat.h"

#if SD_FAT_TYPE == 0
SdFat SD;
typedef File file_t;
#elif SD_FAT_TYPE == 1
SdFat32 SD;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
SdExFat SD;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
SdFs SD;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE