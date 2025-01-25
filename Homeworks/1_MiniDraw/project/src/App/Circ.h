#pragma once

#include "Shape.h"

class Circ : public Shape {
public:
	Circ();
	~Circ();

	void Draw(QPainter& painter);
};
