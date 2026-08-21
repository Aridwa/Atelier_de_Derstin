/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   app_icon_ico;
    const int            app_icon_icoSize = 4286;

    extern const char*   app_icon_png;
    const int            app_icon_pngSize = 63409;

    extern const char*   Atelier_de_Derstin_ico;
    const int            Atelier_de_Derstin_icoSize = 4286;

    extern const char*   Atelier_de_Derstin_png;
    const int            Atelier_de_Derstin_pngSize = 68287;

    extern const char*   mabinogi_preset_en_txt;
    const int            mabinogi_preset_en_txtSize = 2532;

    extern const char*   mabinogi_preset_ja_txt;
    const int            mabinogi_preset_ja_txtSize = 1255;

    extern const char*   mabinogi_preset_ko_txt;
    const int            mabinogi_preset_ko_txtSize = 3282;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 7;

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
