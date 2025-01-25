#pragma once

#include "Warp.h"

class RBF : public Warp {
public:
	RBF();
	~RBF();
	
	float R(QPoint pi, QPoint pj);

	void Compute();

private:
	float mu = -1;
};
