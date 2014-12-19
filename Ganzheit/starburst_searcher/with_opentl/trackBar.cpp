#include <iostream>
#include <list>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "trackBar.h"

TrackBar::TrackBar(const char *settingName, cv::Rect& position,
			double min, double max, double value, bool barEnabled)
{
	this->text = std::string(settingName);
	this->position = position;
	this->min = min;
	this->max = max;
	this->value = value;
	this->barEnabled = barEnabled;
	this->edit = false;

}

void TrackBar::keyboardEvent(int key)
{
	key &= 0xFF;

	if(!this->edit) {
		return;
	}


	if(key == '\b') {
		this->text = this->text.substr(0, this->text.length() - 1);
	} else if(key == '\n') {
		this->edit = false;
		this->textChanged();
	} else if((key >= 'A' && key <= 'Z') || key == '_') {
		std::stringstream textstream;
		textstream << this->text << (char)key;
		textstream >> this->text;
	}
}

void TrackBar::deactivateEvent()
{
	if(this->edit) {
		this->textChanged();
		this->edit = false;
	}
}

void TrackBar::clickEvent(const cv::Point& point)
{
	cv::Rect rect(5, 0, this->position.width, 25);

	if(point.inside(rect)) {
		if(!this->edit) {
			this->edit = true;
		}
		return;
	}

	if(this->edit) {
		this->edit = false;
		this->textChanged();
		return;
	}

	if(!this->barEnabled) {
		return;
	}

	rect = cv::Rect(5, 25, this->position.width - 10,
		this->position.height - 10);

	if(point.inside(rect)) {
		this->value = this->min + ((double)(point.x - 5) /
			(double)rect.width) * (this->max - this->min);
		this->valueChanged();
	}
}

void TrackBar::draw(cv::Mat& window)
{
	/**********************************************************************
	 * Write the name, value, min and max
	 *********************************************************************/

	std::stringstream nameLine;
	nameLine << this->text;

	if(this->edit) {
		nameLine << "_";
	} else if(this->barEnabled) {
		nameLine << " = " << this->value <<
			" (" << this->min << " - " << this->max << ")";
	}

	cv::putText(window, nameLine.str().c_str(),
		cv::Point(this->position.x, this->position.y + 15),
		cv::FONT_HERSHEY_SIMPLEX, 0.5,
		cv::Scalar(255, 255, 255, 0), 1, false);

	/**********************************************************************
	 * Draw rectangles
	 *********************************************************************/

	// Draw the (outer) rectangle
	cv::Rect rect = this->position;
	rect.y += 25;
	rect.height -= 35;
	cv::rectangle(window, rect, cv::Scalar(255, 255, 255, 0), 1, false);

	if(!this->barEnabled) {
		return;
	}

	// Compute maximum value for the inner rectangle
	rect.x += 5;
	rect.y += 5;
	rect.height -= 10;
	rect.width -= 10;

	// Compute the current value for the inner rectangle
	rect.width = ((this->value - this->min) / (this->max - this->min)) *
		(rect.width);

	// Draw
	cv::rectangle(window, rect, cv::Scalar(255, 255, 255, 0), CV_FILLED,
		false);
}

