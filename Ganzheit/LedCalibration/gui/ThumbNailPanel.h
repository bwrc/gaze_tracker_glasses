#ifndef THUMBNAILPANEL_H
#define THUMBNAILPANEL_H


#include <QObject>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>


class ThumbNail : public QGraphicsItem {

	public:

		ThumbNail(const QImage *_imgBig);
		ThumbNail(const QString &img_path);

		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void setRect(const QRectF &_rect);

	private:

		void create(const QImage *qimg);

		QImage img;

		QRectF rect;

};



class ThumbNailPanel : public QGraphicsScene {

	public:

		ThumbNailPanel();

		void removeSelected(std::list<int> &indices);

		void addItem(QGraphicsItem * item);

		void adjustItems();

	private:

		int indexOf(const QGraphicsItem *item);
		void indicesOf(const QList<QGraphicsItem *> _targetItems, std::list<int> &indices);

};


#endif

