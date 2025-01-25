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
	float m_x;		//�뵱ǰɨ���ߵĽ���
	float m_dx;		//б�ʵĵ���
	float m_ymax;	//��ߵ��������
	float m_ymin;	//��͵��������
};

