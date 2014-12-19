#include "ThumbNailPanel.h"
#include <QFile>
#include <QDir>


static const int TN_W = 64;
static const int TN_H = 64;


ThumbNail::ThumbNail(const QImage *imgBig)
 : QGraphicsItem() {

	create(imgBig);

}


ThumbNail::ThumbNail(const QString &img_path)
 : QGraphicsItem() {

	QImage imgBig;
	if(imgBig.load(img_path)) {

		create(&imgBig);

	}
	else {

		setFlag(QGraphicsItem::ItemIsSelectable);
		img = QImage(TN_W, TN_H, QImage::Format_RGB888);

	}

}


void ThumbNail::create(const QImage *qimg) {

	setFlag(QGraphicsItem::ItemIsSelectable);

	// copy a scaled version of the image
	img = qimg->scaled(400, 300).scaled(TN_W,
										TN_H,
										Qt::IgnoreAspectRatio,
										Qt::SmoothTransformation);

}


QRectF ThumbNail::boundingRect() const {

	return rect;

}


void ThumbNail::setRect(const QRectF &_rect) {

	rect = _rect;

	QGraphicsItem::prepareGeometryChange();

}


void ThumbNail::paint(QPainter *painter,
					  const QStyleOptionGraphicsItem *,	// unused
					  QWidget *) {						// unused

	painter->drawImage(rect, img);

	// draw a boundary rectangle for all selected items
	if(isSelected()) {

		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(4);

		painter->setPen(pen);

		QRect bounds(rect.x() + 2, rect.y() + 2, rect.width() - 4, rect.height() - 4);

		painter->drawRect(bounds);

	}

}



ThumbNailPanel::ThumbNailPanel() : QGraphicsScene() {

}


int ThumbNailPanel::indexOf(const QGraphicsItem *item) {

	// get the items
	QList<QGraphicsItem *> items = this->items(Qt::AscendingOrder);

	// loop and see if the item is in the list
	for(int i = 0; i < items.size(); ++i) {

		if(items.at(i) == item) {
			return i;
		}

	}

	return -1;

}


void ThumbNailPanel::indicesOf(const QList<QGraphicsItem *> _targetItems, std::list<int> &indices) {

	// copy contents
	QList<QGraphicsItem *> targetItems(_targetItems);

	// get all items
	const QList<QGraphicsItem *> allItems = this->items(Qt::AscendingOrder);

	while(targetItems.size() > 0) {

		// get the first element
		const QGraphicsItem *curItem = targetItems.front();

		// get the index of it
		for(int i = 0; i < allItems.size(); ++i) {

			if(allItems.at(i) == curItem) {

				indices.push_back(i);
				targetItems.removeFirst();

				break;

			}

		}

	}

}


void ThumbNailPanel::removeSelected(std::list<int> &indices) {

	indices.clear();

	/********************************************************
	* Remove the selected items
	********************************************************/
	QList<QGraphicsItem *> selItems = selectedItems();

	indicesOf(selItems, indices);

	while(!selItems.empty()) {

		QGraphicsItem *item = selItems.front();

		/*
		 * This causes a double free. The destructor of a QGraphicsItem removes
		 * the item from the QGraphicsScene's list. This is ok, but
		 * removeItem should also invalidate the parent-child relationship, but
		 * it does not! This behaviour is very strange since this is what the
		 * documentation has to say:
		 *
		 * http://qt-project.org/doc/qt-4.8/qgraphicsscene.html#removeItem
		 * void QGraphicsScene::removeItem ( QGraphicsItem * item )
		 *   "Removes the item item and all its children from the scene.
		 *    The ownership of item is passed on to the caller
		 *    (i.e., QGraphicsScene will no longer delete item when destroyed)."
		 */
//		removeItem(item);

		delete item;

		selItems.removeFirst();

	}


	/********************************************************
	* Reposition the existing items
	********************************************************/
	adjustItems();

}


void ThumbNailPanel::addItem(QGraphicsItem *item) {

	// compute the position for the item
	ThumbNail *tn = (ThumbNail *)item;
	QRectF rect(this->items().size()*TN_W, 0, TN_W, TN_H);
	tn->setRect(rect);

	// let the super handle other stuff
	QGraphicsScene::addItem(item);

}


void ThumbNailPanel::adjustItems() {

	QList<QGraphicsItem *> allItems = this->items(Qt::AscendingOrder);
	int len = allItems.size();

	for(int i = 0; i < len; ++i) {

		ThumbNail *curItem = (ThumbNail *)allItems.at(i);

		QRectF rect(i*TN_W, 0, TN_W, TN_H);

		curItem->setRect(rect);

	}

}

