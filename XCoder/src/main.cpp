#include <iostream>
#include "SupercellFlash.h"
#include <qimage.h>
#include <qpainter.h>
#include <QPainterPath>
#include "opencv2/opencv.hpp"
#include "wgpu.h"
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


std::vector<pShapeDrawBitmapCommandVertex> generateChildrensPointF(SupercellSWF swf, pMovieClip movieClip, Matrix2D* matrixIn)
{
	std::vector<pShapeDrawBitmapCommandVertex> A;

	for (int i = 0; i < movieClip->frameElements.size(); i++)
	{
		const pMovieClipFrameElement frameElement = movieClip->frameElements[i];
		optional<uint16_t> maybeShapeIndex = nullopt;
		for (auto shape : swf.shapes) {
			if (shape->id() == frameElement->instanceIndex) {
				maybeShapeIndex = make_optional(shape->id());
				break;
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
			else {
				matrixData = new Matrix2D();
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
					std::vector<ShapeDrawBitmapCommandVertex*> newXY(size);

					for (int xyIdx = 0; xyIdx < newXY.size(); xyIdx++)
					{
						float xNew = matrixData->tx + matrixData->a * command->vertices[xyIdx]->x() + matrixData->c * command->vertices[xyIdx]->y();
						float yNew = matrixData->ty + matrixData->b * command->vertices[xyIdx]->x() + matrixData->d * command->vertices[xyIdx]->y();

						auto newCommand = new ShapeDrawBitmapCommandVertex();
						newCommand->x(xNew);
						newCommand->y(yNew);
						newXY[xyIdx] = newCommand; // ?????????????????????????
					}
					for (auto newXY : newXY) {
						A.push_back(pShapeDrawBitmapCommandVertex(newXY));
					}
				}
			}
			else
			{
				for (auto command : shapeToRender->commands) {
					for (auto vertex : command->vertices) {
						A.push_back(vertex);
					}
				}
			}
		}
		else
		{
			optional<int> maybeMovieClipIndex = nullopt;
			for (auto movieClip : swf.movieClips) {
				if (movieClip->id() == movieClip->frameElements[i]->instanceIndex) {
					maybeMovieClipIndex = make_optional(movieClip->id());
					break;
				}
			}
			if (maybeMovieClipIndex.has_value())
			{
				std::vector<pShapeDrawBitmapCommandVertex> templist = generateChildrensPointF(swf, swf.movieClips[maybeMovieClipIndex.value()], matrixIn);

				for (auto templist : templist) {
					A.push_back(templist);
				}
			}
			// else if (_scFile.getTextFields().FindIndex(t = > t.Id == timelineChildrenId[timelineArray[(i * 3)]]) != -1)
			// {
			// 	// implement
			// }
			// else
			// {
			// 	throw new Exception($"Unknown type of children with id {timelineChildrenId[timelineArray[(i * 3)]]} for movieclip id {this.Id}");
			// }
		}
	}

	return A;
}


//public void setFrame(int frameIndex, RenderingOptions options, Rectangle xyBound)
//{
//	try
//	{
//		if (this._scFile.CurrentRenderingMovieClips.FindIndex(mv => mv.Id == this.Id) == -1)
//			this._scFile.addRenderingItem(this);
//
//		int frameIndextoAdd = 0;
//		int idxToAdd = 0;
//		foreach (MovieClipFrame mvframe in Frames)
//		{
//			if (idxToAdd == frameIndex)
//			{
//				break;
//			}
//
//			frameIndextoAdd += mvframe.Id;
//			idxToAdd++;
//		}
//
//		int timelineIndex = frameIndextoAdd * 3;
//
//		var x = xyBound.X;
//		var y = xyBound.Y;
//
//		var width = xyBound.Width;
//		width = width > 0 ? width : 1;
//
//		var height = xyBound.Height;
//		height = height > 0 ? height : 1;
//
//		var finalShape = new Bitmap(width, height, PixelFormat.Format32bppArgb);
//
//		int frameTimelineCount = _frames[frameIndex].Id;
//
//		for (int i = 0; i < frameTimelineCount; i++)
//			visitFrameTimeline();
//
//		((MovieClipFrame)this.Frames[frameIndex]).setBitmap(finalShape);
//	}
//	catch (Exception ex)
//	{
//		if (ex.GetType() != typeof(OverflowException))
//		{
//			MessageBox.Show(ex.Message + $" in setFrame({frameIndex}) | " + ex.StackTrace);
//		}
//		else
//		{
//			Console.WriteLine(ex.Message + $" in setFrame({frameIndex}) | " + ex.StackTrace);
//			Task.Delay(2000);
//		}
//	}
//}
//		
//
//void visitFrameTimeline()
//{
//	if (timelineChildrenNames[timelineArray[timelineIndex]] == "bounds")
//	{
//		timelineIndex += 3;
//		continue;
//	}
//
//	Bitmap temporaryBitmap = new Bitmap(width, height, PixelFormat.Format32bppArgb);
//	ushort childrenId = timelineChildrenId[timelineArray[timelineIndex]];
//
//	Tuple<Color, byte, Color> colorData = timelineArray[timelineIndex + 2] != 0xFFFF ? this._scFile.getColors(_transformStorageId)[timelineArray[timelineIndex + 2]] : null;
//
//	// SHAPE
//	int shapeIndex = _scFile.GetShapes().FindIndex(s => s.Id == childrenId);
//	if (shapeIndex != -1)
//	{
//		Matrix childrenMatrixData = timelineArray[timelineIndex + 1] != 0xFFFF ? this._scFile.GetMatrixs(_transformStorageId)[timelineArray[timelineIndex + 1]] : null;
//		Matrix matrixData = childrenMatrixData != null ? childrenMatrixData.Clone() : new Matrix();
//
//		if (options.MatrixData != null)
//			matrixData.Multiply(options.MatrixData);
//
//		if (options.editedMatrixPerChildren.ContainsKey(childrenId))
//			matrixData.Multiply(options.editedMatrixPerChildren[childrenId]);
//
//		foreach (ShapeChunk chunk in ((Shape)_scFile.GetShapes()[shapeIndex]).Children)
//		{
//			visitChunk();
//		}
//
//	}
//	else
//	{
//		// Movieclip
//		int movieClipIndex = _scFile.GetMovieClips().FindIndex(s => s.Id == childrenId);
//		if (movieClipIndex != -1)
//		{
//			visitMovieClip();
//		}
//		else
//		{
//			//Textfield
//			int textFieldIndex = _scFile.getTextFields().FindIndex(s => s.Id == childrenId);
//			if (textFieldIndex != -1)
//			{
//				visitTextField();
//			}
//			else
//			{
//				MessageBox.Show($"Movieclip {this.Id} contains children {childrenId} of unknown type.");
//			}
//		}
//	}
//
//	Rectangle originalRectangle = new Rectangle(0, 0, temporaryBitmap.Width, temporaryBitmap.Height);
//
//	if (colorData != null)
//	{
//		Color originalColor = colorData.Item3;
//		Color replacementColor = colorData.Item1;
//
//		byte alphaByte = colorData.Item2;
//
//		ChangeColour(temporaryBitmap, originalColor.A, originalColor.R, originalColor.G, originalColor.B, alphaByte, replacementColor.R, replacementColor.G, replacementColor.B);
//	}
//
//	using (Graphics g = Graphics.FromImage(finalShape))
//	{
//		g.DrawImage(temporaryBitmap, originalRectangle);
//	}
//
//	timelineIndex += 3;
//}
//
//
//void visitChunk() {
//	var texture = (Texture)_scFile.GetTextures()[chunk.GetTextureId()];
//	if (texture != null)
//	{
//		Bitmap bitmap = texture.Bitmap;
//		using (var gpuv = new GraphicsPath())
//		{
//			gpuv.AddPolygon(chunk.UV.ToArray());
//
//			var gxyBound = Rectangle.Round(gpuv.GetBounds());
//
//			int gpuvWidth = gxyBound.Width;
//			int gpuvHeight = gxyBound.Height;
//
//			if (gpuvWidth > 0 && gpuvHeight > 0)
//			{
//
//				var shapeChunk = new Bitmap(gpuvWidth, gpuvHeight);
//
//				var chunkX = gxyBound.X;
//				var chunkY = gxyBound.Y;
//
//				using (var g = Graphics.FromImage(shapeChunk))
//				{
//					gpuv.Transform(new Matrix(1, 0, 0, 1, -chunkX, -chunkY));
//					g.SetClip(gpuv);
//					g.DrawImage(bitmap, -chunkX, -chunkY);
//				}
//
//				GraphicsPath gp = new GraphicsPath();
//				gp.AddPolygon(new[] { new Point(0, 0), new Point(gpuvWidth, 0), new Point(0, gpuvHeight) });
//
//				double[,] matrixArrayUV =
//				{
//					{
//						gpuv.PathPoints[0].X, gpuv.PathPoints[1].X, gpuv.PathPoints[2].X
//					},
//					{
//						gpuv.PathPoints[0].Y, gpuv.PathPoints[1].Y, gpuv.PathPoints[2].Y
//					},
//					{
//						1, 1, 1
//					}
//				};
//
//				PointF[] newXY = new PointF[chunk.XY.Length];
//
//				for (int xyIdx = 0; xyIdx < newXY.Length; xyIdx++)
//				{
//					float xNew = matrixData.Elements[4] + matrixData.Elements[0] * chunk.XY[xyIdx].X + matrixData.Elements[2] * chunk.XY[xyIdx].Y;
//					float yNew = matrixData.Elements[5] + matrixData.Elements[1] * chunk.XY[xyIdx].X + matrixData.Elements[3] * chunk.XY[xyIdx].Y;
//
//					newXY[xyIdx] = new PointF(xNew, yNew);
//				}
//
//				double[,] matrixArrayXY =
//				{
//					{
//						newXY[0].X, newXY[1].X, newXY[2].X
//					},
//					{
//						newXY[0].Y, newXY[1].Y, newXY[2].Y
//					},
//					{
//						1, 1, 1
//					}
//				};
//
//				var matrixUV = Matrix<double>.Build.DenseOfArray(matrixArrayUV);
//				var matrixXY = Matrix<double>.Build.DenseOfArray(matrixArrayXY);
//				var inverseMatrixUV = matrixUV.Inverse();
//				var transformMatrix = matrixXY * inverseMatrixUV;
//				var m = new Matrix((float)transformMatrix[0, 0], (float)transformMatrix[1, 0], (float)transformMatrix[0, 1], (float)transformMatrix[1, 1], (float)transformMatrix[0, 2], (float)transformMatrix[1, 2]);
//
//				//Perform transformations
//				gp.Transform(m);
//
//
//				using (Graphics g = Graphics.FromImage(temporaryBitmap))
//				{
//					//Set origin
//					Matrix originTransform = new Matrix();
//					originTransform.Translate(-x, -y);
//					g.Transform = originTransform;
//
//					g.DrawImage(shapeChunk, gp.PathPoints, gpuv.GetBounds(), GraphicsUnit.Pixel);
//
//					if (options.ViewPolygons)
//					{
//						gpuv.Transform(m);
//						g.DrawPath(new Pen(Color.DeepSkyBlue, 1), gpuv);
//					}
//					g.Flush();
//				}
//
//			}
//
//		}
//	}
//}
//
//void visitMovieClip() {
//	MovieClip extramovieClip = (MovieClip)_scFile.GetMovieClips()[movieClipIndex];
//
//	Matrix newChildrenMatrixData = timelineArray[timelineIndex + 1] != 0xFFFF ? this._scFile.GetMatrixs(_transformStorageId)[timelineArray[timelineIndex + 1]] : null;
//	Matrix newMatrix = newChildrenMatrixData != null ? newChildrenMatrixData.Clone() : new Matrix();
//
//	if (options.MatrixData != null)
//	{
//		newMatrix.Multiply(options.MatrixData);
//	}
//
//	if (options.editedMatrixPerChildren.ContainsKey(extramovieClip.Id))
//	{
//		newMatrix.Multiply(options.editedMatrixPerChildren[extramovieClip.Id]);
//	}
//
//	if ((extramovieClip.getPointFList() == null || extramovieClip.getPointFList().Count == 0) && extramovieClip._animationState == MovieClipState.Stopped)
//		extramovieClip.initPointFList(newMatrix);
//
//	if (_scFile.CurrentRenderingMovieClips.FindIndex(s => s.Id == extramovieClip.Id) == -1)
//		_scFile.CurrentRenderingMovieClips.Add(extramovieClip);
//
//	int extraFrameIndex = extramovieClip._lastPlayedFrame;
//
//	RenderingOptions newOptions = new RenderingOptions() { editedMatrixPerChildren = options.editedMatrixPerChildren, ViewPolygons = options.ViewPolygons };
//	if (newChildrenMatrixData != new Matrix())
//		newOptions.MatrixData = newMatrix; // confirm what to use here
//
//	extramovieClip.setFrame(extraFrameIndex, newOptions, xyBound);
//
//	Bitmap frameData = extramovieClip.getFrame(extraFrameIndex);
//
//	extramovieClip._lastPlayedFrame += 1;
//
//	if (extramovieClip._lastPlayedFrame == extramovieClip.Frames.Count)
//		extramovieClip._lastPlayedFrame = 0;
//
//	using (Graphics g = Graphics.FromImage(temporaryBitmap))
//	{
//		g.DrawImage(frameData, 0, 0);
//		g.Flush();
//	}
//}
//
//void visitTextField() {
//	if (!RenderingOptions.disableTextFieldRendering)
//	{
//		TextField textFieldData = (TextField)_scFile.getTextFields()[textFieldIndex];
//
//		using (Graphics g = Graphics.FromImage(temporaryBitmap))
//		{
//			StringFormat sf = new StringFormat();
//			sf.Alignment = StringAlignment.Center;
//			sf.LineAlignment = StringAlignment.Far;
//
//			InstalledFontCollection fonts = new InstalledFontCollection();
//			FontFamily textFontFamily = fonts.Families.Where(f => f.Name == textFieldData._fontName).FirstOrDefault();
//			if (textFontFamily == null)
//			{
//				MessageBox.Show($"Movieclip childiren {childrenId} textfield font {textFieldData._fontName} not installed");
//				textFontFamily = SystemFonts.DefaultFont.FontFamily;
//			}
//
//			var p = new Pen(textFieldData._fontOutlineColor, 0);
//			p.LineJoin = LineJoin.Round;
//			if (textFieldData._fontOutlineColor != (new Color()))
//			{
//				p.Width = 5;
//			}
//
//			string textRender = string.Empty;
//			if (string.IsNullOrEmpty(textFieldData._textData))
//			{
//				if (string.IsNullOrEmpty(timelineChildrenNames[timelineArray[timelineIndex]]))
//				{
//					textRender = "Text1";
//				}
//				else
//				{
//					textRender = timelineChildrenNames[timelineArray[timelineIndex]];
//				}
//			}
//			else
//			{
//				textRender = textFieldData._textData;
//			}
//
//			GraphicsPath gp = new GraphicsPath();
//			Rectangle r = new Rectangle(0, 0, finalShape.Width, finalShape.Height);
//			gp.AddString(textRender, textFontFamily, (int)FontStyle.Regular, textFieldData._fontSize, r, sf);
//
//			g.SmoothingMode = SmoothingMode.AntiAlias;
//			g.PixelOffsetMode = PixelOffsetMode.HighQuality;
//
//			g.DrawPath(p, gp);
//			g.DrawString(textRender, (new Font(textFontFamily, textFieldData._fontSize, FontStyle.Regular, GraphicsUnit.Pixel)), (new SolidBrush(textFieldData._fontColor)), r, sf);
//
//			gp.Dispose();
//			g.Flush();
//		}
//	}
//}

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
			for (auto movieClip : swf.movieClips) {
				auto points = generateChildrensPointF(swf, movieClip, nullptr);
				for (auto point : points) {
					std::cout << point->x() << ',' << point->y();
				}
			}
			/*for (auto shape : swf.shapes) {
				saveShape(texture, shape, filePath);
			}*/
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