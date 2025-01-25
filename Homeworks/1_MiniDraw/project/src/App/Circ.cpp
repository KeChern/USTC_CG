#include "Circ.h"
#include<math.h>

Circ::Circ()
{
}

Circ::~Circ()
{
}

void Circ::Draw(QPainter& painter)
{
	
	int R = sqrt((end.x() - start.x()) * (end.x() - start.x()) + (end.y() - start.y()) * (end.y() - start.y()));
	painter.drawEllipse(start, R, R);
}
