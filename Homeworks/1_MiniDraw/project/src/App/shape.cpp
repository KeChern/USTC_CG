#include "Shape.h"

Shape::Shape()
{
}

Shape::~Shape()
{
}

void Shape::set_start(QPoint s)
{
	start = s;
}

void Shape::set_end(QPoint e)
{
	end = e;
}

void Shape::set_is_filled(bool a)
{
	is_filled = a;
}

void Shape::set_color(QColor c)
{
	color = c;
}

void Shape::set_thickness(int t)
{
	thickness = t;
}

void Shape::Push(QPoint s)
{
	Point_list_.push_back(s);
}