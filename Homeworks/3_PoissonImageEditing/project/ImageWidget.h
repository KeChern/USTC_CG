#pragma once
#include <QWidget>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
class ChildWindow;
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

enum DrawStatus
{
	kChoose,
	kPaste,
	kNone
};

typedef Eigen::Matrix<Eigen::Vector3f, Eigen::Dynamic, Eigen::Dynamic> Mat3Vector;

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(ChildWindow* relatewindow);
	~ImageWidget(void);

	int ImageWidth();											// Width of image
	int ImageHeight();											// Height of image
	void set_draw_status_to_choose();
	void set_draw_status_to_paste();
	QImage* image();
	void set_source_window(ChildWindow* childwindow);

protected:
	void paintEvent(QPaintEvent* paintevent);
	void mousePressEvent(QMouseEvent* mouseevent);
	void mouseMoveEvent(QMouseEvent* mouseevent);
	void mouseReleaseEvent(QMouseEvent* mouseevent);

public slots:
	// File IO
	void Open(QString filename);								// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal = false, bool vertical = true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin

public:
	QPoint point_start_;					// Left top point of rectangle region
	QPoint point_end_;						// Right bottom point of rectangle region

private:
	QImage* image_;						// image 
	QImage* image_backup_;

	// Pointer of child window
	ChildWindow* source_window_;				// Source child window

	// Signs
	DrawStatus draw_status_;					// Enum type of draw status
	bool is_choosing_;
	bool is_pasting_;


public://以下为自行添加的元素
	std::vector<QPoint>poly_inter_vertex_;
	QPolygon poly;
	int x_max;
	int x_min;
	int y_max;
	int y_min;
	int enlarged_width;
	int enlarged_height;
	int Area;
	Mat3Vector Gradient_X;
	Mat3Vector Gradient_Y;
	Mat3Vector get_gradient_x(Mat3Vector Mat);
	Mat3Vector get_gradient_y(Mat3Vector Mat);
	Mat3Vector Transform(QImage* image, int begin_w, int end_w, int begin_h, int end_h);
	Eigen::SparseMatrix<float> CoMatrix;
	int controlcolor(float a);
};

