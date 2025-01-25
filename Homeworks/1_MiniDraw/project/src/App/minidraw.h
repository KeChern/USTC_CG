#pragma once

#include <ui_minidraw.h>
#include <viewwidget.h>

#include <QtWidgets/QMainWindow>
#include <qmessagebox.h>
#include <QCheckBox>
#include <QSlider>
class MiniDraw : public QMainWindow {
	Q_OBJECT

public:
	MiniDraw(QWidget* parent = 0);
	~MiniDraw();

	QMenu* pMenu;
	QToolBar* pToolBar;

	QAction* Action_About;

	QAction* Action_Line;
	QAction* Action_Tria;
	QAction* Action_Rect;
	QAction* Action_Circ;
	QAction* Action_Elli;
	QAction* Action_Poly;
	QAction* Action_Free;

	QAction* Action_ChooseColor;
	QAction* Action_ClearOne;
	QAction* Action_ClearAll;

	QCheckBox* checkbox;
	QSlider* slider;

	void Creat_Menu();
	void Creat_ToolBar();
	void Creat_Action();
	void Creat_CheckBox();
	void Creat_Slider();

	void AboutBox();

private:
	Ui::MiniDrawClass ui;
	ViewWidget* view_widget_;
};
