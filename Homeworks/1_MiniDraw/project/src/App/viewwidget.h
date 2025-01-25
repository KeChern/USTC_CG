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

class ViewWidget : public QWidget//������
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = 0);
	~ViewWidget();

private:
	Ui::ViewWidget ui;

private:
	bool draw_status_;    // ��ǰ����״̬��true Ϊ���Ƶ�ǰ����϶���ͼԪ��false Ϊ������
	QColor color;         // ��ǰ��ɫ
	int thickness = 3;    // ��ǰ�ʵĴ�ϸ
 	QPoint start_point_;  // ��ǰͼԪ����ʼ��
	QPoint end_point_;    // ��ǰͼԪ����ֹ��
	Shape::Type type_;

	Shape* shape_;
	std::vector<Shape*> shape_list_;

public:
	bool is_filled = false; // �Ƿ�Ҫ���
	bool is_drawpoly = false; //�Ƿ��ڻ��ƶ����

public:
	void mousePressEvent(QMouseEvent* event);   // ��������Ӧ���������Ҽ�����˫����
	void mouseMoveEvent(QMouseEvent* event);    // ����ƶ���Ӧ��������һ����Ҫ�������ĵ������������
	void mouseReleaseEvent(QMouseEvent* event); // ����ͷ���Ӧ���������Ҽ�����˫����
	void mouseDoubleClickEvent(QMouseEvent* event);
public:
	void paintEvent(QPaintEvent*);              // Qt ���еĻ��ƶ�ֻ���ڴ˺��������
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
