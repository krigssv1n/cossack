/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Collonse_ttf;
    const int            Collonse_ttfSize = 51360;

    extern const char*   CollonseBoldBold_ttf;
    const int            CollonseBoldBold_ttfSize = 46764;

    extern const char*   CollonseHollow_ttf;
    const int            CollonseHollow_ttfSize = 75792;

    extern const char*   background_jpg;
    const int            background_jpgSize = 120937;

    extern const char*   handle_png;
    const int            handle_pngSize = 23932;

    extern const char*   logo_png;
    const int            logo_pngSize = 62997;

    extern const char*   scale_png;
    const int            scale_pngSize = 10966;

    extern const char*   screw_png;
    const int            screw_pngSize = 9212;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 8;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
