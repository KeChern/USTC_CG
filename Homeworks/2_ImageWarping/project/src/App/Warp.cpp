#include "Warp.h"

Warp::Warp()
{
	ptr_image_ = new QImage();
}

Warp::~Warp()
{
}

void Warp::SetImage(QImage* image)
{
	*(ptr_image_) = *(image);
}

void Warp::SetStartPoints(std::vector<QPoint>s)
{
	start_list_ = s;
}

void Warp::setDeNoise(bool v)
{
	denoise_status_ = v;
}

void Warp::SetEndPoints(std::vector<QPoint>e)
{
	end_list_ = e;
}

QImage* Warp::Image()
{
	return ptr_image_;
}

float Warp::dist(QPoint pi, QPoint pj)
{
	float d2 = pow(pi.x() - pj.x(), 2) + pow(pi.y() - pj.y(), 2);
	return sqrt(d2);
}