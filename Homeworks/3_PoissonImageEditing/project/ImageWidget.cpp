#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include "ChildWindow.h"

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow* relatewindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();

	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);

	source_window_ = NULL;
}

ImageWidget::~ImageWidget(void)
{
}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::set_draw_status_to_choose()
{
	draw_status_ = kChoose;
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect(0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_);
	
	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);
	painter.drawPolygon(poly);

	//助教原框架是用矩形
	/*painter.drawRect(point_start_.x(), point_start_.y(),
		point_end_.x() - point_start_.x(), point_end_.y() - point_start_.y());*/

	painter.end();
}


//关键部分
void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
		case kChoose: 
		{
			is_choosing_ = true;
			poly << mouseevent->pos();
			//以下是为了提高效率的预处理
			if (poly.size() >= 3)
			{
				poly_inter_vertex_.clear();
				for (int i = 0; i < ImageWidth(); i++)
				{
					for (int j = 0; j < ImageHeight(); j++)
					{
						if (poly.containsPoint(QPoint(i, j), Qt::OddEvenFill))
						{
							poly_inter_vertex_.push_back(QPoint(i, j));
						}
					}
				}
				x_max = poly_inter_vertex_[0].x();
				x_min = poly_inter_vertex_[0].x();
				y_max = poly_inter_vertex_[0].y();
				y_min = poly_inter_vertex_[0].y();
				for (int i = 1; i < poly_inter_vertex_.size(); i++)
				{
					if (x_max < poly_inter_vertex_[i].x())
					{
						x_max = poly_inter_vertex_[i].x();
					}
					if (x_min > poly_inter_vertex_[i].x())
					{
						x_min = poly_inter_vertex_[i].x();
					}
					if (y_max < poly_inter_vertex_[i].y())
					{
						y_max = poly_inter_vertex_[i].y();
					}
					if (y_min > poly_inter_vertex_[i].y())
					{
						y_min = poly_inter_vertex_[i].y();
					}
				}
				x_max = x_max + 2;
				x_min = x_min - 2;
				y_max = y_max + 2;
				y_min = y_min - 2;
				enlarged_width = x_max - x_min + 1;
				enlarged_height = y_max - y_min + 1;
				Area = enlarged_width * enlarged_height;
				//cout << x_max << " " << x_min << " " << y_max << " " << y_min << " " << enlarged_width << " " << enlarged_height << " " << Area << endl;
				std::vector<Eigen::Triplet<float>> tripletList;
				for (int i = 0; i < Area; i++)
				{
					tripletList.push_back(Eigen::Triplet<float>(i, i, 1));
				}
				float temp = 0.25;
				for (int i = 0; i < poly_inter_vertex_.size(); i++)
				{
					int index = poly_inter_vertex_[i].x() - x_min + (poly_inter_vertex_[i].y() - y_min) * enlarged_width;
					tripletList.push_back(Eigen::Triplet<float>(index, index, -2));
					tripletList.push_back(Eigen::Triplet<float>(index, index + 2, temp));
					tripletList.push_back(Eigen::Triplet<float>(index, index - 2, temp));
					tripletList.push_back(Eigen::Triplet<float>(index, index + 2 * enlarged_width, temp));
					tripletList.push_back(Eigen::Triplet<float>(index, index - 2 * enlarged_width, temp));
				}
				CoMatrix = Eigen::SparseMatrix<float>(Area, Area);
				CoMatrix.setFromTriplets(tripletList.begin(), tripletList.end());
			}
			break;
		}
		case kPaste:
		{
			is_pasting_ = true;

			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();
			int begin_x = source_window_->imagewidget_->x_min;
			int begin_y = source_window_->imagewidget_->y_min;
			int end_x = source_window_->imagewidget_->x_max;
			int end_y = source_window_->imagewidget_->y_max;
			int w = source_window_->imagewidget_->enlarged_width;
			int h = source_window_->imagewidget_->enlarged_height;
			int N = source_window_->imagewidget_->Area;
			std::vector<QPoint>inter_vertex_ = source_window_->imagewidget_->poly_inter_vertex_;
			Mat3Vector source_grad_x = source_window_->imagewidget_->Gradient_X;
			Mat3Vector source_grad_y = source_window_->imagewidget_->Gradient_Y;
			Eigen::SparseMatrix<float> comat = source_window_->imagewidget_->CoMatrix;

			// Paste
			if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
			{
				Mat3Vector mixgrad_x = Gradient_X.block(ypos, xpos, h, w);
				Mat3Vector mixgrad_y = Gradient_Y.block(ypos, xpos, h, w);
				for (int i = 0; i < inter_vertex_.size(); i++)
				{
					int x1 = inter_vertex_[i].x() - begin_x;
					int y1 = inter_vertex_[i].y() - begin_y;
					mixgrad_x(y1, x1) = source_grad_x(inter_vertex_[i].y(), inter_vertex_[i].x());
					mixgrad_y(y1, x1) = source_grad_y(inter_vertex_[i].y(), inter_vertex_[i].x());
				}
				mixgrad_x = get_gradient_x(mixgrad_x);
				mixgrad_y = get_gradient_y(mixgrad_y);
				Mat3Vector DIV = mixgrad_x + mixgrad_y;
				Eigen::MatrixXf b(N, 3);
				Mat3Vector smallimage = Transform(image_, xpos, ypos, w, h);
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						int index = j * w + i;
						b(index, 0) = smallimage(j, i).x();
						b(index, 1) = smallimage(j, i).y();
						b(index, 2) = smallimage(j, i).z();
					}
				}
				for (int i = 0; i < inter_vertex_.size(); i++)
				{
					int x1 = inter_vertex_[i].x() - begin_x;
					int y1 = inter_vertex_[i].y() - begin_y;
					int index = y1 * w + x1;
					b(index, 0) = DIV(y1, x1).x();
					b(index, 1) = DIV(y1, x1).y();
					b(index, 2) = DIV(y1, x1).z();
				}
				Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
				solver.compute(comat);//对A进行预分解
				Eigen::MatrixXf result = solver.solve(b);
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						int index = j * w + i;
						int red = controlcolor(result(index, 0));
						int green = controlcolor(result(index, 1));
						int blue = controlcolor(result(index, 2));
						image_->setPixel(xpos + i, ypos + j, qRgb(red, green, blue));
					}
				}
			}
		update();
		break;
		}
		default:
			break;
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		// Store point position for rectangle region
		/*if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
		}*/
		break;
	case kPaste:
	{
		is_pasting_ = true;

		int xpos = mouseevent->pos().rx();
		int ypos = mouseevent->pos().ry();
		int begin_x = source_window_->imagewidget_->x_min;
		int begin_y = source_window_->imagewidget_->y_min;
		int end_x = source_window_->imagewidget_->x_max;
		int end_y = source_window_->imagewidget_->y_max;
		int w = source_window_->imagewidget_->enlarged_width;
		int h = source_window_->imagewidget_->enlarged_height;
		int N = source_window_->imagewidget_->Area;
		std::vector<QPoint>inter_vertex_ = source_window_->imagewidget_->poly_inter_vertex_;
		Mat3Vector source_grad_x = source_window_->imagewidget_->Gradient_X;
		Mat3Vector source_grad_y = source_window_->imagewidget_->Gradient_Y;
		Eigen::SparseMatrix<float> comat = source_window_->imagewidget_->CoMatrix;
		// Paste
		if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
		{
			*(image_) = *(image_backup_);
			Mat3Vector mixgrad_x = Gradient_X.block(ypos, xpos, h, w);
			Mat3Vector mixgrad_y = Gradient_Y.block(ypos, xpos, h, w);
			for (int i = 0; i < inter_vertex_.size(); i++)
			{
				int x1 = inter_vertex_[i].x() - begin_x;
				int y1 = inter_vertex_[i].y() - begin_y;
				mixgrad_x(y1, x1) = source_grad_x(inter_vertex_[i].y(), inter_vertex_[i].x());
				mixgrad_y(y1, x1) = source_grad_y(inter_vertex_[i].y(), inter_vertex_[i].x());
			}
			mixgrad_x = get_gradient_x(mixgrad_x);
			mixgrad_y = get_gradient_y(mixgrad_y);
			Mat3Vector DIV = mixgrad_x + mixgrad_y;
			Eigen::MatrixXf b(N, 3);
			Mat3Vector smallimage = Transform(image_, xpos, ypos, w, h);
			for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{
					int index = j * w + i;
					b(index, 0) = smallimage(j, i).x();
					b(index, 1) = smallimage(j, i).y();
					b(index, 2) = smallimage(j, i).z();
				}
			}
			for (int i = 0; i < inter_vertex_.size(); i++)
			{
				int x1 = inter_vertex_[i].x() - begin_x;
				int y1 = inter_vertex_[i].y() - begin_y;
				int index = y1 * w + x1;
				b(index, 0) = DIV(y1, x1).x();
				b(index, 1) = DIV(y1, x1).y();
				b(index, 2) = DIV(y1, x1).z();
			}
			Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
			solver.compute(comat);//对A进行预分解
			Eigen::MatrixXf result = solver.solve(b);
			for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{
					int index = j * w + i;
					int red = controlcolor(result(index, 0));
					int green = controlcolor(result(index, 1));
					int blue = controlcolor(result(index, 2));

					image_->setPixel(xpos + i, ypos + j, qRgb(red, green, blue));
				}
			}
		}
		update();
		break;
	}
	default:
		break;
	}
	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose: 
		/*if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
			is_choosing_ = false;
			draw_status_ = kNone;
		}*/
	break;
	case kPaste:
	{
		is_pasting_ = true;

		int xpos = mouseevent->pos().rx();
		int ypos = mouseevent->pos().ry();
		int begin_x = source_window_->imagewidget_->x_min;
		int begin_y = source_window_->imagewidget_->y_min;
		int end_x = source_window_->imagewidget_->x_max;
		int end_y = source_window_->imagewidget_->y_max;
		int w = source_window_->imagewidget_->enlarged_width;
		int h = source_window_->imagewidget_->enlarged_height;
		int N = source_window_->imagewidget_->Area;
		std::vector<QPoint>inter_vertex_ = source_window_->imagewidget_->poly_inter_vertex_;
		Mat3Vector source_grad_x = source_window_->imagewidget_->Gradient_X;
		Mat3Vector source_grad_y = source_window_->imagewidget_->Gradient_Y;
		Eigen::SparseMatrix<float> comat = source_window_->imagewidget_->CoMatrix;
		// Paste
		if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
		{
			*(image_) = *(image_backup_);
			Mat3Vector mixgrad_x = Gradient_X.block(ypos, xpos, h, w);
			Mat3Vector mixgrad_y = Gradient_Y.block(ypos, xpos, h, w);
			for (int i = 0; i < inter_vertex_.size(); i++)
			{
				int x1 = inter_vertex_[i].x() - begin_x;
				int y1 = inter_vertex_[i].y() - begin_y;
				mixgrad_x(y1, x1) = source_grad_x(inter_vertex_[i].y(), inter_vertex_[i].x());
				mixgrad_y(y1, x1) = source_grad_y(inter_vertex_[i].y(), inter_vertex_[i].x());
			}
			mixgrad_x = get_gradient_x(mixgrad_x);
			mixgrad_y = get_gradient_y(mixgrad_y);
			Mat3Vector DIV = mixgrad_x + mixgrad_y;
			Eigen::MatrixXf b(N, 3);
			Mat3Vector smallimage = Transform(image_, xpos, ypos, w, h);
			for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{
					int index = j * w + i;
					b(index, 0) = smallimage(j, i).x();
					b(index, 1) = smallimage(j, i).y();
					b(index, 2) = smallimage(j, i).z();
				}
			}
			for (int i = 0; i < inter_vertex_.size(); i++)
			{
				int x1 = inter_vertex_[i].x() - begin_x;
				int y1 = inter_vertex_[i].y() - begin_y;
				int index = y1 * w + x1;
				b(index, 0) = DIV(y1, x1).x();
				b(index, 1) = DIV(y1, x1).y();
				b(index, 2) = DIV(y1, x1).z();
			}
			/*for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{
					image_->setPixel(i, j, qRgb(int(DIV(j, i).x()), int(DIV(j, i).y()), int(DIV(j, i).z())));
				}
			}*/

			Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
			solver.compute(comat);//对A进行预分解
			Eigen::MatrixXf result = solver.solve(b);
			for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{
					int index = j * w + i;
					int red = controlcolor(result(index, 0));
					int green = controlcolor(result(index, 1));
					int blue = controlcolor(result(index, 2));

					image_->setPixel(xpos + i, ypos + j, qRgb(red, green, blue));
				}
			}
		}
		update();
		break;
	}
	default:
		break;
	}
	update();
}



//以下4个是自行添加的函数
Mat3Vector ImageWidget::Transform(QImage* image, int begin_x, int begin_y, int w, int h)
{
	Mat3Vector temp = Mat3Vector();
	temp.resize(h, w);
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			QRgb color = image->pixel(i + begin_x, j + begin_y);
			temp(j, i) = Eigen::Vector3f(static_cast<float>(qRed(color)), static_cast<float>(qGreen(color)), static_cast<float>(qBlue(color)));
		}
	}
	return temp;
}

int ImageWidget::controlcolor(float a)//将颜色值转换为0至255之间
{
	if (a < 0)
	{
		return 0;
	}
	else if (a > 255)
	{
		return 255;
	}
	else {
		return static_cast<int>(a);
	}
}

Mat3Vector ImageWidget::get_gradient_x(Mat3Vector Mat)//对x方向求梯度
{
	int w = Mat.cols();
	int h = Mat.rows();
	Mat3Vector temp(Mat);
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			if (i == w - 1)
			{
				temp(j, i) = Mat(j, i) - Mat(j, i - 1);
			}
			else if (i == 0)
			{
				temp(j, i) = Mat(j, i + 1) - Mat(j, i);
			}
			else
			{
				temp(j, i) = (Mat(j, i + 1) - Mat(j, i - 1)) / 2;
			}
		}
	}
	return temp;
}

Mat3Vector ImageWidget::get_gradient_y(Mat3Vector Mat)//对y方向求梯度
{
	int w = Mat.cols();
	int h = Mat.rows();
	Mat3Vector temp(Mat);
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			if (j == h - 1)
			{
				temp(j, i) = Mat(j, i) - Mat(j - 1, i);
			}
			else if (j == 0)
			{
				temp(j, i) = Mat(j + 1, i) - Mat(j, i);
			}
			else
			{
				temp(j, i) = (Mat(j + 1, i) - Mat(j - 1, i)) / 2;
			}
		}
	}
	return temp;
}



void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		image_->load(filename);
		*(image_backup_) = *(image_);

		Gradient_X = Transform(image_, 0, 0, image_->width(), image_->height());
		Gradient_Y = Transform(image_, 0, 0, image_->width(), image_->height());
		Gradient_X = get_gradient_x(Gradient_X);
		Gradient_Y = get_gradient_y(Gradient_Y);
	}
	//	setFixedSize(image_->width(), image_->height());
	//	relate_window_->setWindowFlags(Qt::Dialog);
	//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
	//	relate_window_->setWindowFlags(Qt::SubWindow);

		//image_->invertPixels(QImage::InvertRgb);
		//*(image_) = image_->mirrored(true, true);
		//*(image_) = image_->rgbSwapped();
	cout << "image size: " << image_->width() << ' ' << image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(image_));
	int width = image_->width();
	int height = image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}

	}
	else
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	point_start_ = point_end_ = QPoint(0, 0);
	update();
}
