#ifndef RESULTITEM_H
#define RESULTITEM_H



#include "LEDCalibrator.h"
#include "CameraCalibrator.h"
#include <QTreeWidgetItem>


/* For each LED item there must be these QTreeWidgetItems */
class LEDItemContainer {

	public:

		LEDItemContainer();

		LEDItemContainer(bool allocate);


		/* Copy contructor */
		LEDItemContainer(const LEDItemContainer &other);

		/* Assignment operator */
		LEDItemContainer & operator= (const LEDItemContainer &other);

		~LEDItemContainer();


		QTreeWidgetItem *itemLED;
		QTreeWidgetItem *itemCircleSpacing;
		QTreeWidgetItem *itemLEDPos;

	private:

		void allocate();
		void deallocate();

};




/****************************************************************
 *
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
 *
 ****************************************************************/
class ResultItem : public QObject, public QTreeWidgetItem {

    Q_OBJECT

	public:


		/* The possible calibration modes */
		enum CalibrationMode {

			ModeCamera,
			ModeLED

		};

		/* Which item was changed, used in the signal calibItemChanged */
		enum ChangedItem {

			ItemLED,
			ItemCamera

		};



		ResultItem();

		~ResultItem();

		/*
		 * Creates directories and stores the given object points.
		 * Makes sure that there is at least 1 LED calibration
		 * container in the vector.
		 */
		bool init(const QString &_calibDir,
				  const std::vector<cv::Point3f> &camera_object_points,
				  const std::vector<cv::Point3f> &LED_object_points,
				  const cv::Size &imgSize);


		/* Add data to the camera */
		void addCamCalibData(const calib::CameraCalibSample &sample, QImage *qimg);


		/* Add data to the current LED */
		void addLEDCalibData(const calib::LEDCalibSample &sample, QImage *qimg);


		/* Add a new LED and make it the current LED */
		void addNewLED();

		/* Delete the given LED */
		void deleteLED(const QTreeWidgetItem *itemLED);

		/* Delete samples by index */
		void deleteCameraSample(int index);
		void deleteLEDSample(int index);


		/* Delete samples by indices */
		void deleteCameraSamples(std::list<int> &indices);
		void deleteLEDSamples(std::list<int> &indices);

		/* Save and load calibration stuff */
		bool save();
		bool load();


		/* Calibrate the camera using the given calibrator */
    bool calibrateCamera();

		/* Calibrate the current LED using the given calibrator */
		bool calibrateLED();


		/* Clear the camera calibration data container */
		void clearCamCalibContainer();

		/* Clear the current LED calibration data container */
		void clearLEDCalibContainer();

		/*
		 * Select the current LED based on the item. This function
		 * is called by MainWindow, when an item has been changed.
		 */
		void selectLED(QTreeWidgetItem *item);

		/* Is the camera calibarted or not */
		bool isCamCalibrated() {return cameraCalibData.isCalibrated();}

		/* Is the current LED calibarted or not */
		bool isLEDCalibrated() {return LEDCalibData[curLED].isCalibrated();}

		/*  */
		const calib::LEDCalibContainer &getLEDCalibContainer() const {return LEDCalibData[curLED];}

		/*  */
		const calib::CameraCalibContainer &getCamCalibContainer() const {return cameraCalibData;}

		/*
		 * Called by ControlPanel when this item or any of its
		 * decendants has been double clicked.
		 */
		void doubleClick(QTreeWidgetItem *item, int column);

		/* Get the calibration file name */
		QString getFileName();


		/*
		 * Called when a child of this item has been selected.
		 * Can be any of these:
		 *
		 *     Camera
		 *     intr
		 *     dist
		 *     err
		 *     imgSz
		 *
		 *     LEDs
		 *     LED
		 *     cs
		 *     pos
		 */
		void childSelected(QTreeWidgetItem *itemChild);

		CalibrationMode getCalibrationMode() {return calibMode;}


	signals:

		/* Emitted when either the Camera or an LED item has been selected */
		void calibItemChanged(int item);


	private:


		/* Generate a unique file name for the next camera calibration image */
		QString genNextCamCalibImgFile();

		/* Generate a unique file name for the next LED calibration image */
		QString genNextLEDCalibImgFile();

		void createLEDItems();
		void deleteLEDItems();


		int getLEDCalibIndexByItemCS(const QTreeWidgetItem *itemCs);
		int getLEDCalibIndexByLED(const QTreeWidgetItem *itemLEDTarget);


		/*
		 * Look into the image folders and find the largest image index.
		 * Set idImgLED and idImgCam to be one above the maxima.
		 */
		void adjustImageIDs();

		/*  */
		void updateTree();


		/*
		 * Contains the LED calibration samples and results. The init()
		 * method takes care that there is always at least 1 container
		 * available.
		 */
		std::vector<calib::LEDCalibContainer> LEDCalibData;

		/* Contains the camera calibration samples and results */
		calib::CameraCalibContainer cameraCalibData;

		/* The directory where all calibration stuff resides */
		QString calibDir;

		/* Used in giving names to the jpeg-image files */
		int idImgLED, idImgCam;


		/* Current LED being calibrated */
		int curLED;


		QTreeWidgetItem *itemCam;
		QTreeWidgetItem *itemIntr;
		QTreeWidgetItem *itemDist;
		QTreeWidgetItem *itemReprojErr;
		QTreeWidgetItem *itemImgSize;
		QTreeWidgetItem *itemLEDs;


		std::vector<LEDItemContainer *> LEDItems;

		CalibrationMode calibMode;

};


#endif

