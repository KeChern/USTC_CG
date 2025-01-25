#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>

using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();

	/*test(2,2,1,1);
	test(6,2,7,3);
	test(5,5,6,6);
	test(2,6,3,7);*/
}

void ImageWidget::test(int i, int j, int r, int t)
{
	start_list_.push_back(QPoint(i * 32, j * 32));
	end_list_.push_back(QPoint(r * 32, t * 32));
}

ImageWidget::~ImageWidget(void)
{
}

void ImageWidget::paintEvent(QPaintEvent *paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2, ptr_image_->width(), ptr_image_->height());
	painter.drawImage(rect, *ptr_image_); 

	if (warp_status_ && start_list_.size() > 0)
	{
		// draw the points
		painter.setRenderHint(QPainter::Antialiasing, true);// ·´×ßÑù
		painter.setPen(QPen(QColor(255, 0, 0), 4));

		for (int i = 0; i < end_list_.size(); i++)
		{
			painter.drawLine(start_list_[i] + bias, end_list_[i] + bias);
		}
		//painter.drawLine(start_point_ + bias, end_point_ + bias);

		painter.setPen(QPen(QColor(0, 0, 255), 10));
		for (int i = 0; i < end_list_.size(); i++)
		{
			painter.setPen(QPen(QColor(0, 0, 255), 10));
			painter.drawPoint(start_list_[i] + bias);

			painter.setPen(QPen(QColor(0, 255, 0), 10));
			painter.drawPoint(end_list_[i] + bias);
		}
		painter.setPen(QPen(QColor(0, 0, 255), 10));
		//painter.drawPoint(start_point_ + bias);

		painter.setPen(QPen(QColor(0, 255, 0), 10));
		//painter.drawPoint(end_point_ + bias);
	}

	update();
	
	painter.end();
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}

	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout << "image size: " << ptr_image_->width() << ' ' << ptr_image_->height() << endl;

	bias = QPoint((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2);

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

	ptr_image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width-1-i, height-1-j));
				}
			}
		} 
		else			
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width-1-i, j));
				}
			}
		}
		
	}
	else
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(i, height-1-j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			int gray_value = (qRed(color)+qGreen(color)+qBlue(color))/3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value) );
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(ptr_image_) = *(ptr_image_backup_);
	update();
}

void ImageWidget::mousePressEvent(QMouseEvent* event)
{
	if (warp_status_)
	{
		start_point_ = event->pos() - bias;
		end_point_ = event->pos() - bias;
		start_list_.push_back(event->pos() - bias);
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (warp_status_)
	{
		end_point_ = event->pos() - bias;
	}
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (warp_status_)
	{
		end_list_.push_back(event->pos() - bias);
	}
}

void ImageWidget::setWarpStatus()
{
	if (warp_status_ == true)
	{
		start_list_.clear();
		end_list_.clear();
		*(ptr_image_) = *(ptr_image_backup_);
	}
	warp_status_ = !warp_status_;
}

void ImageWidget::setDeNoise()
{
	denoise_status_ = !denoise_status_;
}

void ImageWidget::IDW_ImageWarp()
{
	if (warp_status_)
	{
		warp_ = new IDW();
		warp_->SetImage(ptr_image_);
		warp_->SetStartPoints(start_list_);
		warp_->SetEndPoints(end_list_);
		warp_->setDeNoise(denoise_status_);
		warp_->Compute();
		*(ptr_image_) = *(warp_->Image());
		update();
	}
}

void ImageWidget::RBF_ImageWarp()
{
	if (warp_status_)
	{
		warp_ = new RBF();
		warp_->SetImage(ptr_image_);
		warp_->SetStartPoints(start_list_);
		warp_->SetEndPoints(end_list_);
		warp_->setDeNoise(denoise_status_);
		warp_->Compute();
		*(ptr_image_) = *(warp_->Image());
		update();
	}
}