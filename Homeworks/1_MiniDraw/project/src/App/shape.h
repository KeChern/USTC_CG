#pragma once

#include <QtGui>

class Shape
{
public:
	Shape();
	virtual ~Shape();
	virtual void Draw(QPainter& paint) = 0;
	void set_start(QPoint s);
	void set_end(QPoint e);

	void set_is_filled(bool a);
	void set_color(QColor c);
	void set_thickness(int t);
	void Push(QPoint s);

	bool is_filled;
	QColor color;
	int thickness;

public:
	enum Type
	{
		kDefault = 0,
		kLine = 1,
		kTria = 2,
		kRect = 3,
		kCirc = 4,
		kElli = 5,
		kPoly = 6,
		kFree = 7,
	};

protected:
	QPoint start;
	QPoint end;
	std::vector<QPoint> Point_list_;
};

