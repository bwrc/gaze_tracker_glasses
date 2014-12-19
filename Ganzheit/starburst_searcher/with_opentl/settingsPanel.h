#ifndef SETTINGS_PANE_H
#define SETTINGS_PANE_H

#include <iostream>
#include <list>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "trackerSettings.h"

#define KEY_UP		0xFF52
#define KEY_DOWN	0xFF54

class UIObject {

	public:

		virtual ~UIObject() {}

		virtual void clickEvent(const cv::Point& point) {};
		virtual void keyboardEvent(int key) {};
		virtual void activateEvent() {};
		virtual void deactivateEvent() {};
		virtual const std::string& name() = 0;

		virtual const cv::Rect& getPosition() = 0;
		virtual void setPosition(const cv::Rect& position) = 0;
		virtual void draw(cv::Mat& window) = 0;

};

class SettingsPanel {
	public:
		SettingsPanel();
		~SettingsPanel();

		void keyboardEvent(int key);

		void setSettings(const LocalTrackerSettings& settings);
		void save(SettingsIO& settings);
		void open(SettingsIO& settings);
		void show();
		LocalTrackerSettings *getSettings();
		void draw();

		void organizeUIObjects();
		std::list<UIObject *> uiObjects;
		UIObject * activeObject;
		UIObject * lastObject;

	private:
		void removeUIObjects();
		std::string windowName;
		bool panelAvailable;
		void drawRemoveButton(cv::Mat& window, UIObject* object);
};

#endif // SETTINGSPANE_H
