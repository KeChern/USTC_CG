#pragma once
#include <QWidget>

class Edge
{
public:
	Edge();
	~Edge();
	Edge(QPoint A, QPoint B);

	void Set_x(float x);
	void Set_dx(float dx);
	void Set_ymax(float ymax);
	void Set_ymin(float ymin);

	float Get_x();
	float Get_dx();
	float Get_ymax();
	float Get_ymin();

private:
	float m_x;		//与当前扫描线的交点
	float m_dx;		//斜率的倒数
	float m_ymax;	//最高点的纵坐标
	float m_ymin;	//最低点的纵坐标
};

