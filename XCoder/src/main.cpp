#include <iostream>
#include "SupercellFlash.h"

#include <QPainterPath>
#include "opencv2/opencv.hpp"

#include "tl/expected.hpp"
#include <optional>

using namespace std;
using namespace sc;
using namespace tl;

void visitFrame(SupercellSWF& swf, sc::pMovieClip exportMovieClip, int frameIndex) {
	for (auto instance : exportMovieClip->instances) {
		const int instanceId = instance->id;
		optional<sc::pShape> maybeShape = nullopt;
		for (auto shape : swf.shapes) {
			if (instanceId == shape->id()) {
				maybeShape = shape;
				break;
			}
		}
		if (!maybeShape.has_value()) {
			break;
		}
		auto shape = maybeShape.value();
		QPainterPath path;
		for (auto command : shape->commands) {
			for (auto vertex : command->vertices) {
				path.lineTo(vertex->x(), vertex->y());
			}
		}
		auto rect = path.boundingRect();

		if (instance->name != "bounds") {
			continue;
		}
	}
}

int main(int argc, char* argv[]) {
	// The first argument (argv[0]) is the program name
	// The following arguments (argv[1] onwards) are the file paths

	// Check if any file paths were provided
	if (argc > 1) {
		// Iterate over the file paths
		for (int i = 1; i < argc; ++i) {
			const string filePath = argv[i];
			if (!filesystem::exists(filePath)) {
				cout << "File path does not exist: " << filePath << '\n';
				return 0;
			}
			SupercellSWF swf;
			try {
				swf.load(filePath);
			}
			catch (const std::exception& err) {
				cout << "Failed to load file. Error: " << err.what() << '\n';
			}
			sc::pSWFTexture texture = swf.textures[0];
			if (texture == nullptr) break;
			fs::path imagePath = fs::path(filePath).parent_path() / fs::path(filePath).stem().concat(string("_") + to_string(i) + ".png");
			texture->textureEncoding(sc::SWFTexture::TextureEncoding::Raw);
			texture->pixelFormat(sc::SWFTexture::PixelFormat::RGBA8);
			texture->linear(true);
			cv::Mat image = cv::Mat(cv::Size(texture->width(), texture->height()), CV_8UC4, texture->textureData.data(), cv::Mat::AUTO_STEP);
			cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
			cv::imwrite(imagePath.string(), image);
			/*cout << "File has: " << '\n';
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

				for (int frameIndex = 0; frameIndex < exportMovieClip->frames.size(); ++frameIndex) {
					visitFrame(swf, exportMovieClip, frameIndex);
				}*/
			//}
		}
	}
	else {
		std::cout << "No file paths provided." << '\n';
	}

	return 0;
}