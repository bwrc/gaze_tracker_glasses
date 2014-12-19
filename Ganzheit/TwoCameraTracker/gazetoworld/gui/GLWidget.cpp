#include "GLWidget.h"


namespace gui {


GLWidget::GLWidget(const View &_view) {

	view = _view;

}


GLWidget::~GLWidget() {


}


void GLWidget::draw() {

	select_view(view);

}


}

