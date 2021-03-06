// To work, the project needs:
// Build Customizations  -> Cuda9.1 (so that cudart is found)
// Link to cudart.lib (Linker -> Input -> Additional Dependencies -> cudart.lib

#include <opensor_imagefilter\ImageFilter.h>

#define BLOCKWIDTH 32
#define BLOCKHEIGHT 12
#define STRIDEALIGNMENT 32

int main() {
	ImageFilter *smooth = new ImageFilter(BLOCKWIDTH, BLOCKHEIGHT, STRIDEALIGNMENT);
	ImageFilter *edgedetect = new ImageFilter(BLOCKWIDTH, BLOCKHEIGHT, STRIDEALIGNMENT);
	ImageFilter *imconvert = new ImageFilter(BLOCKWIDTH, BLOCKHEIGHT, STRIDEALIGNMENT);
	cv::namedWindow("output", cv::WINDOW_NORMAL);
	cv::resizeWindow("output", 640, 480);
	cv::namedWindow("input", cv::WINDOW_NORMAL);
	cv::resizeWindow("input", 640, 480);
	cv::VideoCapture stream;
	cv::Mat im;
	if (!stream.open(0)) {
		return 0;
	}
	int width = 640;
	int height = 480;

	stream.set(CV_CAP_PROP_FRAME_WIDTH, width);
	stream.set(CV_CAP_PROP_FRAME_HEIGHT, height);

	smooth->initialize(width, height, CV_8UC3, CV_8UC3);
	edgedetect->initialize(width, height, CV_8UC3, CV_32F);
	imconvert->initialize(width, height, CV_8UC3, CV_32FC3);

	cv::Mat smoothout = cv::Mat(cv::Size(width, height), CV_8UC3);
	cv::Mat edgedetectout = cv::Mat(cv::Size(width, height), CV_32F);
	cv::Mat imconvertout = cv::Mat(cv::Size(width, height), CV_32FC3);

	clock_t timer;
	float currentFPS;
	int kernelsize = 5;
	float threshold = 0.1f;
	float thresholdh = 0.1f;
	float thresholds = 0.005f;
	float thresholdv = 0.01f;
	int bwb = 2500;

	for (;;) {
		stream >> im;
		if (im.empty()) break; // end of video stream
		char pressed = cv::waitKey(10);
		if (pressed == 27) break; //press escape
		if ((char)pressed == 'q') {
			kernelsize += 2;
			if (kernelsize > 97) kernelsize = 97;
		}
		if ((char)pressed == 'w') {
			kernelsize -= 2;
			if (kernelsize < 3) kernelsize = 3;
		}
		if ((char)pressed == 'e') {
			threshold += 0.01f;
		}
		if ((char)pressed == 'r') {
			threshold -= 0.01f;
		}
		if ((char)pressed == '4') thresholdh += 0.05f;
		if ((char)pressed == '1') thresholdh -= 0.05f;
		if ((char)pressed == '5') thresholds += 0.0001f;
		if ((char)pressed == '2') thresholds -= 0.0001f;
		if ((char)pressed == '6') thresholdv += 0.001f;
		if ((char)pressed == '3') thresholdv -= 0.001f;
		//if ((char)pressed == '9') bwb += 50;
		//if ((char)pressed == '7') bwb -= 50;
		//stream.set(CV_CAP_PROP_WHITE_BALANCE_BLUE_U, bwb);

		timer = clock();

		/*for (int k = 1; k <= 100; k++) {
			if (smooth->imFilter(im, smoothout, "median", kernelsize)) return 0;
		}*/
		if (smooth->imFilter(im, smoothout, "median", kernelsize)) return 0;
		//if (smooth->imFilter(smoothout, smoothout, "median", kernelsize)) return 0;

		//if (smooth->imFilter(im, smoothout, "box", kernelsize)) return 0;

		//if (smooth->imFilter(im, smoothout, "gaussianblur", kernelsize)) return 0;
		//if (imconvert->imFilter(smoothout, imconvertout, "rgbtohsv", kernelsize)) return 0;
		//if (edgedetect->imFilter(imconvertout, edgedetectout, "edgedetect_hsv", kernelsize, make_float3(thresholdh, thresholds, thresholdv))) return 0;
		//if (edgedetect->imFilter(smoothout, edgedetectout, "edgedetect", kernelsize, threshold)) return 0;
		timer = clock() - timer;
		currentFPS = 1000.0f / timer;

		cv::Mat hsv[3];
		cv::split(imconvertout, hsv);
		cv::Mat hue;
		hsv[0].convertTo(hue, CV_8UC1);
		double minval, maxval;
		cv::minMaxLoc(hsv[2], &minval, &maxval);
		//std::cout << minval << " " << maxval << std::endl;
		std::ostringstream message;
		//message << "FPS: " << currentFPS << " ||  PT: " << timer << "ms || KS: " << kernelsize << " || TH: " << stream.get(CV_CAP_PROP_HUE) << " |" << stream.get(CV_CAP_PROP_SATURATION) << " |" << thresholdv;
		//message << "FPS: " << currentFPS << " ||  PT: " << timer << "ms || KS: " << kernelsize << " || TH: " << thresholdh << " | " << thresholds << " | " << thresholdv;
		message << "FPS: " << currentFPS << " ||  PT: " << timer << "ms || KS: " << kernelsize << " || TH: " << threshold;
		//message << "TH: " << thresholdh << " | " << thresholds << " | " << thresholdv;
		cv::putText(smoothout, message.str(), cv::Point(0, 10), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 255, 255));
		cv::imshow("output", im);
		cv::imshow("input", smoothout);
	}
	smooth->close();
	edgedetect->close();
	return 0;
}