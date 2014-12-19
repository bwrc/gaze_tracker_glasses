#include "ResultItem.h"
#include <QMessageBox>
//#include "XMLInputOutput.h"
#include "CalibDataWriter.h"
#include "CalibDataReader.h"
#include <QDir>
#include <QInputDialog>
#include <QMenu>


/***************************************************************
 * Inside the user-defined folder will be these
 ***************************************************************/

/* The calibration file */
static const QString calibFile("calibration.calib");

/* The camera calibration image directory */
static const QString camCalibImgDir("images/camcalib");

/* The LED calibration image directory */
static const QString LEDCalibImgDir("images/LEDcalib");



ResultItem::ResultItem() : QTreeWidgetItem() {

	calibMode = ModeCamera;

}


ResultItem::~ResultItem() {

	for(size_t i = 0; i < LEDItems.size(); ++i) {
		delete LEDItems[i];
	}

	LEDItems.clear();

}


void vecToStr(double *vec, int n, QString &str) {

	str.clear();

	for(int i = 0; i < n; ++i) {
		str += QString::number(vec[i]) + QString(",");
	}

	if(n > 0) {
		str.resize(str.count() - 1);
	}

}


bool ResultItem::init(const QString &_calibDir,
					  const std::vector<cv::Point3f> &camera_object_points,
					  const std::vector<cv::Point3f> &,
					  const cv::Size &imgSize) {

	// make sure that there is always at least 1 LED calibration container
	LEDCalibData.push_back(calib::LEDCalibContainer());

	// copy data
	cameraCalibData	= calib::CameraCalibContainer(camera_object_points, imgSize);
	calibDir		= _calibDir;

	// used in naming the jpeg-files
	adjustImageIDs();

	// the current LED being calibrated
	curLED = 0;

	// create the the directory tree inide calibDir
	QDir dir;
	QString path = calibDir + QString("/") + camCalibImgDir;

	if(!dir.mkpath(path)) {

		return false;

	}


	path = calibDir + QString("/") + LEDCalibImgDir;

	if(!dir.mkpath(path)) {

		return false;

	}


	// set the column values
	this->setText(0, "new item");
	this->setText(1, "Calibration item");

	itemCam			= new QTreeWidgetItem(this);
	itemIntr		= new QTreeWidgetItem(itemCam);
	itemDist		= new QTreeWidgetItem(itemCam);
	itemReprojErr	= new QTreeWidgetItem(itemCam);
	itemImgSize		= new QTreeWidgetItem(itemCam);
	itemLEDs		= new QTreeWidgetItem(this);

	createLEDItems();

	// select the first LED
	selectLED(LEDItems[0]->itemLED);

	updateTree();

	return true;

}


void ResultItem::createLEDItems() {

	// clear old items
	deleteLEDItems();


	// allocate for new items
	size_t sz = LEDCalibData.size();
	LEDItems.resize(sz);

	// populate
	for(size_t i = 0; i < sz; ++i) {

		// make new
		LEDItems[i] = new LEDItemContainer(true);
		itemLEDs->addChild(LEDItems[i]->itemLED);

	}

}


void ResultItem::deleteLEDItems() {

	for(size_t i = 0; i < LEDItems.size(); ++i) {
		delete LEDItems[i];
	}

	LEDItems.clear();

}


void ResultItem::childSelected(QTreeWidgetItem *itemChild) {

	// now check if the selected item is an LED
	QString text = itemChild->text(0);

	if(text == "LED") {

		calibMode = ModeLED;

		selectLED(itemChild);

		emit calibItemChanged(ItemLED);

	}
	else if(text == "camera") {

		calibMode = ModeCamera;

		emit calibItemChanged(ItemCamera);

	}

}


void ResultItem::selectLED(QTreeWidgetItem *item) {

	size_t sz = LEDItems.size();

	// first set fonts to non-bold
	for(size_t i = 0; i < sz; ++i) {

		QBrush brush;

		QFont font;
		font.setBold(false);

		LEDItems[i]->itemLED->setFont(0, font);
		LEDItems[i]->itemLED->setBackground(0, brush);

	}

	for(size_t i = 0; i < sz; ++i) {

		if(item == LEDItems[i]->itemLED) {

			QFont font;
			font.setBold(true);

			item->setFont(0, font);

			int r = 0;
			int g = 100;
			int b = 200;
			QBrush brush(QColor(r, g, b));

			item->setBackground(0, brush);


			curLED = i;
			break;

		}

	}

}


/*
 * When this function is called the current LED has already been
 * chosen due to MainWindow::list_item_changed(). Therefore, the
 * current LED can be modified on request.
 */
void ResultItem::doubleClick(QTreeWidgetItem *item, int ) {

	// do nothing with top-level items, ResultItem
	if(item->parent() == NULL) {

		return;

	}

	QString txt = item->text(0);


	/*******************************************************
	 * Circle spacing
	 *******************************************************/
	if(txt == "cs") {

		// get the parent LED
		int index = getLEDCalibIndexByItemCS(item);

		calib::LEDCalibContainer &container = LEDCalibData[index];

		double oldcs = container.circleSpacing;

		// get the new value
		bool ok;
		double newcs = QInputDialog::getDouble(
							NULL,							// parent
							tr("Enter circle spacing"),		// title
							tr("Enter circle spacing:"),	// label
							oldcs,							// current value
							0,								// min
							100,							// max
							5,								// decimals
							&ok);							// success

		if(ok) {

			container.circleSpacing = newcs;
			LEDItems[index]->itemCircleSpacing->setText(2, QString::number(newcs));

		}

	}

}


int ResultItem::getLEDCalibIndexByLED(const QTreeWidgetItem *itemLEDTarget) {

	size_t sz = LEDItems.size();

	int index = 0;

	for(size_t i = 0; i < sz; ++i) {

		const QTreeWidgetItem *itemLED = LEDItems[i]->itemLED;

		if(itemLEDTarget == itemLED) {

			index = i;

			break;

		}

	}

	return index;

}


int ResultItem::getLEDCalibIndexByItemCS(const QTreeWidgetItem *itemCs) {

	size_t sz = LEDItems.size();

	int index = 0;

	for(size_t i = 0; i < sz; ++i) {

		const QTreeWidgetItem *itemLED = LEDItems[i]->itemLED;

		if(itemCs->parent() == itemLED) {

			index = i;

			break;

		}

	}

	return index;

}


void ResultItem::updateTree() {

	/***********************************************************
	 * Camera branch
	 ***********************************************************/

	itemCam->setText(0, "camera");
	itemCam->setText(1, "calibrator");
	itemCam->setText(2, QString::number((int)cameraCalibData.getSamples().size()));

	itemIntr->setText(0, "intr");
	itemIntr->setText(1, "intr mat");
	QString strIntr;
	vecToStr(cameraCalibData.intr, 9, strIntr);
	itemIntr->setText(2, strIntr);

	itemDist->setText(0, "dist");
	itemDist->setText(1, "dist coeffs");
	QString strDist;
	vecToStr(cameraCalibData.dist, 5, strDist);
	itemDist->setText(2, strDist);

	itemReprojErr->setText(0, "err");
	itemReprojErr->setText(1, "reproj err");
	itemReprojErr->setText(2, QString::number(cameraCalibData.reproj_err));

	itemImgSize->setText(0, "imgSz");
	itemImgSize->setText(1, "image size");
	itemImgSize->setText(2, QString::number(cameraCalibData.imgSize.width)	+
							QString("x")									+
							QString::number(cameraCalibData.imgSize.height));


	/***********************************************************
	 * LED branch
	 ***********************************************************/

	itemLEDs->setText(0, "LEDs");
	itemLEDs->setText(1, "calibrator");
	itemLEDs->setText(2, QString::number((int)LEDCalibData.size()));

	size_t sz = LEDItems.size();
	for(size_t i = 0; i < sz; ++i) {

		// reference to the current calib container
		calib::LEDCalibContainer &container = LEDCalibData[i];

		// convert values to QStrings
		QString strCs = QString::number(container.circleSpacing);
		QString strLEDPos = 
					QString::number(container.LED_pos[0]) + QString(" ") +
					QString::number(container.LED_pos[1]) + QString(" ") +
					QString::number(container.LED_pos[2]);

		// reference to the current item container
		LEDItemContainer *curItem = LEDItems[i];

		curItem->itemLED->setText(0, "LED");
		curItem->itemLED->setText(1, "calibrator");
		curItem->itemLED->setText(2, QString::number((int)container.getSamples().size()));

		curItem->itemCircleSpacing->setText(0, "cs");
		curItem->itemCircleSpacing->setText(1, "circle spacing");
		curItem->itemCircleSpacing->setText(2, strCs);

		curItem->itemLEDPos->setText(0, "pos");
		curItem->itemLEDPos->setText(1, "LED position");
		curItem->itemLEDPos->setText(2, strLEDPos);

	}

}


void ResultItem::adjustImageIDs() {

	// initial values
	idImgLED = idImgCam = 0;

	{

		QDir camDir(calibDir + QString("/") + QString(camCalibImgDir));
		QStringList filters;
		filters << "*.jpeg" << "*.jpg";
		camDir.setNameFilters(filters);

		QStringList list = camDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

		int max = -1;
		int sz = list.count();

		for(int i = 0; i < sz; ++i) {

			// get the current file
			QString &curFile = list[i];

			// get the position of the dot
			int indexOfDot = curFile.indexOf('.');

			if(indexOfDot == -1) {
				continue;
			}

			// example 11.jpg >> 11
			curFile.truncate(indexOfDot);

			bool ok;
			int num = curFile.toInt(&ok);
			if(ok) {

				max = num > max ? num : max;

			}

		}

		idImgCam = max + 1;

	}


	{

		QDir LEDDir(calibDir + QString("/") + QString(LEDCalibImgDir));
		QStringList filters;
		filters << "*.jpeg" << "*.jpg";
		LEDDir.setNameFilters(filters);

		QStringList list = LEDDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

		int max = -1;
		int sz = list.count();

		for(int i = 0; i < sz; ++i) {

			// get the current file
			QString &curFile = list[i];

			// get the position of the dot
			int indexOfDot = curFile.indexOf('.');

			if(indexOfDot == -1) {
				continue;
			}

			// example 11.jpg >> 11
			curFile.truncate(indexOfDot);

			bool ok;
			int num = curFile.toInt(&ok);

			if(ok) {

				max = num > max ? num : max;

			}

		}

		idImgLED = max + 1;

	}

}


void ResultItem::addCamCalibData(const calib::CameraCalibSample &sample, QImage *qimg) {

	QString qstr = genNextCamCalibImgFile();
	std::string str(qstr.toAscii());

	cameraCalibData.addSample(sample, str);

	qimg->save(qstr);

	itemCam->setText(2, QString::number((int)cameraCalibData.getSamples().size()));

}


void ResultItem::addLEDCalibData(const calib::LEDCalibSample &sample, QImage *qimg) {

	QString qstr = genNextLEDCalibImgFile();
	std::string str(qstr.toAscii());

	calib::LEDCalibContainer &curData = LEDCalibData[curLED];
	curData.addSample(sample, str);

	qimg->save(qstr);

	LEDItems[curLED]->itemLED->setText(2, QString::number((int)LEDCalibData[curLED].getSamples().size()));

}



bool ResultItem::save() {

	const QString path = calibDir + QString("/") + QString(calibFile);


	/*
	 *	do not save without permission. Permission may be granted if:
	 *		1. the file does not exist
	 *		2. the file exists and the user gives permission
	 */
	bool b_save = false;

	if(QFile(path).exists()) {

		// let the user choose the save path
		QMessageBox msgBox;
		QString txt;
		txt = QString("The file ") + path + QString(" exists");
		msgBox.setText(txt);
		msgBox.setInformativeText("Overwrite?");
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();

		switch(ret) {

			// Save was clicked
			case QMessageBox::Save: {

				b_save = true;

				break;
			}


			// Cancel was clicked
			case QMessageBox::Cancel: {

				break;

			}


			// should never be reached
			default: {

				break;

			}

		}

	}

	else {

		b_save = true;

	}


	if(b_save) {

		CalibDataWriter writer;

		if(writer.create(path.toStdString())) {

			// save the data
			if(!writer.writeCameraData(cameraCalibData)) {
				writer.close();
				return false;
			}

			if(!writer.writeLEDData(LEDCalibData)) {
				writer.close();
				return false;
			}

		}

		writer.close();

	}

	return true;

}



bool ResultItem::load() {

	// first remove all previous items and data
	deleteLEDItems();
	cameraCalibData.clear();
	LEDCalibData.clear();


	const QString path = calibDir + QString("/") + calibFile;


	CalibDataReader rder;

	if(rder.create(path.toStdString())) {

		// read the data
		if(!rder.readCameraContainer(cameraCalibData)) {
			return false;
		}

		if(!rder.readLEDContainers(LEDCalibData)) {

			return false;
		}

		// make sure that there is room for at least one LED
		if(LEDCalibData.size() == 0) {

			LEDCalibData.push_back(calib::LEDCalibContainer());

		}


		QString name_item;
		QStringList strlist = calibDir.split('/');

		if(strlist.count() > 0) {
			name_item = strlist.last();
		}
		else {
			name_item = path;
		}

		this->setText(0, name_item);

	}
	else {
		return false;
	}


	createLEDItems();

	updateTree();

	selectLED(LEDItems[0]->itemLED);

	return true;

}


QString ResultItem::genNextCamCalibImgFile() {

	QString res = calibDir + QString("/") + camCalibImgDir;
	res.append(QString("/") + QString::number(idImgCam) + QString(".jpg"));

	++idImgCam;

	return res;

}


QString ResultItem::genNextLEDCalibImgFile() {

	QString res = calibDir + QString("/") + LEDCalibImgDir;
	res.append(QString("/") + QString::number(idImgLED) + QString(".jpg"));

	++idImgLED;

	return res;

}


QString ResultItem::getFileName() {

	QString res = calibDir + QString("/") + calibFile;

	return res;

}


bool ResultItem::calibrateCamera() {

	bool bCalibOk = calib::CameraCalibrator::calibrateCamera(cameraCalibData);

	if(bCalibOk) {
        updateTree();
	}

	return bCalibOk;

}


bool ResultItem::calibrateLED() {

	Camera cam = cameraCalibData.makeCameraObject();

	bool bCalibOk = calib::calibrateLED(LEDCalibData[curLED], cam);

	if(bCalibOk) {
		updateTree();
	}

	return bCalibOk;

}


void ResultItem::clearCamCalibContainer() {

	cameraCalibData.clear();
	updateTree();

}


void ResultItem::clearLEDCalibContainer() {

	LEDCalibData[curLED].clear();
	updateTree();

}


void ResultItem::addNewLED() {

	// place an empty container to the calibration data vector
	LEDCalibData.push_back(calib::LEDCalibContainer());

	// make the new LED current
	curLED = LEDCalibData.size() - 1;

	// make a new corresponding LEDItemContainer
	LEDItems.push_back(new LEDItemContainer(true));
	itemLEDs->insertChild(LEDItems.size() - 1, LEDItems.back()->itemLED);

	updateTree();

	selectLED(LEDItems.back()->itemLED);

}


void ResultItem::deleteLED(const QTreeWidgetItem *itemLED) {

	// get the index in the list
	int index = getLEDCalibIndexByLED(itemLED);

	// get the item container iterator..
	std::vector<LEDItemContainer *>::iterator it = LEDItems.begin() + index;

	// ...deallocate memory
	delete *it;

	// ...and erase from the list
	LEDItems.erase(it);

	std::vector<calib::LEDCalibContainer>::iterator itLEDCalib = LEDCalibData.begin() + index;
	LEDCalibData.erase(itLEDCalib);

	// ensure that there is always at least 1 container in the vector
	if(LEDCalibData.size() == 0) {

		LEDCalibData.push_back(calib::LEDCalibContainer());

		// item container stuff
		LEDItems.push_back(new LEDItemContainer(true));
		itemLEDs->addChild(LEDItems.back()->itemLED);

	}


	selectLED(LEDItems.front()->itemLED);

	updateTree();

}


void ResultItem::deleteCameraSample(int index) {

	cameraCalibData.deleteSample(index);

	itemCam->setText(2, QString::number((int)cameraCalibData.getSamples().size()));

}


void ResultItem::deleteLEDSample(int index) {

	LEDCalibData[curLED].deleteSample(index);

	LEDItems[curLED]->itemLED->setText(2, QString::number((int)LEDCalibData[curLED].getSamples().size()));

}


void ResultItem::deleteCameraSamples(std::list<int> &indices) {

	cameraCalibData.deleteSamples(indices);

	itemCam->setText(2, QString::number((int)cameraCalibData.getSamples().size()));

}



void ResultItem::deleteLEDSamples(std::list<int> &indices) {

	LEDCalibData[curLED].deleteSamples(indices);

	LEDItems[curLED]->itemLED->setText(2, QString::number((int)LEDCalibData[curLED].getSamples().size()));

}




LEDItemContainer::LEDItemContainer() {
	itemLED = itemCircleSpacing = itemLEDPos = NULL;
}


LEDItemContainer::LEDItemContainer(bool b_allocate) {

	if(b_allocate) {
		allocate();
	}
	else {
		itemLED = itemCircleSpacing = itemLEDPos = NULL;
	}

}


LEDItemContainer::LEDItemContainer(const LEDItemContainer &) {

	allocate();

}


LEDItemContainer &LEDItemContainer::operator= (const LEDItemContainer &other) {

	if(this != &other) {

		deallocate();
		allocate();

	}

	return *this;

}


LEDItemContainer::~LEDItemContainer() {

	deallocate();

}


void LEDItemContainer::allocate() {

	itemLED				= new QTreeWidgetItem();
	itemCircleSpacing	= new QTreeWidgetItem(itemLED);
	itemLEDPos			= new QTreeWidgetItem(itemLED);

}


void LEDItemContainer::deallocate() {

	if(itemLED != NULL) {

		// remove children
		itemLED->removeChild(itemCircleSpacing);
		itemLED->removeChild(itemLEDPos);

		// remove this from the parent
		if(itemLED->parent() != NULL) {
			itemLED->parent()->removeChild(itemLED);
		}

	}

	// deallocate memory
	delete itemLED;
	delete itemCircleSpacing;
	delete itemLEDPos;

	// set to NULL
	itemLED = itemCircleSpacing = itemLEDPos = NULL;

}

