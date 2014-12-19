#ifndef TRACKBAR_H
#define TRACKBAR_H

#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "settingsPanel.h"

class TrackBar : public UIObject {
	public:
		TrackBar(const char * settingName, cv::Rect& position, double min, double max, double value, bool barEnabled);


		virtual ~TrackBar() {}

		const std::string& name() { return this->text; }

		void setValue(double value) { this->value = value; }
		double getValue() { return this->value; }
		const std::string& getText() { return this->text; }
		void setText(const std::string& text) { this->text = text; }
		double getMin() { return this->min; }
		void setMin(double min) { this->min = min; }
		double getMax() { return this->max; }
		void setMax(double max) { this->max = max; }
		const cv::Rect& getPosition() { return position; }
		void setPosition(const cv::Rect& position) { this->position = position; }

		void setEnabled(bool enabled) { this->barEnabled = enabled; }

		void draw(cv::Mat& window);
		void clickEvent(const cv::Point& point);
		void keyboardEvent(int key);
		void deactivateEvent();

		virtual void valueChanged() {}
		virtual void textChanged() {}

	private:

		bool barEnabled;

		cv::Rect position;

		double min;
		double max;
		double value;

		std::string text;
	protected:
		bool edit;
};


#endif


