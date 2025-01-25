#include "Poly.h"

Poly::Poly()
{
}

Poly::~Poly()
{
}

void Poly::Draw(QPainter& painter)
{
	for (int i = 0; i < Point_list_.size() - 1; i++)
	{
		painter.drawLine(Point_list_[i], Point_list_[i + 1]);
	}
	painter.drawLine(Point_list_.back(), end);

	// Моід
	if (start == end)
	{
		int N = Point_list_.size();
		QPoint* p = new QPoint[N + 1];
		for (int i = 0; i < N; i++)
		{
			p[i] = Point_list_[i];
		}
		p[N] = end;
		painter.drawPolygon(p, N + 1);
	}
}
