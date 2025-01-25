#include "minidraw.h"
#include <QToolBar>
#include<QIcon>


MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	view_widget_ = new ViewWidget();
	Creat_Action();
	Creat_ToolBar();
	Creat_Menu();

	setCentralWidget(view_widget_);// 将ViewWidget控件设置为主窗口的中心窗口
}

void MiniDraw::Creat_Action()
{
	Action_About = new QAction(tr("&About"), this);
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

	Action_Line = new QAction(tr("&Line"), this);
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));
	Action_Line->setIcon(QIcon("../assets/Line.png"));

	Action_Tria = new QAction(tr("&Triangle"), this);
	connect(Action_Tria, &QAction::triggered, view_widget_, &ViewWidget::setTria);
	Action_Tria->setIcon(QIcon("../assets/Tria.png"));

	Action_Rect = new QAction(tr("&Rectangle"), this);
	connect(Action_Rect, &QAction::triggered, view_widget_, &ViewWidget::setRect);
	Action_Rect->setIcon(QIcon("../assets/Rect.png"));

	Action_Circ = new QAction(tr("&Circle"), this);
	connect(Action_Circ, &QAction::triggered, view_widget_, &ViewWidget::setCirc);
	Action_Circ->setIcon(QIcon("../assets/Circ.png"));

	Action_Elli = new QAction(tr("&Ellipse"), this);
	connect(Action_Elli, &QAction::triggered, view_widget_, &ViewWidget::setElli);
	Action_Elli->setIcon(QIcon("../assets/Elli.png"));

	Action_Poly = new QAction(tr("&Polygon"), this);
	connect(Action_Poly, &QAction::triggered, view_widget_, &ViewWidget::setPoly);
	Action_Poly->setIcon(QIcon("../assets/Poly.png"));

	Action_Free = new QAction(tr("&Free"), this);
	connect(Action_Free, &QAction::triggered, view_widget_, &ViewWidget::setFree);
	Action_Free->setIcon(QIcon("../assets/Free.png"));

	Action_ChooseColor = new QAction(tr("&Color"), this);
	connect(Action_ChooseColor, &QAction::triggered, view_widget_, &ViewWidget::ChooseColor);
	Action_ChooseColor->setIcon(QIcon("../assets/ChooseColor.png"));

	Action_ClearOne = new QAction(tr("&ClearOne"), this);
	connect(Action_ClearOne, &QAction::triggered, view_widget_, &ViewWidget::ClearOne);
	Action_ClearOne->setIcon(QIcon("../assets/SmallEraser.png"));

	Action_ClearAll = new QAction(tr("&ClearAll"), this);
	connect(Action_ClearAll, &QAction::triggered, view_widget_, &ViewWidget::ClearAll);
	Action_ClearAll->setIcon(QIcon("../assets/BigEraser.png"));
}
void MiniDraw::Creat_CheckBox()
{
	checkbox = new QCheckBox("Fill", this);
	pToolBar->addWidget(checkbox);
	connect(checkbox, &QCheckBox::clicked, view_widget_, &ViewWidget::setfillcolor);
}

void MiniDraw::Creat_Slider()
{
	slider = new QSlider(Qt::Horizontal);
	pToolBar->addWidget(slider);
	slider->setMinimum(1); // 设置滑动条的最小值
	slider->setMaximum(7); // 设置滑动条的最大值
	slider->setValue(3);   // 设置滑动条初始值
	connect(slider, SIGNAL(valueChanged(int)), view_widget_, SLOT(setThickness(int)));
}

void MiniDraw::Creat_ToolBar() {
	pToolBar = addToolBar(tr("&Main"));
	pToolBar->addAction(Action_Line);
	pToolBar->addAction(Action_Tria);
	pToolBar->addAction(Action_Rect);
	pToolBar->addAction(Action_Circ);
	pToolBar->addAction(Action_Elli);
	pToolBar->addAction(Action_Poly);
	pToolBar->addAction(Action_Free);

	pToolBar->addAction(Action_ChooseColor);

	pToolBar->addAction(Action_ClearOne);
	pToolBar->addAction(Action_ClearAll);

	pToolBar->addSeparator();
	Creat_CheckBox();

	pToolBar->addSeparator();
	Creat_Slider();
}

void MiniDraw::Creat_Menu()
{
	pMenu = menuBar()->addMenu(tr("&Figure Tools"));
	pMenu->addAction(Action_About);
	pMenu->addAction(Action_Line);
	pMenu->addAction(Action_Tria);
	pMenu->addAction(Action_Rect);
	pMenu->addAction(Action_Circ);
	pMenu->addAction(Action_Elli);
	pMenu->addAction(Action_Poly);
	pMenu->addAction(Action_Free);
	pMenu->addAction(Action_ChooseColor);
	pMenu->addAction(Action_ClearOne);
	pMenu->addAction(Action_ClearAll);
}

void MiniDraw::AboutBox()
{
	QMessageBox::about(this, tr("About"), tr("This is a small program called MiniDraw.\n You can paint everything by the tools."));
}

MiniDraw::~MiniDraw() {}
