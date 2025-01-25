#pragma once
#include <QWidget>
#include <qpainter.h>

#include "Warp.h"
#include "IDW.h"
#include "RBF.h"

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(void);
	~ImageWidget(void);

protected:
	void paintEvent(QPaintEvent *paintevent);                   // Qt ���еĻ��ƶ�ֻ���ڴ˺��������

public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal=false, bool vertical=true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin

	void setWarpStatus();
	void setDeNoise();
	void IDW_ImageWarp();
	void RBF_ImageWarp();

public:
	void mousePressEvent(QMouseEvent* event);   // ��������Ӧ����
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event); // ����ͷ���Ӧ���� 

	void test(int i, int j, int r, int t);
private:
	QPoint bias;

	bool warp_status_ = false;
	bool denoise_status_ = false;

	QImage		*ptr_image_;				// image 
	QImage		*ptr_image_backup_;

	QPoint start_point_; // ��ǰͼԪ����ʼ��
	QPoint end_point_;    // ��ǰͼԪ����ֹ��

	std::vector<QPoint>start_list_;   //��㼯
	std::vector<QPoint>end_list_;     //�յ㼯

	Warp* warp_;
};

