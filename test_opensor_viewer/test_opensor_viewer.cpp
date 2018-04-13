#include <opensor_viewer\Viewer.h>
#include <opensor_filereader\FileReader.h>
#include <thread>

int test_depthandvertexcapture_simple() {
	sor::Viewer *viewer = new sor::Viewer();
	viewer->createWindow(512, 512, "test");
	viewer->setCameraProjectionType(sor::Viewer::ProjectionType::ORTHOGRAPHIC);

	sor::FileReader *objFile = new sor::FileReader();
	//divide by 100 because unit of model is cm
	std::string objectName = "pillara";
	std::string objectFileName = "D:/dev/c-projects/OpenSor/test_opensor_viewer/data/models/" + objectName + ".obj";
	objFile->readObj(objectFileName, 0.01f);
	//std::cout << objFile->vertexArray.size() << std::endl;
	//std::cout << objFile->indexArray.size() << std::endl;

	//preview - get depth from here
	sor::CgObject *previewObject = new sor::CgObject();
	previewObject->setRenderType(sor::CgObject::RenderType::DEFAULT_RENDER);
	previewObject->loadShader("myshader2.vert", "myshader2.frag");
	previewObject->loadData(objFile->vertexArray, objFile->indexArray);
	previewObject->loadTexture("default_texture.jpg");
	previewObject->setDrawMode(sor::CgObject::Mode::TRIANGLES);
	viewer->cgObject->push_back(previewObject);

	//vertex object
	sor::CgObject *vertexObject = new sor::CgObject();
	vertexObject->setRenderType(sor::CgObject::RenderType::VERTEX);
	vertexObject->loadShader("pointshader.vert", "pointshader.frag");
	vertexObject->loadData(objFile->vertexArray, objFile->indexArray);
	vertexObject->loadTexture("default_texture.jpg");
	vertexObject->setDrawMode(sor::CgObject::Mode::POINTS);
	viewer->cgObject->push_back(vertexObject);

	//black object
	sor::CgObject *blackObject = new sor::CgObject();
	blackObject->setRenderType(sor::CgObject::RenderType::BLACK);
	blackObject->loadShader("myshader2.vert", "myshader2.frag");
	blackObject->loadData(objFile->vertexArray, objFile->indexArray);
	blackObject->loadTexture(cv::Mat::zeros(32, 32, CV_8UC3) + cv::Scalar(0, 0, 0));
	blackObject->setDrawMode(sor::CgObject::Mode::TRIANGLES);
	viewer->cgObject->push_back(blackObject);

	//black plane for ground
	sor::CgObject *plane = new sor::CgObject();
	plane->setRenderType(sor::CgObject::RenderType::DEFAULT_RENDER);
	plane->loadShader("myshader2.vert", "myshader2.frag");
	GLfloat planeVertices[] = { -100.0f, 0.0f, -100.0f, 0.0f, 0.0f,
		-100.0f, 0.0f, 100.0f, 0.0f, 1.0f,
		100.0f, 0.0f, 100.0f, 1.0f, 1.0f,
		100.0f, 0.0f, -100.0f, 1.0f, 0.0f };
	GLuint planeIndices[] = { 0,1,2,0,2,3 };
	std::vector<float> planeVertexArray(std::begin(planeVertices), std::end(planeVertices));
	std::vector<unsigned int> planeIndexArray(std::begin(planeIndices), std::end(planeIndices));
	plane->loadData(planeVertexArray, planeIndexArray);
	plane->loadTexture("default_texture.jpg");
	//plane->loadTexture(cv::Mat::zeros(32, 32, CV_8UC3) + cv::Scalar(125, 125, 125));
	plane->setDrawMode(sor::CgObject::Mode::TRIANGLES);
	viewer->cgObject->push_back(plane);

	//viewer->setFramebuffer();
	//viewer->camera->poseCamera(0, 0, 3, 0, 1, 0, -90.0f, -45.0f);
	viewer->depthFileNameCounter = 0;
	viewer->vertexFileNameCounter = 0;
	viewer->depthFileNameHeader = "data/depth/d" + objectName;
	viewer->previewFileNameHeader = "data/depthpreview/" + objectName;
	viewer->vertexFileNameHeader = "data/vertex/v" + objectName;
	viewer->depthAndVertexCapture();
	viewer->close();
	return 0;
}

int test_depthmap_simple() {
	sor::Viewer *viewer = new sor::Viewer();
	viewer->createWindow(1024, 1024, "test");

	sor::FileReader *objFile = new sor::FileReader();
	//divide by 100 because unit of model is cm
	objFile->readObj("H:/deeplearning/faceretopology/asukakyo.obj", 0.01f);
	//std::cout << objFile->vertexArray.size() << std::endl;
	//std::cout << objFile->indexArray.size() << std::endl;

	//box solid
	sor::CgObject *cgobject = new sor::CgObject();
	//cgobject->loadShader("depthshader.vert", "depthshader.frag");
	cgobject->loadShader("myshader2.vert", "myshader2.frag");
	//cgobject->loadShader("pointshader.vert", "pointshader.frag");
	cgobject->loadData(objFile->vertexArray, objFile->indexArray);
	cgobject->loadTexture("default_texture.jpg");
	cgobject->setDrawMode(sor::CgObject::Mode::TRIANGLES);
	viewer->cgObject->push_back(cgobject);

	viewer->setFramebuffer();
	//viewer->camera->poseCamera(0, 0, 3, 0, 1, 0, -90.0f, -45.0f);
	//viewer->captureDepth("test.png");
	viewer->run();
	viewer->close();
	return 0;
}

int test_filereader_simple() {
	sor::Viewer *viewer = new sor::Viewer();
	viewer->createWindow(800, 600, "test");

	sor::FileReader *objFile = new sor::FileReader();
	objFile->readObj("H:/deeplearning/faceretopology/monkey2.obj");
	//std::cout << objFile->vertexArray.size() << std::endl;
	//std::cout << objFile->indexArray.size() << std::endl;

	//box solid
	//cv::Mat texture = cv::imread("texture.png");
	sor::CgObject *cgobject = new sor::CgObject();
	//cgobject->loadShader("depthshader.vert", "depthshader.frag");
	cgobject->loadShader("myshader2.vert", "myshader2.frag");
	cgobject->loadData(objFile->vertexArray, objFile->indexArray);
	cgobject->loadTexture("default_texture.jpg");
	cgobject->setDrawMode(sor::CgObject::Mode::TRIANGLES);
	viewer->cgObject->push_back(cgobject);

	viewer->run();
	viewer->close();

	return 0;
}

int test_viewer_simple() {
	sor::Viewer *viewer = new sor::Viewer();
	viewer->createWindow(800, 600, "test");

	GLfloat vertices[] = {
		// front
		1.0, 1.0, 1.0, 1.0f, 1.0f,
		-1.0, 1.0, 1.0, 1.0f, 0.0f,
		-1.0,  -1.0, 1.0, 0.0f, 0.0f,
		1.0,  -1.0, 1.0, 0.0f, 1.0f,
		// back
		1.0, 1.0, -1.0, 1.0f, 1.0f,
		-1.0, 1.0, -1.0, 1.0f, 0.0f,
		-1.0, -1.0, -1.0, 0.0f, 0.0f,
		1.0,  -1.0,  -1.0, 0.0f, 1.0f
	};

	GLushort indices[] = {
		0, 1, 2, 2, 3, 0, // front
		1, 5, 6, 6, 2, 1, // right
		7, 6, 5, 5, 4, 7, // back
		4, 0, 3, 3, 7, 4, // left
		4, 5, 1, 1, 0, 4, // bottom
		3, 2, 6, 6, 7, 3, // top
	};

	std::vector<float> vertexArray(std::begin(vertices), std::end(vertices));
	std::vector<unsigned int> indexArray(std::begin(indices), std::end(indices));

	//box solid
	cv::Mat texture = cv::imread("texture.jpg");
	sor::CgObject *cgobject = new sor::CgObject();
	cgobject->loadShader("depthshader.vert", "depthshader.frag");
	cgobject->loadData(vertexArray, indexArray);
	cgobject->loadTexture(texture);
	cgobject->setDrawMode(sor::CgObject::Mode::TRIANGLES);
	viewer->cgObject->push_back(cgobject);

	//box vertex only
	sor::CgObject *point = new sor::CgObject();
	point->loadShader("pointshader.vert", "pointshader.frag");
	point->loadData(vertexArray, indexArray);
	point->setDrawMode(sor::CgObject::Mode::POINTS);
	viewer->cgObject->push_back(point);

	viewer->run();
	viewer->close();

	return 0;
}

int main() {
	//test_viewer_simple();
	//test_filereader_simple();
	//test_depthmap_simple();
	test_depthandvertexcapture_simple();
	return 0;
}