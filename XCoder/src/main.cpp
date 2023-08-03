#include <iostream>
#include "SupercellFlash.h"

#include <qimage.h>
#include <qpainter.h>
#include <QPainterPath>
#include "opencv2/opencv.hpp"

#include "tl/expected.hpp"
#include <optional>

using namespace std;
using namespace sc;
using namespace tl;

void cropPathRegion(const QPainterPath& path, const std::vector<uint8_t>& textureData, int width, int height, QImage& outputImage) {
	// Create a QImage from the textureData (assuming RGBA format)
	QImage textureImage(textureData.data(), width, height, QImage::Format_ARGB32);
	textureImage = textureImage.rgbSwapped(); // Swap Blue and Red channels to get RGBA

	// Create a QPainter to draw on the outputImage
	QPainter painter(&outputImage);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setClipPath(path);

	// Draw the textureImage onto the outputImage using the path as a clip
	painter.drawImage(0, 0, textureImage);
}

void saveToPng(const QString& filePath, const QImage& image) {
	image.save(filePath, "PNG");
}


//void visitFrame(SupercellSWF& swf, sc::pMovieClip exportMovieClip, int frameIndex) {
//	for (auto instance : exportMovieClip->instances) {
//		const int instanceId = instance->id;
//		optional<sc::pShape> maybeShape = nullopt;
//		for (auto shape : swf.shapes) {
//			if (instanceId == shape->id()) {
//				maybeShape = shape;
//				break;
//			}
//		}
//		if (!maybeShape.has_value()) {
//			break;
//		}
//		auto shape = maybeShape.value();
//		QPainterPath path;
//		for (auto command : shape->commands) {
//			for (auto vertex : command->vertices) {
//				path.lineTo(vertex->x(), vertex->y());
//			}
//		}
//		auto rect = path.boundingRect();
//
//		if (instance->name != "bounds") {
//			continue;
//		}
//		cropPathRegion(path, textureData, width, height, outputImage);
//	}
//}

void saveToTexturePng(sc::pSWFTexture& texture, string filePath, int i) {
	fs::path imagePath = fs::path(filePath).parent_path() / fs::path(filePath).stem().concat(string("_") + to_string(i) + ".png");
	texture->textureEncoding(sc::SWFTexture::TextureEncoding::Raw);
	texture->pixelFormat(sc::SWFTexture::PixelFormat::RGBA8);
	texture->linear(true);
	cv::Mat image = cv::Mat(cv::Size(texture->width(), texture->height()), CV_8UC4, texture->textureData.data(), cv::Mat::AUTO_STEP);
	cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
	cv::imwrite(imagePath.string(), image);
}

void saveShape(sc::pSWFTexture& texture, sc::pShape& shape, string filePath) {
	QPainterPath path;
	for (auto command : shape->commands) {
		for (auto vertex : command->vertices) {
			path.lineTo(vertex->x(), vertex->y());
		}
		path.lineTo(command->vertices[0]->x(), command->vertices[0]->y());
	}
	auto rect = path.boundingRect();
	const int width = rect.width();
	const int height = rect.height();
	QImage croppedImage(width, height, QImage::Format_RGBA8888);
	croppedImage.fill(Qt::transparent);
	cropPathRegion(path, texture->textureData, width, height, croppedImage);
	fs::path imagePath = fs::path(filePath).parent_path() / fs::path(filePath).stem().concat(string("_shape_") + to_string(shape->id()) + ".png");
	saveToPng(QString(imagePath.generic_string().c_str()), croppedImage);
}


std::vector<QVector2D> generateChildrensPointF(SupercellSWF swf, pMovieClip movieClip, Matrix2D* matrixIn)
{
	std::vector<pShapeDrawBitmapCommandVertex> A;

	for (int i = 0; i < movieClip->frameElements.size(); i++)
	{
		const pMovieClipFrameElement frameElement = movieClip->frameElements[i];
		optional<uint16_t> maybeShapeIndex = nullopt;
		for (auto shape : swf.shapes) {
			if (shape->id() == frameElement->instanceIndex) {
				maybeShapeIndex = make_optional(shape->id());
			}
		}
		if (maybeShapeIndex.has_value())
		{
			pShape shapeToRender = swf.shapes[maybeShapeIndex.value()];

			Matrix2D* childrenMatrixData = nullptr;
			if (frameElement->colorTransformIndex != 0xFFFF) {
				swf.matrixBanks[movieClip->matrixBankIndex()]->getMatrixIndex(childrenMatrixData, frameElement->matrixIndex);
			}
			Matrix2D* matrixData;
			if (childrenMatrixData != nullptr) {
				matrixData = childrenMatrixData;
			}

			if (matrixIn != nullptr)
			{
				//matrixData.Multiply(matrixIn);
			}

			if (matrixData != nullptr)
			{
				for (pShapeDrawBitmapCommand command : shapeToRender->commands)
				{
					auto size = command->vertices.size();
					std::vector<pShapeDrawBitmapCommandVertex> newXY;
					newXY.reserve(size);
					//PointF[] newXY = new PointF[chunk.XY.Length];

					for (int xyIdx = 0; xyIdx < newXY.size(); xyIdx++)
					{
						float xNew = matrixData->tx + matrixData->a * command->vertices[xyIdx]->x() + matrixData->b * command->vertices[xyIdx]->y();
						float yNew = matrixData->ty + matrixData->b * command->vertices[xyIdx]->x() + matrixData->c * command->vertices[xyIdx]->y();

						auto newCommand = pShapeDrawBitmapCommandVertex(new ShapeDrawBitmapCommandVertex());
						newCommand->x(xNew);
						newCommand->y(yNew);
						newXY[xyIdx] = newCommand;
					}
					for (auto newXY : newXY) {
						A.push_back(newXY);
					}
				}
			}
			else
			{
				PointF[] pointsXY = shapeToRender.Children.SelectMany(chunk = > ((ShapeChunk)chunk).XY).ToArray();
				A.AddRange(pointsXY);
			}
		}
		else
		{
			int movieClipIndex = _scFile.GetMovieClips().FindIndex(s = > s.Id == timelineChildrenId[timelineArray[(i * 3)]]);

			if (movieClipIndex != -1)
			{
				List<PointF> templist = ((MovieClip)_scFile.GetMovieClips()[movieClipIndex]).generateChildrensPointF(matrixIn, token);

				if (templist == null || token.IsCancellationRequested)
					return null;

				A.AddRange(templist);
			}
			else if (_scFile.getTextFields().FindIndex(t = > t.Id == timelineChildrenId[timelineArray[(i * 3)]]) != -1)
			{
				// implement
			}
			else
			{
				throw new Exception($"Unknown type of children with id {timelineChildrenId[timelineArray[(i * 3)]]} for movieclip id {this.Id}");
			}
		}
	}

	return A;
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
			saveToTexturePng(texture, filePath, i);
			swf.movieClipModifiers
				for (auto shape : swf.shapes) {
					saveShape(texture, shape, filePath);
				}
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