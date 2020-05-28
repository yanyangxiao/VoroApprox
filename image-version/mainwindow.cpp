
#include "mainwindow.h"
#include "nanogui/button.h"
#include "nanogui/checkbox.h"
#include "nanogui/textbox.h"
#include "nanogui/combobox.h"
#include "nanogui/slider.h"
#include "nanogui/label.h"
#include "nanogui/messagedialog.h"
#include "nanogui/layout.h"
#include "nanogui/opengl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../xlog.h"

MainWindow::MainWindow(int w, int h, const char *title)
	: nanogui::Screen(Eigen::Vector2i(w, h), std::string(title)),
	_voroApprox(NULL), _render(NULL)
{
	// check if your display card supports MSAA
	GLint bufs, samples;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
	glGetIntegerv(GL_SAMPLES, &samples);
	xlog("MSAA: buffers = %d, samples = %d", bufs, samples);

	glHint(GL_SAMPLES, samples);
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	_voroApprox = new VoroApprox;
	_render = new Render;

	init_widgets();

	_params.image = NULL;
	_params.approx = NULL;

	init_mvp();
	_render->set_window_size(mSize.x(), mSize.y());
	_render->set_point_size(_params.pointSize);
	_render->set_line_width(_params.lineWidth);

	setBackground(nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

MainWindow::~MainWindow()
{
	if (_voroApprox)
	{
		delete _voroApprox;
		_voroApprox = NULL;
	}

	if (_render)
	{
		delete _render;
		_render = NULL;
	}

	if (_params.image)
	{
		stbi_image_free(_params.image);
		_params.image = NULL;
	}

	if (_params.approx)
	{
		delete[] _params.approx;
		_params.approx = NULL;
	}
}

void MainWindow::init_widgets()
{
	_panel = new nanogui::Window(this, "");
	_panel->setPosition(Eigen::Vector2i(10, 10));
	_panel->setLayout(new nanogui::GroupLayout());

	init_file_widget();
	init_parameter_widget();
	init_algorithm_widget();
	init_render_widget();

	_frameCount = 0;
	_startTime = glfwGetTime();
	_fpsLabel = new nanogui::Label(_panel, "FPS: " + std::to_string(_frameCount));

	performLayout();
}

void MainWindow::init_file_widget()
{
	// file dialog
	new nanogui::Label(_panel, "Files", "sans-bold");

	// load image button
	nanogui::Button *loadImageBtn = new nanogui::Button(_panel, "load image");
	loadImageBtn->setCallback([&]
	{
		std::string fileName = nanogui::file_dialog(
		{ { "png", "Portable Network Graphics" }, 
		{ "jpeg", "Joint Photographic Experts Group" }, 
		{ "jpg", "Joint Photographic Experts Group" },
		{ "bmp", "Bitmap"},
		{ "pnm", "Portable Any Map"} }, false);
		if (!fileName.empty())
		{
			load_image(fileName.c_str());
		}
	});

	// load mesh button
	nanogui::Button *loadSitesBtn = new nanogui::Button(_panel, "load sites");
	loadSitesBtn->setCallback([&]
	{
		if (!_params.image)
		{
			new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning,
				"No Image", "please load image first");
			return;
		}

		std::string fileName = nanogui::file_dialog(
		{ { "txt", "sites coordinates" } }, false);
		if (!fileName.empty())
		{
			load_sites(fileName.c_str());
		}
	});

	// save mesh button
	nanogui::Button *saveSitesBtn = new nanogui::Button(_panel, "save sites");
	saveSitesBtn->setCallback([&]
	{
		if (!_voroApprox || _voroApprox->sites().empty())
		{
			new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning,
				"No Sites", "no sites");
			return;
		}

		std::string fileName = nanogui::file_dialog(
		{ { "txt", "sites coordinates" } }, true);
		if (!fileName.empty())
		{
			save_sites(fileName.c_str());
		}
	});

	// save image button
	nanogui::Button *saveApproxBtn = new nanogui::Button(_panel, "save approximation");
	saveApproxBtn->setCallback([&]
	{
		if (!_params.image)
		{
			new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning,
				"No Image", "please load image first");
			return;
		}

		if (!_voroApprox || _voroApprox->sites().empty())
		{
			new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning,
				"No Sites", "no sites");
			return;
		}

		std::string fileName = nanogui::file_dialog(
		{ { "png", "Portable Network Graphics" } }, true);
		if (!fileName.empty())
		{
			save_approximation(fileName.c_str());
		}
	});
}

void MainWindow::init_parameter_widget()
{
	// parameters
	new nanogui::Label(_panel, "Parameters", "sans-bold");
	nanogui::Widget *paraLayout = new nanogui::Widget(_panel);
	paraLayout->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Maximum, 10, 6));

	_params.degree = 1;
	new nanogui::Label(paraLayout, "degree:", "sans-bold");
	nanogui::IntBox<int> *degreeBox = new nanogui::IntBox<int>(paraLayout);
	degreeBox->setEditable(true);
	degreeBox->setFixedSize(Eigen::Vector2i(100, 20));
	degreeBox->setValue(_params.degree);
	degreeBox->setFontSize(16);
	degreeBox->setSpinnable(true);
	degreeBox->setMinValue(0);
	degreeBox->setMaxValue(2);
	degreeBox->setValueIncrement(1);
	degreeBox->setCallback([&](int a)
	{
		_params.degree = a;
		if (_voroApprox)
		{
			_voroApprox->set_degree(a);
		}
	});

	_params.sitesNumber = 1000;
	new nanogui::Label(paraLayout, "sites number:", "sans-bold");
	nanogui::IntBox<int> *sitesNumberBox = new nanogui::IntBox<int>(paraLayout);
	sitesNumberBox->setEditable(true);
	sitesNumberBox->setFixedSize(Eigen::Vector2i(100, 20));
	sitesNumberBox->setValue(_params.sitesNumber);
	sitesNumberBox->setFontSize(16);
	sitesNumberBox->setSpinnable(true);
	sitesNumberBox->setMinValue(3);
	sitesNumberBox->setValueIncrement(1);
	sitesNumberBox->setCallback([&](int a)
	{
		_params.sitesNumber = a;
	});

	_params.iteration = 200;
	new nanogui::Label(paraLayout, "iteration:", "sans-bold");
	nanogui::IntBox<int> *iterationBox = new nanogui::IntBox<int>(paraLayout);
	iterationBox->setEditable(true);
	iterationBox->setFixedSize(Eigen::Vector2i(100, 20));
	iterationBox->setValue(_params.iteration);
	iterationBox->setDefaultValue("1");
	iterationBox->setFontSize(16);
	iterationBox->setSpinnable(true);
	iterationBox->setMinValue(1);
	iterationBox->setValueIncrement(1);
	iterationBox->setCallback([&](int a)
	{
		_params.iteration = a;
	});

	_params.stepScale = 0.1;
	new nanogui::Label(paraLayout, "step scale:", "sans-bold");
	nanogui::FloatBox<double> *stepScaleBox = new nanogui::FloatBox<double>(paraLayout);
	stepScaleBox->setEditable(true);
	stepScaleBox->setFixedSize(Eigen::Vector2i(100, 20));
	stepScaleBox->setValue(_params.stepScale);
	stepScaleBox->setFontSize(16);
	stepScaleBox->setSpinnable(true);
	stepScaleBox->setMinValue(0.001);
	stepScaleBox->setMaxValue(5.0);
	stepScaleBox->setValueIncrement(0.05);
	stepScaleBox->setCallback([&](double a)
	{
		_params.stepScale = a;
	});

	_params.approxScale = 1.0f;
	new nanogui::Label(paraLayout, "approx scale:", "sans-bold");
	nanogui::FloatBox<float> *approxScaleBox = new nanogui::FloatBox<float>(paraLayout);
	approxScaleBox->setEditable(true);
	approxScaleBox->setFixedSize(Eigen::Vector2i(100, 20));
	approxScaleBox->setValue(_params.approxScale);
	approxScaleBox->setFontSize(16);
	approxScaleBox->setSpinnable(true);
	approxScaleBox->setMinValue(0.1f);
	approxScaleBox->setValueIncrement(0.5f);
	approxScaleBox->setCallback([&](float a)
	{
		_params.approxScale = a;
	});
}

void MainWindow::init_algorithm_widget()
{
	// algorithm
	new nanogui::Label(_panel, "Algorithm", "sans-bold");
	
	nanogui::PopupButton *initBtn = new nanogui::PopupButton(_panel, "init");
	nanogui::Popup *initPopup = initBtn->popup();
	initPopup->setLayout(new nanogui::GroupLayout());
	
	nanogui::Button *randomInitBtn = new nanogui::Button(initPopup, "random init");
	randomInitBtn->setCallback([&]
	{
		if (!_params.image || !_voroApprox)
			return;

		if (_params.approx)
		{
			delete[] _params.approx;
			_params.approx = NULL;
		}

		_voroApprox->random_init(_params.sitesNumber);

		if (_render)
		{
			std::vector<float> sites;
			_voroApprox->sites_data(sites);
			int n = (int)sites.size() / 2;
			float *ptr = n > 0 ? &sites[0] : NULL;
			_render->set_sites(ptr, n);

			std::vector<float> corners;
			std::vector<int> edges;
			_voroApprox->voronoi_data(corners, edges);
			n = (int)corners.size() / 2;
			ptr = n > 0 ? &corners[0] : NULL;
			int m = (int)edges.size() / 2;
			int *indices = m > 0 ? &edges[0] : NULL;
			_render->set_voronoi(ptr, n, indices, m);

			_render->set_approx(NULL, 0, 0, 0);
		}
	});

	nanogui::Button *greedyInitBtn = new nanogui::Button(initPopup, "greedy init");
	greedyInitBtn->setCallback([&]
	{
		if (!_params.image || !_voroApprox)
			return;

		if (_params.approx)
		{
			delete[] _params.approx;
			_params.approx = NULL;
		}

		_voroApprox->greedy_init(_params.sitesNumber);

		if (_render)
		{
			std::vector<float> sites;
			_voroApprox->sites_data(sites);
			int n = (int)sites.size() / 2;
			float *ptr = n > 0 ? &sites[0] : NULL;
			_render->set_sites(ptr, n);

			std::vector<float> corners;
			std::vector<int> edges;
			_voroApprox->voronoi_data(corners, edges);
			n = (int)corners.size() / 2;
			ptr = n > 0 ? &corners[0] : NULL;
			int m = (int)edges.size() / 2;
			int *indices = m > 0 ? &edges[0] : NULL;
			_render->set_voronoi(ptr, n, indices, m);

			_render->set_approx(NULL, 0, 0, 0);
		}
	});

	nanogui::Button *optBtn = new nanogui::Button(_panel, "optimize");
	optBtn->setCallback([&]()
	{
		if (!_params.image || !_voroApprox)
			return;

		if (_params.approx)
		{
			delete[] _params.approx;
			_params.approx = NULL;
		}

		_voroApprox->optimize(_params.degree, _params.iteration, _params.stepScale);

		if (_render)
		{
			std::vector<float> sites;
			_voroApprox->sites_data(sites);
			int n = (int)sites.size() / 2;
			float *ptr = n > 0 ? &sites[0] : NULL;
			_render->set_sites(ptr, n);

			std::vector<float> corners;
			std::vector<int> edges;
			_voroApprox->voronoi_data(corners, edges);
			n = (int)corners.size() / 2;
			ptr = n > 0 ? &corners[0] : NULL;
			int m = (int)edges.size() / 2;
			int *indices = m > 0 ? &edges[0] : NULL;
			_render->set_voronoi(ptr, n, indices, m);

			_render->set_approx(NULL, 0, 0, 0);
		}
	});

	nanogui::Button *approxBtn = new nanogui::Button(_panel, "approximate");
	approxBtn->setCallback([&]()
	{
		if (!_params.image || !_voroApprox)
			return;

		int width = int(_params.width * _params.approxScale);
		int height = int(_params.height * _params.approxScale);

		if (_params.approx)
		{
			delete[] _params.approx;
			_params.approx = NULL;
		}
		_params.approx = new unsigned char[width * height * _params.channel];

		_voroApprox->approximate(_params.degree, _params.approx, width, height, _params.channel);

		if (_render)
		{
			_render->set_approx(_params.approx, width, height, _params.channel);
		}
	});
}

void MainWindow::init_render_widget()
{
	// render switches
	new nanogui::Label(_panel, "Render", "sans-bold");

	nanogui::Button *resetBtn = new nanogui::Button(_panel, "reset view");
	resetBtn->setCallback([&]
	{
		init_mvp();
	});

	_params.showImage = true;
	nanogui::CheckBox *showImageCheckBox = new nanogui::CheckBox(_panel, "image", [&](bool state)
	{ 
		_params.showImage = state;
	});
	showImageCheckBox->setChecked(_params.showImage);

	_params.showSites = true;
	nanogui::CheckBox *showSitesCheckBox = new nanogui::CheckBox(_panel, "sites", [&](bool state)
	{
		_params.showSites = state;
	});
	showSitesCheckBox->setChecked(_params.showSites);

	_params.showVoronoi = true;
	nanogui::CheckBox *showVoronoiCheckBox = new nanogui::CheckBox(_panel, "voronoi", [&](bool state)
	{
		_params.showVoronoi = state;
	});
	showVoronoiCheckBox->setChecked(_params.showVoronoi);

	_params.showApprox = true;
	nanogui::CheckBox *showApproxImageCheckBox = new nanogui::CheckBox(_panel, "approximation", [&](bool state)
	{
		_params.showApprox = state;
	});
	showApproxImageCheckBox->setChecked(_params.showApprox);
}

void MainWindow::drawContents()
{
	if (_render)
	{
		if (_params.showImage)
		{
			_render->draw_image();
		}

		if (_params.showApprox)
		{
			_render->draw_approx();
		}

		if (_params.showVoronoi)
		{
			_render->draw_voronoi();
		}

		if (_params.showSites)
		{
			_render->draw_sites();
		}
	}

	++_frameCount;

	double currentTime = glfwGetTime();
	if (currentTime - _startTime > 1.0)
	{
		_fpsLabel->setCaption("FPS: " + std::to_string(_frameCount));
		_frameCount = 0;
		_startTime = currentTime;
	}
}


// event
bool MainWindow::resizeEvent(const Eigen::Vector2i& size)
{
	if (!nanogui::Screen::resizeEvent(size))
	{
		float ratio = float(size.y()) / float(size.x());
		_params.projectionMat = glm::ortho(-1.0f, 1.0f, -ratio, ratio);

		if (_render)
		{
			_render->set_projection_matrix(&_params.projectionMat[0][0]);
			_render->set_window_size(size.x(), size.y());
		}
	}

	return true;
}

bool MainWindow::scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel)
{
	if (!nanogui::Screen::scrollEvent(p, rel))
	{
		if (_params.resizingPoint)
		{
			_params.pointSize += int(rel.y()) * 0.5f;

			if (_params.pointSize < 1.0f)
				_params.pointSize = 1.0f;
			else if (_params.pointSize > 30.0f)
				_params.pointSize = 30.0f;

			xlog("point size = %f", _params.pointSize);

			_render->set_point_size(_params.pointSize);
		}
		else if (_params.resizingLine)
		{
			_params.lineWidth += int(rel.y()) * 0.5f;

			if (_params.lineWidth < 1.0f)
				_params.lineWidth = 1.0f;
			else if (_params.lineWidth > 10.0f)
				_params.lineWidth = 10.0f;

			xlog("line width = %f", _params.lineWidth);

			_render->set_line_width(_params.lineWidth);
		}
		else
		{
			float s = _params.modelMat[0][0];
			s += rel.y() / 30.0f;

			if (s < 0.01f)
				s = 0.01f;

			if (s > 100.0f)
				s = 100.0f;

			_params.modelMat[0][0] = s;
			_params.modelMat[1][1] = s;
			_params.modelMat[2][2] = s;

			if (_render)
			{
				_render->set_model_matrix(&_params.modelMat[0][0]);
			}
		}
	}

	glfwPostEmptyEvent();

	return true;
}

bool MainWindow::mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers)
{
	if (!nanogui::Screen::mouseMotionEvent(p, rel, button, modifiers))
	{
		if (_params.translating)
		{
			float dx = float(rel.x()) / float(mSize.x());
			float dy = float(rel.y()) / float(mSize.y());

			// translate
			//_params.modelMat = glm::translate(_params.modelMat, glm::vec3(dx, -dy, 0));
			_params.modelMat[3][0] += dx;
			_params.modelMat[3][1] -= dy;

			if (_render)
			{
				_render->set_model_matrix(&_params.modelMat[0][0]);
			}
		}
	}

	glfwPostEmptyEvent();

	return true;
}

bool MainWindow::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers)
{
	if (!nanogui::Screen::mouseButtonEvent(p, button, down, modifiers))
	{
		if (button == GLFW_MOUSE_BUTTON_2)
			_params.translating = down;
	}

	return true;
}

bool MainWindow::keyboardEvent(int key, int scancode, int action, int modifiers)
{
	if (!nanogui::Screen::keyboardEvent(key, scancode, action, modifiers))
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				_panel->setPosition(Eigen::Vector2i(10, 10));
				init_mvp();
				break;
			case GLFW_KEY_LEFT_ALT:
				_params.resizingPoint = true;
				break;
			case GLFW_KEY_RIGHT_ALT:
				_params.resizingLine = true;
				break;
			default:
				break;
			}
		}
		else if (action == GLFW_RELEASE)
		{
			switch (key)
			{
			case GLFW_KEY_LEFT_ALT:
				_params.resizingPoint = false;
				break;
			case GLFW_KEY_RIGHT_ALT:
				_params.resizingLine = false;
				break;
			default:
				break;
			}
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
void MainWindow::init_mvp()
{
	_params.modelMat = glm::mat4(1.0f);
	_params.modelMat = glm::scale(_params.modelMat, glm::vec3(0.5f, 0.5f, 0.5f));

	_params.viewMat = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	float ratio = float(mSize.y()) / float(mSize.x());
	_params.projectionMat = glm::ortho(-1.0f, 1.0f, -ratio, ratio);

	_params.translating = false;

	if (_render)
	{
		_render->set_mvp_matrix(&_params.modelMat[0][0], &_params.viewMat[0][0], &_params.projectionMat[0][0]);
	}
}

void MainWindow::load_image(const char* fileName)
{
	xlog("%s", fileName);

	if (_params.approx)
	{
		delete[] _params.approx;
		_params.approx = NULL;
	}

	if (_params.image)
	{
		stbi_image_free(_params.image);
		_params.image = NULL;
	}

	stbi_set_flip_vertically_on_load(true);
	_params.image = stbi_load(fileName, &_params.width, &_params.height, &_params.channel, 0);
	if (_params.image)
	{
		xlog("width = %d, height = %d, channel = %d", _params.width, _params.height, _params.channel);
		
		_params.modelMat = glm::mat4(1.0f);
		_params.modelMat = glm::scale(_params.modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
		
		if (_render)
		{
			_render->set_image(_params.image, _params.width, _params.height, _params.channel);
			_render->set_sites(NULL, 0);
			_render->set_voronoi(NULL, 0, NULL, 0);
			_render->set_approx(NULL, 0, 0, 0);
			_render->set_model_matrix(&_params.modelMat[0][0]);
		}

		if (_voroApprox)
		{
			_voroApprox->set_image(_params.image, _params.width, _params.height, _params.channel);
		}
	}
	else
	{
		xlog("failed");
	}
}

void MainWindow::load_sites(const char *fileName)
{
	if (!_params.image || !_voroApprox)
		return;

	if (_params.approx)
	{
		delete[] _params.approx;
		_params.approx = NULL;
	}

	std::vector<double> sites;

	std::ifstream file(fileName);
	while (!file.eof())
	{
		double x, y;
		file >> x >> y;
		sites.push_back(x);
		sites.push_back(y);
	}

	file.close();

	_voroApprox->set_sites(&sites[0], (int)sites.size() / 2);

	if (_render)
	{
		std::vector<float> sites;
		_voroApprox->sites_data(sites);
		int n = (int)sites.size() / 2;
		float *ptr = n > 0 ? &sites[0] : NULL;
		_render->set_sites(ptr, n);

		std::vector<float> corners;
		std::vector<int> edges;
		_voroApprox->voronoi_data(corners, edges);
		n = (int)corners.size() / 2;
		ptr = n > 0 ? &corners[0] : NULL;
		int m = (int)edges.size() / 2;
		int *indices = m > 0 ? &edges[0] : NULL;
		_render->set_voronoi(ptr, n, indices, m);

		_render->set_approx(NULL, 0, 0, 0);
	}
}

void MainWindow::save_sites(const char *fileName)
{
	if (!_voroApprox)
		return;

	std::vector<float> sites;
	_voroApprox->sites_data(sites);

	if (sites.empty())
		return;

	std::ofstream file(fileName);

	int vnb = (int)sites.size() / 2;
	for (int i = 0; i < vnb; ++i)
	{
		file << sites[2 * i] << " " << sites[2 * i + 1];
		if (i < vnb - 1)
			file << std::endl;
	}

	file.close();
}

void MainWindow::save_approximation(const char *fileName)
{
	if (!_params.image || !_voroApprox)
		return;

	int width = int(_params.width * _params.approxScale);
	int height = int(_params.height * _params.approxScale);

	xlog("save approximation with width = %d, height = %d, channel = %d", width, height, _params.channel);

	if (_params.approx)
	{
		delete[] _params.approx;
		_params.approx = NULL;
	}
	_params.approx = new unsigned char[width * height * _params.channel];

	_voroApprox->approximate(_params.degree, _params.approx, width, height, _params.channel);

	stbi_flip_vertically_on_write(true);
	stbi_write_png(fileName, width, height, _params.channel, _params.approx, 0);

	xlog("done");
}
