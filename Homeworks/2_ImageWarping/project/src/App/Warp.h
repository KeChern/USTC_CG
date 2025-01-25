#pragma once
#include <QWidget>
#include<Eigen/Dense>
#include <math.h>

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class Warp
{
public:
	Warp();
	virtual ~Warp();
	virtual void Compute() = 0;

	void SetImage(QImage* image);
	void SetStartPoints(std::vector<QPoint>s);
	void SetEndPoints(std::vector<QPoint>e);
	void setDeNoise(bool v);

	QImage* Image();
	float dist(QPoint pi, QPoint pj);

public:
	enum Type
	{
		kDefault = 0,
		kIDW = 1,
		kRBF = 2,
	};

protected:
	QImage* ptr_image_;

	std::vector<QPoint>start_list_;   //起点集
	std::vector<QPoint>end_list_;     //终点集

	bool denoise_status_ = false;
};