#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef _DEBUG
#define LIB_EXT "d.lib"
#else
#define LIB_EXT ".lib"
#endif
#define CV_LIB_PATH "D:/dev/staticlib64/"
#define CV_VER_NUM  CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#pragma comment(lib, CV_LIB_PATH "opencv_core" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_highgui" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_imgcodecs" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_imgproc" CV_VER_NUM LIB_EXT)

#pragma comment(lib, CV_LIB_PATH "IlmImf.lib")
#pragma comment(lib, CV_LIB_PATH "ippicvmt.lib")
#pragma comment(lib, CV_LIB_PATH "ippiw.lib")
#pragma comment(lib, CV_LIB_PATH "ittnotify.lib")
#pragma comment(lib, CV_LIB_PATH "libjasper.lib")
#pragma comment(lib, CV_LIB_PATH "libjpeg-turbo.lib")
#pragma comment(lib, CV_LIB_PATH "libpng.lib")
#pragma comment(lib, CV_LIB_PATH "libprotobuf.lib")
#pragma comment(lib, CV_LIB_PATH "libtiff.lib")
#pragma comment(lib, CV_LIB_PATH "libwebp.lib")
#pragma comment(lib, CV_LIB_PATH "zlib.lib")

int main() {
	cv::Mat im = cv::imread("C:/Users/menandro/Pictures/hand.png");
	std::cout << im.size() << " " << im.channels() << std::endl;
	cv::Mat imout;
	cv::resize(im, imout, cv::Size(448, 450), cv::INTER_NEAREST);
	std::cout << imout.size() << " " << imout.channels() << std::endl;

	cv::Mat imdepth = cv::Mat::zeros(cv::Size(448, 450), CV_16UC1);
	cv::imshow("here", imdepth);
	cv::waitKey();

	for (int j = 0; j < 450; j++) {
		std::cout << j << " " << (int)imout.at<cv::Vec3b>(j, 200)[0] << std::endl;
		for (int i = 0; i < 448; i++) {
			cv::Vec3b pix = imout.at<cv::Vec3b>(j, i);
			//std::cout << j << ":" << i << " " << pix[0] << std::endl;
			if ((int)pix[0] == 0) {
				imdepth.at<unsigned short>(j, i) = 300;
			}
			else {
				imdepth.at<unsigned short>(j, i) = 1000;
			}
		}
	}

	//std::cout << imdepth.size << " " << imdepth.type() << " " << imdepth.at<unsigned short>(200,200) << std::endl;
	//im.convertTo(imout, CV_16C1
	cv::imshow("hand", imdepth);
	cv::imwrite("C:/Users/menandro/Pictures/handdepth.png", imdepth);
	cv::waitKey();
	return 0;
}
