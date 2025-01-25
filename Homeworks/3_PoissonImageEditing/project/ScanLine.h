#pragma once
#include <QWidget>
#include<vector>
#include "Edge.h"

class ScanLine
{
public:
	ScanLine();
	~ScanLine();

	void compute();
	int Poly_y_min();
	int Poly_y_max();

private:
	std::vector<QPoint> vertex_list_;
	std::vector<std::vector<Edge>> NET;
	int poly_y_max;
	int poly_y_min;
};

