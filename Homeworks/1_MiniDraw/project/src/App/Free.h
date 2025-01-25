#pragma once

#include "Shape.h"

class Free : public Shape {
public:
	Free();
	~Free();

	void Draw(QPainter& painter);
	

};
