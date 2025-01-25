#pragma once

#include <ui_viewwidget.h>

#include "Shape.h"
#include "Line.h"
#include "Tria.h"
#include "Rect.h"
#include "Circ.h"
#include "Elli.h"
#include "Poly.h"
#include "Free.h"

#include <qevent.h>
#include <qpainter.h>
#include <QWidget>

#include <vector>

class ViewWidget : public QWidget//窗口类
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = 0);
	~ViewWidget();

private:
	Ui::ViewWidget ui;

private:
	bool draw_status_;    // 当前绘制状态，true 为绘制当前鼠标拖动的图元，false 为不绘制
	QColor color;         // 当前颜色
	int thickness = 3;    // 当前笔的粗细
 	QPoint start_point_;  // 当前图元的起始点
	QPoint end_point_;    // 当前图元的终止点
	Shape::Type type_;

	Shape* shape_;
	std::vector<Shape*> shape_list_;

public:
	bool is_filled = false; // 是否要填充
	bool is_drawpoly = false; //是否在绘制多边形

public:
	void mousePressEvent(QMouseEvent* event);   // 鼠标击发响应函数（左右键，单双击）
	void mouseMoveEvent(QMouseEvent* event);    // 鼠标移动响应函数（其一个重要性质在文档最后有详述）
	void mouseReleaseEvent(QMouseEvent* event); // 鼠标释放响应函数（左右键，单双击）
	void mouseDoubleClickEvent(QMouseEvent* event);
public:
	void paintEvent(QPaintEvent*);              // Qt 所有的绘制都只能在此函数中完成
signals:
public slots:
	void setLine();
	void setTria();
	void setRect();
	void setCirc();
	void setElli();
	void setPoly();
	void setFree();

	void setfillcolor();
	void ChooseColor();
	void setThickness(int t);

	void ClearOne();
	void ClearAll();
};
