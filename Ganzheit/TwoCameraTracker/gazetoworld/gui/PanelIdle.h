#ifndef PANEL_IDLE_H
#define PANEL_IDLE_H


#include "GLWidget.h"


namespace gui {


class PanelIdle : public GLWidget {

	public:

		PanelIdle(const View &_view);
		~PanelIdle();

		void draw();


	private:


};


}


#endif

