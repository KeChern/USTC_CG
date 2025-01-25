#include "IDW.h"
#include<iostream>

using namespace Eigen;
using namespace std;

IDW::IDW()
{
}

IDW::~IDW()
{
}

float IDW::sigma(QPoint x, QPoint y)
{
	float f = pow(dist(x, y), mu);
	if (f < 1e-6)return 0;
	else
	{
		return 1 / f;
	}
	
}

float IDW::w(QPoint x,int k)
{
	float sum = 0;
	for (int i = 0; i < start_list_.size(); i++)
	{
		sum += sigma(x, start_list_[i]);
	}
	return sigma(x, start_list_[k]) / sum;
}

void IDW::Add_c(int i, int j, float co)
{
	Coeff_Matrix(i, j) += co;
}

void IDW::Add_b(int i, int j, float co)
{
	b_Matrix(i, j) += co;
}

void IDW::Compute()
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
	Coeff_Matrix = MatrixXf::Zero(2 * n, 2 * n);
	b_Matrix = MatrixXf::Zero(2 * n, 2);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (i != j)
			{
				float s = w(start_list_[j], i);
				Add_c(2 * i, 2 * i, s * (p_list_[j].x() - p_list_[i].x()) * (p_list_[j].x() - p_list_[i].x()));
				Add_c(2 * i, 2 * i + 1, s * (p_list_[j].y() - p_list_[i].y()) * (p_list_[j].x() - p_list_[i].x()));
				Add_b(2 * i, 0, s * (q_list_[j].x() - q_list_[i].x()) * (p_list_[j].x() - p_list_[i].x()));
				Add_b(2 * i, 1, s * (q_list_[j].y() - q_list_[i].y()) * (p_list_[j].x() - p_list_[i].x()));

				Add_c(2 * i + 1, 2 * i, s * (p_list_[j].x() - p_list_[i].x()) * (p_list_[j].y() - p_list_[i].y()));
				Add_c(2 * i + 1, 2 * i + 1, s * (p_list_[j].y() - p_list_[i].y()) * (p_list_[j].y() - p_list_[i].y()));
				Add_b(2 * i + 1, 0, s * (q_list_[j].x() - q_list_[i].x()) * (p_list_[j].y() - p_list_[i].y()));
				Add_b(2 * i + 1, 1, s * (q_list_[j].y() - q_list_[i].y()) * (p_list_[j].y() - p_list_[i].y()));
			}
		}
	}
	MatrixXf result_Matrix = Coeff_Matrix.colPivHouseholderQr().solve(b_Matrix);

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
			float X_f = 0;
			float Y_f = 0;
			for (int k = 0; k < n; k++)
			{
				X_f += w(QPoint(i, j), k) * (result_Matrix(2 * k, 0) * (i - p_list_[k].x()) + result_Matrix(2 * k + 1, 0) * (j - p_list_[k].y()) + q_list_[k].x());
				Y_f += w(QPoint(i, j), k) * (result_Matrix(2 * k, 1) * (i - p_list_[k].x()) + result_Matrix(2 * k + 1, 1) * (j - p_list_[k].y()) + q_list_[k].y());
			}
			int X = static_cast<int>(X_f);
			int Y = static_cast<int>(Y_f);

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