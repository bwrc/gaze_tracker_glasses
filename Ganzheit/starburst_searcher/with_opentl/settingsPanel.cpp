#include <sstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "settingsPanel.h"
#include "trackBar.h"

static LocalTrackerSettings *settings;

class SettingsTrackBar : public TrackBar {
	public:
		SettingsTrackBar(const char * settingName, cv::Rect& position) : TrackBar(settingName, position, 0, 0, 0, false)
		{
			this->textChanged();
		}

		/**************************************************************
		 * Modified keyboard event handler (allows browsing the
		 * settings variables)
		 *************************************************************/

		void keyboardEvent(int fkey)
		{

			int key = fkey & 0xffff;

			if(!this->edit) {
				return;
			}

			if(key == KEY_UP || key == KEY_DOWN) {
				// Are we already browsing the list?
				if(this->setting == (std::list<TrackerSetting>::iterator)NULL) {
					// Nope, start from the begin
					this->setting = settings->getFirstObject();
				} else {

					// Yes. Move to previous
					if(key == KEY_UP) {
						// Is this the first object?
						if(this->setting == settings->getFirstObject()) {
							// Yes. Go to the last object
							this->setting = settings->getLastObject();
							this->setting--;
						} else {
							// No. Just take the previous object
							this->setting--;
						}
					}

					// Yes. Handle down key
					if(key == KEY_DOWN) {

						// Always go to the next element
						this->setting++;						

						// Check for last iterator (note! last iterator is null)
						if(this->setting == settings->getLastObject()) {
							this->setting = settings->getFirstObject();
						}
					}
				}

				// Update the text
				this->setText(this->setting->getName());
				return;

			} else {
				// List handling completed
				this->setting = (std::list<TrackerSetting>::iterator)NULL;
			}

			// Call the keyboard event handler.
			TrackBar::keyboardEvent(key);
		}

		/**************************************************************
		 * Update the variables after the name has been changed
		 *************************************************************/

		void textChanged()
		{
			// Check whether the settings exist
			this->setting = settings->getSettingObject(this->getText());
			if(this->setting != (std::list<TrackerSetting>::iterator)NULL) {

				/* Setting is available.. load min, max, value
				 * and enable the object */

				this->setMin(this->setting->getMin());
				this->setMax(this->setting->getMax());
				this->setValue(this->setting->getValue());
				this->setEnabled(true);
			} else {
				/* Setting is not available.. just disable this
				 * object */
				this->setEnabled(false);
			}
		}

		/**************************************************************
		 * Update the value of a setting after the trackbar value has
		 * changed
		 *************************************************************/

		void valueChanged()
		{
			// Check that the current setting is valid

			if(this->setting !=
				(std::list<TrackerSetting>::iterator)NULL) {

				setting->setValue(this->getValue());
			}
		}
	private:

		std::list<TrackerSetting>::iterator setting;

};

/******************************************************************************
 * Mouse callback for this window. This function determines which object has
 * been clicked and delivers the requrest for the correct object.
 *****************************************************************************/

static void mouse_callback( int event, int x, int y, int flags, void* param )
{

	SettingsPanel * _this = (SettingsPanel *)param;
	cv::Point point(x, y);

	// Check that the mouse have been clicked
	if(event != CV_EVENT_LBUTTONDOWN)
		return;

	// Go through each UI object
	std::list<UIObject *>::iterator uiObjectIt = _this->uiObjects.begin();
	for(; uiObjectIt != _this->uiObjects.end(); uiObjectIt++) {
		UIObject * uiObject = *uiObjectIt;

		// Is the click inside the object?
		const cv::Rect rect = uiObject->getPosition();
		if(point.inside(rect)) {

			/******************************************************
			 * Handle object removal
			 *****************************************************/

			cv::Rect removeButton(rect.x + rect.width - 10,
				rect.y + 10, 10, 10);

			if(point.inside(removeButton)) {
				if(uiObject != _this->lastObject) {
					_this->uiObjects.erase(uiObjectIt);
					_this->organizeUIObjects();
				}
				break;

			}

			/******************************************************
			 * Activate the object if it is not already active
			 *****************************************************/

			if(_this->activeObject != uiObject) {

				// Deactivate the old object
				if(_this->activeObject != NULL) {
					_this->activeObject->deactivateEvent();
				}

				// Activate the object
				_this->activeObject = uiObject;
				uiObject->activateEvent();

				/* If the last trackbar was activated,
				 * add a new object below the old last object*/

				if(uiObject == _this->lastObject) {
					cv::Rect pos =_this->lastObject->getPosition();
					pos.y += pos.height + 10;
					UIObject *obj = new SettingsTrackBar("", pos);
					_this->uiObjects.push_back(obj);
					_this->lastObject = obj;

				}

			}

			// Deliver the click to the object
			uiObject->clickEvent(cv::Point(x - rect.x, y - rect.y));

			break;
		}
	}

	_this->draw();

}

/******************************************************************************
 * Keyboard handler. This function determines which object should receive the
 * keypress according to the active object.
 *
 * HACK! This is a public member of this class as opencv does not support
 * event-based keyboard handling
 *****************************************************************************/

void SettingsPanel::keyboardEvent(int key)
{
	if(this->activeObject != NULL) {
		this->activeObject->keyboardEvent(key);
	}

	this->draw();
}

/******************************************************************************
 * Draw a remove button for given object
 *****************************************************************************/

void SettingsPanel::drawRemoveButton(cv::Mat& window, UIObject* uiObject)
{
	const cv::Rect rect = uiObject->getPosition();

	// Draw the box
	cv::Rect button(rect.x + rect.width - 10, rect.y + 10, 10, 10);
	cv::rectangle(window, button, 255);

	// Draw the cross
	cv::line(window, cv::Point(button.x, button.y),
		cv::Point(button.x + button.width - 1,
		button.y + button.height - 1), 255);

	cv::line(window, cv::Point(button.x + button.width - 1, button.y),
		cv::Point(button.x, button.y + button.height - 1), 255);
}

/******************************************************************************
 * Organize the UI objects. The implemented organization simply forces all
 * objects into single column.
 *****************************************************************************/

void SettingsPanel::organizeUIObjects()
{

	std::list<UIObject *>::iterator uiObjectIt = this->uiObjects.begin();

	int pos_y = 0;
	for(; uiObjectIt != this->uiObjects.end(); uiObjectIt++) {
		UIObject * uiObject = *uiObjectIt;

		cv::Rect rect = uiObject->getPosition();
		rect.y = pos_y;
		rect.x = 10;
		uiObject->setPosition(rect);

		pos_y = rect.y + rect.height + 10;
		
	}
}

/******************************************************************************
 * Draw the visible objects of this window
 *****************************************************************************/

void SettingsPanel::draw()
{

	std::list<UIObject *>::iterator uiObjectIt = this->uiObjects.begin();

	// Determine the minimum height of the window
	int height = 0, width = 0;
	for(; uiObjectIt != this->uiObjects.end(); uiObjectIt++) {
		UIObject * uiObject = *uiObjectIt;

		const cv::Rect rect = uiObject->getPosition();
		width = std::max(width, rect.x + rect.width + 10);
		height = std::max(height, rect.y + rect.height + 10);
	}

	cv::Mat window = cv::Mat::zeros(height, width, CV_8UC1);

	// Go through each object and call draw method
	for(uiObjectIt = this->uiObjects.begin();
		uiObjectIt != uiObjects.end(); uiObjectIt++) {

		UIObject * uiObject = *uiObjectIt;

		// Draw the object
		uiObject->draw(window);

		// Draw remove button if this is not the last object
		if(uiObject != this->lastObject) {
			drawRemoveButton(window, uiObject);
		}
	}

	// Show the updated form
	const char * windowName = this->windowName.c_str();
	imshow(windowName, window);	
}

/******************************************************************************
 * Constructor.. just create the first trackbar
 *****************************************************************************/

SettingsPanel::SettingsPanel()
{


settings = new LocalTrackerSettings();

	this->windowName = std::string("Settings");
	this->panelAvailable = false;
	this->activeObject = NULL;

	cv::Rect pos(10, 0, 500, 50);
	UIObject *obj = new SettingsTrackBar("", pos);
	this->uiObjects.push_back(obj);
	this->lastObject = obj;
}

/******************************************************************************
 * Remove all trackbar objects
 *****************************************************************************/

void SettingsPanel::removeUIObjects()
{
	std::list<UIObject *>::iterator uiObjectIt = this->uiObjects.begin();

	for(; uiObjectIt != this->uiObjects.end(); uiObjectIt++) {
		delete *uiObjectIt;
	}
	uiObjects.clear();
}

/******************************************************************************
 * Destructor. Remove all trackbar objects
 *****************************************************************************/

SettingsPanel::~SettingsPanel()
{
delete settings;
	removeUIObjects();
}

/******************************************************************************
 * Load settings
 *****************************************************************************/

void SettingsPanel::setSettings(const LocalTrackerSettings& ext_settings)
{
	delete settings;
	settings = new LocalTrackerSettings(ext_settings);
}

/******************************************************************************
 * Show the panel and register mouse click callback
 *****************************************************************************/

void SettingsPanel::show()
{
	const char * window = this->windowName.c_str();
	cv::namedWindow(window, CV_WINDOW_AUTOSIZE);

	cvSetMouseCallback(window, mouse_callback, this);

	this->draw();
	
}

/******************************************************************************
 * Return settings
 *****************************************************************************/

LocalTrackerSettings *SettingsPanel::getSettings()
{
	return settings;
}

/******************************************************************************
 * Store current view into given SettingsIO object
 *****************************************************************************/

void SettingsPanel::save(SettingsIO& settingsIO)
{
	std::list<std::string> uiSettingNames;

	// Construct a string list of the current objects
	std::list<UIObject *>::iterator uiObjectIt = this->uiObjects.begin();
	for(; uiObjectIt != uiObjects.end(); uiObjectIt++) {

		UIObject * uiObject = *uiObjectIt;

		// Skip the last object
		if(uiObject != this->lastObject) {
			uiSettingNames.push_back(uiObject->name());
		}
	}

	// Save string list into the file
	settingsIO.setFields(std::string("SettingsPanel"),
		std::string("trackbar"), uiSettingNames);



}

/******************************************************************************
 * Load the UI from the given settingsIO object
 *****************************************************************************/

void SettingsPanel::open(SettingsIO& settingsIO)
{
	std::list<std::string> uiSettingNames;
	settingsIO.getFields(std::string("SettingsPanel"), std::string("trackbar"),
		uiSettingNames);

	// Load name of the objects from the file
	std::list<std::string>::iterator settingName = uiSettingNames.begin();

	// Remove current objects
	this->removeUIObjects();

	/* Place objects anywhere in the view... objects will be reorganized
	 * after loading them */
	cv::Rect pos(10, 0, 500, 50);
	for(; settingName != uiSettingNames.end(); settingName++) {
		
		this->uiObjects.push_back(
			new SettingsTrackBar(settingName->c_str(), pos));
	}

	// Create the last object
	UIObject *obj = new SettingsTrackBar("", pos);
	this->uiObjects.push_back(obj);
	this->lastObject = obj;

	// Finally, organize the objects
	this->organizeUIObjects();

}

