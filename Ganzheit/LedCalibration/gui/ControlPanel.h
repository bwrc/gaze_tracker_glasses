#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H


#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QTreeWidget>
#include <QComboBox>
#include <QAction>
#include "ResultItem.h"


class TreeWidget : public QTreeWidget {

    Q_OBJECT

	public:

		TreeWidget(QWidget *parent);

	private:

		void contextMenuEvent(QContextMenuEvent *event);

		QAction *actionNewLED;
		QAction *actionDeleteLED;
		QAction *actionDeleteResultItem;

};


class ControlPanel : public QWidget {

    Q_OBJECT

	public:

		ControlPanel(QWidget *parent);

		void insertNewItem(ResultItem *item);
		ResultItem *getResultItem() {return curItem;}

		void disableAll();
		void enableAll();

		QComboBox *videoSource;

		QPushButton *btn_calibrate;
		QPushButton *btn_add_calib_img;
		QPushButton *btn_reset_calibration;

		QSlider *slider_th_reflection;
		QSlider *slider_th_rect;

		QCheckBox *checkBoxCalibrate;
		QCheckBox *checkBoxGray;

		TreeWidget *list_calib;

		static QTreeWidgetItem *getAncestor(QTreeWidgetItem *item);

		/*
		 * Returns true if the given calibration directory is already
		 * being used by any item and false otherwise.
		 */
		bool contains(const QString &calibDir);

	public slots:

		void doubleClick(QTreeWidgetItem *item, int column);

		void listItemChanged();

		void onCheckBoxStateChanged(int);

	private:

		void selectItem(ResultItem *item);

		void resizeEvent(QResizeEvent *evt);

		void setStateToAll(bool state);

		ResultItem *curItem;

};


#endif

