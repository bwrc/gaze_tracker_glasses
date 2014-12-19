#include "SettingsPanel.h"
#include "trackerSettings.h"
#include <SDL/SDL.h>
#include <sys/time.h>


namespace gui {


    /*******************************************************
     * class SettingsPanel
     *******************************************************/

    SettingsPanel::SettingsPanel(const View &_v,
                                 SettingsIO& settingsIO)
        : GLWidget(_v) {

        // initialise the local settings with the IO object
        m_settings.open(settingsIO);

        // image must be updated initially
        m_bUpdateImage = true;


        /************************************************************
         * Collect modifiable settings for the UI
         ************************************************************/
        std::list<std::string> listUISettingNames;
        settingsIO.getFields(std::string("SettingsPanel"), std::string("trackbar"),
                             listUISettingNames);

        // Load name of the objects from the file
        std::list<std::string>::iterator settingName = listUISettingNames.begin();
        for(; settingName != listUISettingNames.end(); settingName++) {
		
            list_settings.push_back(*settingName);

        }


        // create the image
        const int w = _v.w;
        const int h = _v.h;
        img = cv::Mat(h, w, CV_8UC3);


        // create the trackbar
        int cy = img.rows / 2;
        cv::Mat img_bar = cv::Mat(img, cv::Rect(img.cols / 2, cy-20, img.cols / 2, 30));
        trackbar = new Trackbar(img_bar);

        pos = 0;

        prepare_bmp();

        // create the texture
        glGenTextures(1, &texture);

        /* Create Nearest Filtered Texture */
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    }


    SettingsPanel::~SettingsPanel() {
        glDeleteTextures(1, &texture);

        delete trackbar;
    }


    void SettingsPanel::selectNext() {

        int sz = (int)list_settings.size();

        if(pos + 1 >= sz) {
            return;
        }

        ++pos;

        prepare_bmp();
    }


    void SettingsPanel::selectPrevious() {

        if(pos - 1 < 0) {
            return;
        }

        --pos;

        prepare_bmp();
    }


    void SettingsPanel::prepare_bmp() {

        // clear the image
        cv::rectangle(img, cv::Point(0, 0), cv::Point(img.cols, img.rows), cv::Scalar(0, 0, 0), CV_FILLED);

        int cy = img.rows / 2;	// y centre
        int stepy = 30;			// step between strings

        int x = 10;
        int y = cy - 2 * stepy;


        /**************************************************************************
         * draw a coloured area behind the selected text
         **************************************************************************/
        cv::rectangle(img, cv::Point(0, cy-20), cv::Point(img.cols, cy+10), cv::Scalar(40, 40, 40), CV_FILLED);


        if(list_settings.size() > 0) {

            /**************************************************************************
             * get the selected settings object, ask for specs and draw a trackbar
             **************************************************************************/
            std::list<TrackerSetting>::iterator it = m_settings.getSettingObject(list_settings[pos]);
            trackbar->changeObj(it);
            trackbar->draw();


            size_t sz_list_settings = list_settings.size();

            size_t tmp_pos = pos - 2;

            // maximum of five will be displayed
            for(int i = 0; i < 5; ++i) {

                if(tmp_pos >= 0 && tmp_pos < sz_list_settings) {

                    cv::putText(img,						// destination image
                                list_settings[tmp_pos],		// string
                                cv::Point(x, y),			// where to draw
                                cv::FONT_HERSHEY_SIMPLEX,	// font face
                                0.7,						// font scale
                                cv::Scalar(245, 245, 245),	// colour
                                1,							// thickness
                                CV_AA);						// antialiased

                }

                y += stepy;
                ++tmp_pos;
            }

        }

        m_bUpdateImage = true;

    }


    void SettingsPanel::update() {
        trackerSettings.set(m_settings);
    }


    void SettingsPanel::draw() {

        GLWidget::draw();

        glEnable(GL_TEXTURE_2D);

        // draw the results to the lower view
        glBindTexture(GL_TEXTURE_2D, texture);

        if(m_bUpdateImage) {
            glTexImage2D(GL_TEXTURE_2D, 0, 3, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
            m_bUpdateImage = false;
        }

        glBegin(GL_QUADS);
		glTexCoord2d(0.0, 0.0); glVertex2d(0,			img.rows-1);	// upper left
		glTexCoord2d(0.0, 1.0); glVertex2d(0,			0);				// lower left
		glTexCoord2d(1.0, 1.0); glVertex2d(img.cols-1,	0);				// lower right
		glTexCoord2d(1.0, 0.0); glVertex2d(img.cols-1,	img.rows-1);	// upper right
        glEnd();

        glDisable(GL_TEXTURE_2D);

    }


    void SettingsPanel::incVal(int nDirection) {
        m_bUpdateImage = true;
        trackbar->incVal(nDirection);
    }


    bool SettingsPanel::handleKeys(const unsigned char *keystate) {

        static const double DEBOUNCE1 = 100.0;

        struct timeval tnow;
        static struct timeval t1 = tnow;


        gettimeofday(&tnow, NULL);
        long usec = tnow.tv_usec - t1.tv_usec;
        long sec = tnow.tv_sec - t1.tv_sec;
        const double diff_millis1 = 1000.0 * sec + usec / 1000.0;


        bool b_changed = false;

        if(diff_millis1 > DEBOUNCE1) {

            if(keystate[SDLK_DOWN]) {

                selectNext();
            }
            else if(keystate[SDLK_UP]) {

                selectPrevious();
            }

            t1 = tnow;

        }



        static const double DEBOUNCE2 = 40.0;
        static struct timeval t2 = tnow;

        usec = tnow.tv_usec - t2.tv_usec;
        sec = tnow.tv_sec - t2.tv_sec;
        const double diff_millis2 = 1000.0 * sec + usec / 1000.0;

        if(diff_millis2 > DEBOUNCE2) {

            if(keystate[SDLK_LEFT]) {

                b_changed = true;

                incVal(-1);
            }
            else if(keystate[SDLK_RIGHT]) {

                b_changed = true;

                incVal(1);
            }

            t2 = tnow;

        }

        return b_changed;

    }



    /*******************************************************
     * class Trackbar
     *******************************************************/

    Trackbar::Trackbar(cv::Mat &_img) {

        // does not copy, creates a header to the given data
        img = _img;
    }


    Trackbar::~Trackbar() {

    }


    void Trackbar::changeObj(std::list<TrackerSetting>::iterator _settings_obj) {

        // copy the settings object
        settings_obj = _settings_obj;

    }


    void Trackbar::draw() {

        const double max = settings_obj->getMax();
        const double val = settings_obj->getValue();

        const int w = img.cols;
        const int h = img.rows;

        const int w_bar = 0.8*w;

        const int end_pix = (int)(val / max * w_bar + 0.5);

        // the background
        cv::rectangle(img, cv::Point(0, 0), cv::Point(w-1, h-1), cv::Scalar(0, 200, 0), CV_FILLED);

        cv::line(img, cv::Point(w_bar, 0), cv::Point(w_bar, h), cv::Scalar(0, 0, 0), 1);

        // the indicator
        cv::rectangle(img, cv::Point(0, 3), cv::Point(end_pix, h-1-3), cv::Scalar(80, 40, 80), CV_FILLED);


        char str[128];
        if(val < 1) {
            sprintf(str, "%.2f (x 100)", val*100);
        }
        else {
            sprintf(str, "%.2f", val);
        }

        cv::putText(img,                            // destination image
                    std::string(str),               // string
                    cv::Point(w_bar, img.rows/2+2), // where to draw
                    cv::FONT_HERSHEY_SIMPLEX,       // font face
                    0.3,                            // font scale
                    cv::Scalar(255, 0, 0),          // colour
                    1,                              // thickness
                    CV_AA);                         // antialiased

    }


    void Trackbar::incVal(int nDirection) {

        const double min = settings_obj->getMin();
        const double max = settings_obj->getMax();
        const double val = settings_obj->getValue();
        const double dInc = nDirection * settings_obj->getInc();

        const double dNewVal = val + dInc;

        if(dNewVal >= min && dNewVal <= max) {

            settings_obj->setValue(dNewVal);

            draw();

        }

    }


} // end of "namespace gui"

