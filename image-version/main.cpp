
#include "mainwindow.h"
#include "../xlog.h"

#define WINWIDTH 1200
#define WINHEIGHT 800

int main(int argc, char **argv)
{
	srand((unsigned int)time(NULL));

	nanogui::init();

	nanogui::ref<MainWindow> app =
		new MainWindow(WINWIDTH, WINHEIGHT, "VoroApprox Image - Xiao Yanyang (yanyangxiaoxyy@gmail.com)");
	app->drawAll();
	app->setVisible(true);

	nanogui::mainloop();

	nanogui::shutdown();

	return 0;
}

