
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "nanogui/screen.h"
#include "nanogui/window.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "voroapprox.h"
#include "render.h"

class MainWindow : public nanogui::Screen
{
	struct Parameters
	{
		unsigned char *image = NULL;
		int width;
		int height;
		int channel;

		unsigned char *approx = NULL;
		float approxScale;

		int degree;
		int sitesNumber;
		int iteration;
		double stepScale;

		bool showImage;
		bool showSites;
		bool showVoronoi;
		bool showApprox;

		// GL
		glm::mat4 modelMat;
		glm::mat4 viewMat;
		glm::mat4 projectionMat;
		bool translating;

		bool resizingPoint = false;
		float pointSize = 10.f;
		bool resizingLine = false;
		float lineWidth = 5.0f;
	};

protected:
	nanogui::Window               *_panel;
	VoroApprox                    *_voroApprox;
	Render                        *_render;

	// parameter
	Parameters                     _params;

	nanogui::Label                *_fpsLabel;
	double                         _startTime;
	int                            _frameCount;

public:
	MainWindow(int w, int h, const char *title);
	~MainWindow();

	virtual void drawContents();
	// events
	virtual bool resizeEvent(const Eigen::Vector2i& size);
	virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel);
	virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers);
	virtual bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers);
	virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

protected:
	void init_widgets();
	void init_file_widget();
	void init_parameter_widget();
	void init_algorithm_widget();
	void init_render_widget();

	// callbacks
	void init_mvp();
	void load_image(const char* fileName);
	void save_approximation(const char *fileName);
	void load_sites(const char *fileName);
	void save_sites(const char *fileName);
};

#endif
