#include "viewwidget.h"
#include <QColorDialog>

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	draw_status_ = false;// 设置初始绘制状态为 – 不绘制
	shape_ = NULL;
	type_ = Shape::kDefault;
}

ViewWidget::~ViewWidget()
{
	for (size_t i = 0; i < shape_list_.size(); i++)
	{
		if (shape_list_[i])
		{
			delete shape_list_[i];
			shape_list_[i] = NULL;
		}
	}
}

void ViewWidget::setLine()
{
	type_ = Shape::kLine;
}

void ViewWidget::setTria()
{
	type_ = Shape::kTria;
}

void ViewWidget::setRect()
{
	type_ = Shape::kRect;
}

void ViewWidget::setCirc()
{
	type_ = Shape::kCirc;
}

void ViewWidget::setElli()
{
	type_ = Shape::kElli;
}

void ViewWidget::setPoly()
{
	type_ = Shape::kPoly;
}

void ViewWidget::setFree()
{
	type_ = Shape::kFree;
}

void ViewWidget::setfillcolor()
{
	is_filled = !is_filled;
}

void ViewWidget::setThickness(int t)
{
	thickness = t;
}

void ViewWidget::ClearAll()
{
	shape_list_.clear();
}
void ViewWidget::ClearOne()
{
	if (shape_list_.size() > 0)
	{
		shape_list_.pop_back();
	}
}

void ViewWidget::ChooseColor()
{
	color = QColorDialog::getColor(Qt::black, this);
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
	if (Qt::LeftButton == event->button())// 判断是否是鼠标左击
	{
		switch (type_)
		{
		case Shape::kLine:
			shape_ = new Line();
			break;
		case Shape::kTria:
			shape_ = new Tria();
			break;
		case Shape::kRect:
			shape_ = new Rect();
			break;
		case Shape::kCirc:
			shape_ = new Circ();
			break;
		case Shape::kElli:
			shape_ = new Elli();
			break;
		case Shape::kPoly:
			this->setMouseTracking(true);
			break;
		case Shape::kFree:
			shape_ = new Free();
			break;
		case Shape::kDefault:
			break;
		}
		if (shape_ != NULL && type_ != Shape::kPoly)
		{
			draw_status_ = true;// 设置绘制状态为 – 绘制
			start_point_ = end_point_ = event->pos();// 将图元初始点设置为当前鼠标击发点
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
			shape_->set_is_filled(is_filled);
			shape_->set_color(color);
			shape_->set_thickness(thickness);
			if (type_ == Shape::kFree)
			{
				shape_->Push(start_point_);
			}
		}
		if (!is_drawpoly && shape_ == NULL && type_ == Shape::kPoly)
		{
			shape_ = new Poly();
			is_drawpoly = true;
			start_point_ = end_point_ = event->pos();
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
			shape_->set_is_filled(is_filled);
			shape_->set_color(color);
			shape_->set_thickness(thickness);
			shape_->Push(event->pos());
		}
	}
	update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (draw_status_ && shape_ != NULL)// 判断当前绘制状态
	{
		end_point_ = event->pos();// 若为真，则设置图元终止点位鼠标当前位置
		shape_->set_end(end_point_);
		if (type_ == Shape::kFree)
		{
			shape_->Push(event->pos());
		}
	}
	if (shape_ != NULL && is_drawpoly && type_ == Shape::kPoly)
	{
		end_point_ = event->pos();
		shape_->set_end(end_point_);
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (shape_ != NULL && type_ != Shape::kPoly)
	{
		draw_status_ = false;// 设置绘制状态为 – 不绘制
		shape_list_.push_back(shape_);
		shape_ = NULL;
	}
	if (is_drawpoly && shape_ != NULL && type_ == Shape::kPoly)
	{
		shape_->Push(event->pos());
	}
}

void ViewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && is_drawpoly && shape_ != NULL && type_ == Shape::kPoly)
	{
		is_drawpoly = false;
		shape_->Push(event->pos());
		shape_->set_end(start_point_);
		shape_list_.push_back(shape_);
		shape_ = NULL;
		this->setMouseTracking(false);
	}
}

void ViewWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);// 定义painter在this指向的控件（此例为ViewWidget）中绘图

	// set painter
	painter.setRenderHint(QPainter::Antialiasing, true);// 反走样

	for (int i = 0; i < shape_list_.size(); i++)
	{
		if (shape_list_[i]->is_filled)painter.setBrush(shape_list_[i]->color);
		painter.setPen(QPen(shape_list_[i]->color, shape_list_[i]->thickness));
		shape_list_[i]->Draw(painter);
		painter.setBrush(QBrush(Qt::NoBrush));
	}
	
	if (shape_ != NULL) {
		if (shape_->is_filled)painter.setBrush(shape_->color);
		painter.setPen(QPen(shape_->color, shape_->thickness));
		shape_->Draw(painter);
		painter.setBrush(QBrush(Qt::NoBrush));
	}

	//一次性画图
	//painter.drawLine(start_point_, end_point_); // 绘制线段
	//painter.end(); // 结束绘图

	update();
}