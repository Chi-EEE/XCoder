#include <iostream>
#include "SupercellFlash.h"

#include "tl/expected.hpp"
#include <optional>

using namespace std;
using namespace sc;
using namespace tl;

int main(int argc, char* argv[]) {
    // The first argument (argv[0]) is the program name
    // The following arguments (argv[1] onwards) are the file paths

    // Check if any file paths were provided
    if (argc > 1) {
        // Iterate over the file paths
        for (int i = 1; i < argc; ++i) {
            SupercellSWF swf;
            try {
                swf.load(argv[i]);
            }
            catch (const std::exception& err) {
                cout << "Failed to load file. Error: " << err.what() << '\n';
            }
            cout << "File has: " << '\n';
            cout << swf.exports.size() << " export names" << '\n';
            cout << swf.textures.size() << " textures" << '\n';
            cout << swf.textFields.size() << " textfields" << '\n';
            cout << swf.shapes.size() << " shapes" << '\n';
            cout << swf.movieClips.size() << " movieclips" << '\n';

            for (uint32_t i = 0; swf.exports.size() > i; i++) {
                sc::pExportName exportName = swf.exports[i];
                if (exportName == nullptr) break;
                const int exportId = exportName->id();
                optional<sc::pMovieClip> maybeExportMovieClip = nullopt;
                for (sc::pMovieClip movieClip : swf.movieClips) {
                    if (exportId == movieClip->id()) {
                        maybeExportMovieClip = make_optional(movieClip);
                        break;
                    }
                }
                if (!maybeExportMovieClip.has_value())
                {
                    break;
                }
                auto exportMovieClip = maybeExportMovieClip.value();

                cout << exportId << " movie clip has: " << exportMovieClip->frames.size() << " frames!" << '\n';

        }
    }
    else {
        std::cout << "No file paths provided." << '\n';
    }

    return 0;
}