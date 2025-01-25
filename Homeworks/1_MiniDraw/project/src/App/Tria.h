#pragma once

#include "Shape.h"

class Tria : public Shape {
public:
	Tria();
	~Tria();
	
	void Draw(QPainter& painter);
};
