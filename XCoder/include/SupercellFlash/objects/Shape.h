#pragma once

#include "DisplayObject.h"
#include "ShapeDrawBitmapCommand.h"

#include <vector>
#include <cstdint>
#include <memory>

using namespace std;

namespace sc
{
	class Shape : public DisplayObject
	{
	public:
		vector<pShapeDrawBitmapCommand> commands;

	public:
		Shape* load(SupercellSWF* swf, uint8_t tag);
		void save(SupercellSWF* swf);

	private:
		uint8_t getTag();
	};

	typedef std::shared_ptr<Shape> pShape;
}
