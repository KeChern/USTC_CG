#include "Edge.h"

Edge::Edge()
{

}

Edge::Edge(QPoint A,QPoint B)
{
	float x1 = static_cast<float>(A.x());
	float x2 = static_cast<float>(B.x());
	float y1 = static_cast<float>(A.y());
	float y2 = static_cast<float>(B.y());

	Set_dx((x1 - x2) / (y1 - y2));

	bool status = y1 < y2 ? true : false;
	if (status)
	{
		Set_x(x1);
		Set_ymax(y2);
		Set_ymin(y1);
	}
	else
	{
		Set_x(x2);
		Set_ymax(y1);
		Set_ymin(y2);
	}
}

Edge::~Edge()
{

}

void Edge::Set_x(float x)
{
	m_x = x;
}

void Edge::Set_dx(float dx)
{
	m_dx = dx;
}

void Edge::Set_ymax(float ymax)
{
	m_ymax = ymax;
}

void Edge::Set_ymin(float ymin)
{
	m_ymin = ymin;
}

float Edge::Get_x()
{
	return m_x;
}

float Edge::Get_dx()
{
	return m_dx;
}

float Edge::Get_ymax()
{
	return m_ymax;
}

float Edge::Get_ymin()
{
	return m_ymin;
}