#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H


#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "settingsIO.h"
#include "localTrackerSettings.h"
#include "GLWidget.h"


namespace gui {


    // forward declaration of the trackbar
    class Trackbar;


    class SettingsPanel : public GLWidget {

	public:
		SettingsPanel(const View &_v, SettingsIO &_settings);

		~SettingsPanel();

		void selectNext();
		void selectPrevious();

		cv::Mat &getImage() {return img;}


		LocalTrackerSettings *getSettings() {return &m_settings;}

		void update();

		void incVal(int nDirection);


		// inherited from GLWidget
		void draw();

		/* Handles keypresses. Returns true if settings were altered. */
		bool handleKeys(const unsigned char *keystate);

	private:

		/*
		 * When a new item has been chosen, this will be called
		 */
		void prepare_bmp();

		Trackbar *trackbar;

		cv::Mat img;

		std::vector<std::string> list_settings;
		int pos;

		GLuint texture;

        bool m_bUpdateImage;

		LocalTrackerSettings m_settings;
    };


    class Trackbar {
	public:
		Trackbar(cv::Mat &_img);
		~Trackbar();

		void draw();

		void changeObj(std::list<TrackerSetting>::iterator _settings_obj);
		void incVal(int nDirection);

	private:

		// a settings object, for the selected setting
		std::list<TrackerSetting>::iterator settings_obj;


		// an image header, the drawing area
		cv::Mat img;
    };


}


#endif

