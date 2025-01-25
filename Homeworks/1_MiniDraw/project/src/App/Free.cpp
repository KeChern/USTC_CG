#include "Free.h"

Free::Free()
{
}

Free::~Free()
{
}

void Free::Draw(QPainter& painter)
{
	for (int i = 0; i < Point_list_.size() - 1; i++)
	{
		painter.drawLine(Point_list_[i], Point_list_[i + 1]);
	}
}

