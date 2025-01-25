#include "ScanLine.h"

ScanLine::ScanLine()
{
	vertex_list_.push_back(QPoint(2, 2));
	vertex_list_.push_back(QPoint(5, 1));
	vertex_list_.push_back(QPoint(11, 3));
	vertex_list_.push_back(QPoint(11, 8));
	vertex_list_.push_back(QPoint(5, 5));
	vertex_list_.push_back(QPoint(2, 7));
	poly_y_max = Poly_y_max();
	poly_y_min = Poly_y_min();
}
ScanLine::~ScanLine()
{

}

void ScanLine::compute()
{
	NET.resize(poly_y_max - poly_y_min + 1);
	for (int i = 0; i < vertex_list_.size() - 1; i++)
	{
		Edge temp(vertex_list_[i], vertex_list_[i + 1]);
		//static_cast<int>(temp.m_ymin)
	}

}

int ScanLine::Poly_y_min()
{
	int min = 0;
	for (int i = 0; i < vertex_list_.size(); i++)
	{
		min = min > vertex_list_[i].y() ? vertex_list_[i].y() : min;
	}
	return min;
}

int ScanLine::Poly_y_max()
{
	int max = 0;
	for (int i = 0; i < vertex_list_.size(); i++)
	{
		max = max < vertex_list_[i].y() ? vertex_list_[i].y() : max;
	}
	return max;
}