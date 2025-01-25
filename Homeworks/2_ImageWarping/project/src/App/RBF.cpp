#include "RBF.h"
#include<iostream>
using namespace Eigen;
using namespace std;

RBF::RBF()
{
}

RBF::~RBF()
{
}

float RBF::R(QPoint pi, QPoint pj)
{
	float d = 100;
	return static_cast<float>(pow(pow(dist(pi, pj), 2) + pow(d, 2), mu));
}

void RBF::Compute()
{
	std::vector<QPoint>p_list_;
	std::vector<QPoint>q_list_;
	
	if (denoise_status_)
	{
		p_list_ = end_list_;
		q_list_ = start_list_;
	}
	else
	{
		p_list_ = start_list_;
		q_list_ = end_list_;
	}

	int n = start_list_.size();
	MatrixXf Coeff_Matrix = MatrixXf::Zero(n + 3, n + 3);
	MatrixXf q_Matrix = MatrixXf::Zero(n + 3, 2);
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			Coeff_Matrix(i, j) = R(p_list_[i], p_list_[j]);
		}
	}
	for (int i = 0; i < n; i++)
	{
		Coeff_Matrix(i, n) = p_list_[i].x();
		Coeff_Matrix(i, n + 1) = p_list_[i].y();
		Coeff_Matrix(i, n + 2) = 1;

		Coeff_Matrix(n, i) = p_list_[i].x();
		Coeff_Matrix(n + 1, i) = p_list_[i].y();
		Coeff_Matrix(n + 2, i) = 1;

		q_Matrix(i, 0) = q_list_[i].x();
		q_Matrix(i, 1) = q_list_[i].y();
	}
	MatrixXf result_Matrix = Coeff_Matrix.colPivHouseholderQr().solve(q_Matrix);

	MatrixXf Coeff = MatrixXf::Zero(1, n + 3);
	MatrixXf goal_point;
	QImage image = *ptr_image_;

	// init the image
	for (int i = 0; i < image.width(); i++)
	{
		for (int j = 0; j < image.height(); j++)
		{
			image.setPixel(i, j, qRgb(255, 255, 255));
		}
	}

	for (int i = 0; i < image.width(); i++)
	{
		for (int j = 0; j < image.height(); j++)
		{
			for (int k = 0; k < n; k++)
			{
				Coeff(0, k) = R(QPoint(i, j), p_list_[k]);
			}
			Coeff(0, n) = i;
			Coeff(0, n + 1) = j;
			Coeff(0, n + 2) = 1;

			goal_point = Coeff * result_Matrix;
			int X = static_cast<int>(goal_point(0, 0));
			int Y = static_cast<int>(goal_point(0, 1));

			if (X >= 0 && X < image.width() && Y >= 0 && Y < image.height())
			{
				if (denoise_status_)
				{
					QRgb color = ptr_image_->pixel(X, Y);
					image.setPixel(i, j, color);
				}
				else
				{
					QRgb color = ptr_image_->pixel(i, j);
					image.setPixel(X, Y, color);
				}
			}
		}
	}
	*ptr_image_ = image;
}