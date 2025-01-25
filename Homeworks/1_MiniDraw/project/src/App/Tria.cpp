#include "Tria.h"

Tria::Tria()
{
}

Tria::~Tria()
{
}

void Tria::Draw(QPainter& painter)
{
	QPoint points[3];
	points[0] = QPoint((start.x() + end.x()) / 2, start.y());
	points[1] = QPoint(start.x(), end.y());
	points[2] = QPoint(end.x(), end.y());

	painter.drawPolygon(points,3);
}
