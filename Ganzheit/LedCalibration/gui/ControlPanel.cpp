/**********************************************************************
 * ControlPanel.cpp
 *
 * This file implements a QWidget for getting settings and parameters
 * from the user.
 *********************************************************************/

#include "ControlPanel.h"
#include <QResizeEvent>
#include <QMenu>
#include <stdio.h>



ControlPanel::ControlPanel(QWidget *parent) : QWidget(parent) {

	videoSource	= new QComboBox(this);
	videoSource->insertItem(0, "cam0");
	videoSource->insertItem(1, "cam1");
	videoSource->insertItem(2, "cam2");

	checkBoxCalibrate = new QCheckBox(tr("Calibrate"), this);
	checkBoxCalibrate->setCheckState(Qt::Unchecked);

	checkBoxGray = new QCheckBox(tr("Gray"), this);
	checkBoxGray->setCheckState(Qt::Unchecked);


	// Add buttons
	btn_calibrate			= new QPushButton(tr("Calibrate"), this);
	btn_add_calib_img		= new QPushButton(tr("Add sample"), this);
	btn_reset_calibration	= new QPushButton(tr("Reset Calibration"), this);

	// Create a listview widget
	list_calib = new TreeWidget(this);
	list_calib->setColumnCount(3);
	QStringList labels; labels << "Name" << "Description" << "Value";
	list_calib->setHeaderLabels(labels);

	// create sliders
	slider_th_reflection = new QSlider(Qt::Horizontal, this);
	slider_th_reflection->setMinimum(0);
	slider_th_reflection->setMaximum(255);
	slider_th_reflection->setValue(255);

	slider_th_rect = new QSlider(Qt::Horizontal, this);
	slider_th_rect->setMinimum(0);
	slider_th_rect->setMaximum(255);
	slider_th_rect->setValue(0);


	curItem = NULL;

	connect(list_calib, SIGNAL(itemDoubleClicked(QTreeWidgetItem * , int  )),
			this, SLOT(doubleClick(QTreeWidgetItem * , int  )));

	connect(checkBoxCalibrate, SIGNAL(stateChanged(int)),
			this, SLOT(onCheckBoxStateChanged(int)));

}


void ControlPanel::resizeEvent(QResizeEvent *evt) {


	/**********************************************************************
	 * Source selection
	 *********************************************************************/
	int h = 30;
	int w = 0.95 * evt->size().width();
	int x = 10, y = 10;

	videoSource->setGeometry(x, y, w, h);


	/**********************************************************************
	 * The checkbox is placed on the upper part of this control
	 *********************************************************************/
	y += h + 5; checkBoxCalibrate->setGeometry(x, y, w/2, h);


	/**********************************************************************
	 * ...And to its right is the grayscale checkbox
	 *********************************************************************/
	checkBoxGray->setGeometry(x + w/2, y, w/2, h);


	/**********************************************************************
	 * The control buttons are placed on bottom of this control
	 *********************************************************************/

	y += h + 5;
	const int desiredBtnW = 150;
	int btnW = desiredBtnW < w ? desiredBtnW : w;
	btn_reset_calibration->setGeometry(x, y, btnW, h);
	y += h + 5; btn_add_calib_img->setGeometry(x, y, btnW, h);
	y += h + 5; btn_calibrate->setGeometry(x, y, btnW, h);


	/**********************************************************************
	 * The list tree is placed below the buttons
	 *********************************************************************/
	y += h + 5;
	h = 250;
	list_calib->setGeometry(x, y, w, h);
	list_calib->setColumnWidth(0, 0.5*w);
	list_calib->setColumnWidth(1, 0.3*w);
	list_calib->setColumnWidth(2, 0.2*w);


	/**********************************************************************
	 * The sliders are placed below the list tree
	 *********************************************************************/
	y += h +5;
	h = 20;
	x = 5;
	slider_th_reflection->setGeometry(x, y, w, h);

	y += h + 5;
	slider_th_rect->setGeometry(x, y, w, h);

}


void ControlPanel::disableAll() {

	setStateToAll(false);

}


void ControlPanel::enableAll() {

	setStateToAll(true);

}


void ControlPanel::setStateToAll(bool state) {

	videoSource->setEnabled(state);

	btn_calibrate->setEnabled(state);
	btn_add_calib_img->setEnabled(state);
	btn_reset_calibration->setEnabled(state);

	slider_th_reflection->setEnabled(state);
	slider_th_rect->setEnabled(state);

	list_calib->setEnabled(state);

}



TreeWidget::TreeWidget(QWidget *parent)
				: QTreeWidget(parent) {

	actionNewLED			= new QAction(tr("New LED"), this);
	actionDeleteLED			= new QAction(tr("Delete LED"), this);
	actionDeleteResultItem	= new QAction(tr("Remove calibration"), this);

}


void TreeWidget::contextMenuEvent(QContextMenuEvent *event) {

	if(event->reason() == QContextMenuEvent::Mouse) {

		// get the clicked item
		QPoint pos = event->pos();
		QTreeWidgetItem *item = itemAt(pos);

		// see if there is an item
		if(item == NULL) {

			return;

		}

		// get the ancestor of the item
		ResultItem *resItem = (ResultItem *)ControlPanel::getAncestor(item);

		if(resItem == item) {

			// create and pop the menu
			QMenu menu(this);
			menu.addAction(actionDeleteResultItem);
			QAction *action = menu.exec(event->globalPos());

			// determine what was selected
			if(action == actionDeleteResultItem) {

				int index = indexOfTopLevelItem(resItem);
				takeTopLevelItem(index);
				delete resItem;

			}

		}
		else {

			// see the type of item that was selected
			QString text = item->text(0);
			if(text == "LEDs") {

				// create and pop the menu
				QMenu menu(this);
				menu.addAction(actionNewLED);
				QAction *action = menu.exec(event->globalPos());

				// determine what was selected
				if(action == actionNewLED) {

					resItem->addNewLED();

				}

			}
			else if(text == "LED") {

				// create and pop the menu
				QMenu menu(this);
				menu.addAction(actionDeleteLED);
				QAction *action = menu.exec(event->globalPos());

				// determine what was selected
				if(action == actionDeleteLED) {

					resItem->deleteLED(item);

				}

			}

		}

	}

}


/*
 *       ResultItem
 *       |
 *       |--Camera
 *       |  |--intr
 *       |  |--dist
 *       |  |--err
 *       |  |--imgSz
 *       |
 *       |--LEDs
 *          |--LED
 *          |  |--cs
 *          |  |--pos
 *          |
 *          |--LED
 *             |--cs
 *             |--pos
 */
void ControlPanel::listItemChanged() {

	/* The item can be a ResultItem, or one of its descendants */
	QTreeWidgetItem *item = list_calib->currentItem();

	if(item == NULL) {
		return;
	}


	/*
	 * ResultItem is defined as a top-level item, i.e. it has no
	 * QTreeWidgetItem parent. Find the ResultItem associated with
	 * this item.
	 */
	ResultItem *resItem = (ResultItem *)ControlPanel::getAncestor(item);

	// see if the ResultItem has changed
	if(resItem != curItem) {

		/* Select the ResultItem, curItem will be set to resItem */
		selectItem(resItem);

	}


	if(curItem != item) {

		curItem->childSelected(item);

	}

}


void ControlPanel::selectItem(ResultItem *item) {

	// select the item
	curItem = item;

	// get top level item count
	int count = list_calib->topLevelItemCount();

	for(int i = 0; i < count; ++i) {

		QTreeWidgetItem *it = list_calib->topLevelItem(i);

		QFont font;
		font.setBold(it == curItem);
		it->setFont(0, font);

		static const QColor colorSelected(0, 150, 250);
		static const QColor colorNotSelected(255, 255, 255);

		const QColor &color = it == curItem ? colorSelected : colorNotSelected;
		QBrush brush(color);
		it->setBackground(0, brush);

	}

}


void ControlPanel::insertNewItem(ResultItem *item) {

	connect(item, SIGNAL(calibItemChanged(int)),
			parent(), SLOT(onCalibItemChanged(int)));


	list_calib->addTopLevelItem(item);

	selectItem(item);

}


void ControlPanel::doubleClick(QTreeWidgetItem *item, int column) {

	curItem->doubleClick(item, column);

}


QTreeWidgetItem *ControlPanel::getAncestor(QTreeWidgetItem *item) {

	// get the ResultItem who is its ancestor
	QTreeWidgetItem *resItem = item;

	while(resItem->parent() != NULL) {

		resItem = resItem->parent();

	}

	return resItem;

}


bool ControlPanel::contains(const QString &calibDir) {

	// generate the filename
	QString fileName = calibDir + QString("/") + QString("calibration.calib");

	// get top level item count
	int count = list_calib->topLevelItemCount();

	for(int i = 0; i < count; ++i) {

		ResultItem *resItem = (ResultItem *)list_calib->topLevelItem(i);

		if(resItem->getFileName() == fileName) {
			return true;
		}

	}


	return false;

}


void ControlPanel::onCheckBoxStateChanged(int newState) {

	if(newState == Qt::Checked) {

		btn_calibrate->setEnabled(true);
		btn_add_calib_img->setEnabled(true);
		btn_reset_calibration->setEnabled(true);

		list_calib->setEnabled(true);

	}
	else {

		btn_calibrate->setEnabled(false);
		btn_add_calib_img->setEnabled(false);
		btn_reset_calibration->setEnabled(false);

		list_calib->setEnabled(false);

	}

}

