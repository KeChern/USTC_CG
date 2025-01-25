#pragma once

#include "Warp.h"



class IDW : public Warp {
public:
	IDW();
	~IDW();
	

	float sigma(QPoint x, QPoint y);
	float w(QPoint x, int k);
	void Add_c(int i, int j, float co);
	void Add_b(int i, int j, float co);

	void Compute();
	
private:
	float mu = 2;

	Eigen::MatrixXf Coeff_Matrix;
	Eigen::MatrixXf b_Matrix;
};
