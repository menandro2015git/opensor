// To work, the project needs:
// Right click Project -> Build Dependencies -> Build Customizations  -> Cuda8.0 (or higher versions) (so that cudart is found)
// Link to cudart.lib (Linker -> Input -> Additional Dependencies -> cudart.lib

#pragma once

#include "lib_link.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <opensor_reconflow/Reconflow.h>
#include <opencv2/optflow.hpp>
#include <opensor_camerapose/CameraPose.h>

// Calibration file for Raw Kitti Dataset
class CalibData {
public:
	CalibData() {
		s = std::vector<cv::Mat>(4);
		k = std::vector<cv::Mat>(4);
		d = std::vector<cv::Mat>(4);
		r = std::vector<cv::Mat>(4);
		t = std::vector<cv::Mat>(4);
		sRect = std::vector<cv::Mat>(4);
		rRect = std::vector<cv::Mat>(4);
		pRect = std::vector<cv::Mat>(4);
	};
	std::string date, time;
	float cornerDist;
	std::vector<cv::Mat> s, k, d, r, t, sRect, rRect, pRect;
	cv::Mat k02; //rectified camera matrix of image_02
};

void readCalibKitti(std::string filename, CalibData *calib) {
	//read from file
	std::fstream calibFile(filename, std::ios_base::in);
	// Read calibTime;

	std::string h;
	calibFile >> h >> calib->date >> calib->time;
	calibFile >> h >> calib->cornerDist;
	for (int k = 0; k < 4; k++) {
		std::string header;
		double a, b, c, d, e, f, g, h, i;

		calibFile >> header >> a >> b; //s
		double smat[2] = { a, b };
		calib->s[k] = cv::Mat(1, 2, CV_64F, smat).clone();

		calibFile >> header >> a >> b >> c >> d >> e >> f >> g >> h >> i; //k
		double kmat[9] = { a, b, c, d, e, f, g, h, i };
		calib->k[k] = cv::Mat(3, 3, CV_64F, kmat).clone();

		calibFile >> header >> a >> b >> c >> d >> e; //d
		double dmat[5] = { a, b, c, d, e };
		calib->d[k] = cv::Mat(1, 5, CV_64F, dmat).clone();

		calibFile >> header >> a >> b >> c >> d >> e >> f >> g >> h >> i; //r
		double rmat[9] = { a, b, c, d, e, f, g, h, i };
		calib->r[k] = cv::Mat(3, 3, CV_64F, rmat).clone();

		calibFile >> header >> a >> b >> c; //t
		double tmat[5] = { a, b, c, d, e };
		calib->t[k] = cv::Mat(3, 1, CV_64F, tmat).clone();

		calibFile >> header >> a >> b; //srect
		double srmat[2] = { a, b };
		calib->sRect[k] = cv::Mat(1, 2, CV_64F, srmat).clone();

		calibFile >> header >> a >> b >> c >> d >> e >> f >> g >> h >> i; //rrect
		double rrmat[9] = { a, b, c, d, e, f, g, h, i };
		calib->rRect[k] = cv::Mat(3, 3, CV_64F, rrmat).clone();

		double m, n, o;
		calibFile >> header >> a >> b >> c >> d >> e >> f >> g >> h >> i >> m >> n >> o; //prect
		double prmat[12] = { a, b, c, d, e, f, g, h, i, m, n, o };
		calib->pRect[k] = cv::Mat(3, 4, CV_64F, prmat).clone();

		double k02mat[9] = { a, b, c, e, f, g, i, m, n };
		calib->k02 = cv::Mat(3, 3, CV_64F, k02mat).clone();
	}
	std::cout << "K: " << calib->k02 << std::endl;
	//std::cout << calib->k02.at<double>(0, 0) << " " << calib->k02.at<double>(0, 1) << " " << calib->k02.at<double>(0, 2) << std::endl;
	//std::cout << calib->k02.at<double>(1, 0) << " " << calib->k02.at<double>(1, 1) << " " << calib->k02.at<double>(1, 2) << std::endl;
	//std::cout << calib->k02.at<double>(2, 0) << " " << calib->k02.at<double>(2, 1) << " " << calib->k02.at<double>(2, 2) << std::endl;
}

void readDepthKitti(std::string filename, cv::Mat &depth, cv::Mat &mask) {
	cv::Mat depthRaw = cv::imread(filename, cv::IMREAD_UNCHANGED);
	depthRaw.convertTo(depth, CV_32F, 1/256.0);
	//std::cout << depthRaw.at<unsigned short>(0, 0) << std::endl;
	//std::cout << depth.at<float>(0, 0) << std::endl;

	// Extract the mask
	mask = cv::Mat::zeros(depthRaw.size(), CV_32F);
	for (int j = 0; j < mask.rows; j++) {
		for (int i = 0; i < mask.cols; i++) {
			if (depth.at<float>(j, i) > 0.0f) {
				mask.at<float>(j, i) = 1.0f;
			}
		}
	}
	//cv::imshow("mask", mask);
	//cv::waitKey();
}

void depthTo3d(cv::Mat depth, cv::Mat mask, cv::Mat &Xin, cv::Mat &Yin, cv::Mat &Zin, cv::Mat K) {
	// Convert depth to 3D points
	Xin = cv::Mat::zeros(mask.size(), CV_32F);
	Yin = cv::Mat::zeros(mask.size(), CV_32F);
	Zin = cv::Mat::zeros(mask.size(), CV_32F);
	double centerX = K.at<double>(0, 2);
	double centerY = K.at<double>(1, 2);
	double focalX = K.at<double>(0, 0);
	double focalY = K.at<double>(1, 1);

	for (int j = 0; j < mask.rows; j++) {
		for (int i = 0; i < mask.cols; i++) {
			if (mask.at<float>(j, i) == 1.0f) {
				Zin.at<float>(j, i) = depth.at<float>(j, i);
				Xin.at<float>(j, i) = (float)(((double)i - centerX) * depth.at<float>(j, i) / focalX);
				Yin.at<float>(j, i) = (float)(((double)j - centerY) * depth.at<float>(j, i) / focalY);
			}
		}
	}
	//cv::imshow("tst", Yin);
	//cv::waitKey();
}

int getRtStereo(std::string filename, cv::Mat &R, cv::Mat &t, cv::Mat &K) {
	double Re[9] = { 1,0,0,0,1,0,0,0,1 };
	double te[3] = { -1, 0.00001, 0.00001 };
	R = cv::Mat(3, 3, CV_64F, Re).clone();
	t = cv::Mat(3, 1, CV_64F, te).clone();
	//read from file
	std::fstream myfile(filename, std::ios_base::in);
	float a, b, c, d, e, f, g, h, i, j, k, l;
	char ch[3];
	myfile >> ch >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >> k >> l;
	double cammat[9] = { a, b, c, e, f, g, i, j, k };
	K = cv::Mat(3, 3, CV_64F, cammat).clone();
	std::cout << "Camera pose found for: " << filename << std::endl;
	std::cout << "K0: " << a << " " << b << " " << c << " " << e << " " << f << " " << g << " " << i << " " << j << " " << k << std::endl;
	return 0;
}

int getRtStereo(std::string filename, cv::Mat &R, cv::Mat &t) {
	double Re[9] = { 1,0,0,0,1,0,0,0,1 };
	double te[3] = { -1, 0.00001, 0.00001 };
	R = cv::Mat(3, 3, CV_64F, Re).clone();
	t = cv::Mat(3, 1, CV_64F, te).clone();
	return 0;
}

int solve2d3dPose(cv::Mat im, cv::Mat Xin, cv::Mat Yin, cv::Mat Zin, cv::Mat depthMask, cv::Mat flownet, 
	cv::Mat K, cv::Mat &R, cv::Mat &t) {
	/// 1. Extract 2d points from im1 using flownet and depth mask
	cv::Mat nonZeroCoord;
	cv::Mat mask;
	std::vector<cv::Point2f> points2d;
	std::vector<cv::Point3f> points3d;

	depthMask.convertTo(mask, CV_8UC1);
	//std::cout << depthMask.at<float>(188, 648) << std::endl;
	cv::findNonZero(mask, nonZeroCoord);
	for (int i = 0; i < nonZeroCoord.total(); i++) {
		//std::cout << "Zero#" << i << ": " << nonZeroCoord.at<cv::Point>(i).x << ", " << nonZeroCoord.at<cv::Point>(i).y << std::endl;
		float x2d, y2d;
		float x3d, y3d, z3d;
		x2d = nonZeroCoord.at<cv::Point>(i).x + flownet.at<cv::Vec2f>(nonZeroCoord.at<cv::Point>(i))[0]; //x + u
		y2d = nonZeroCoord.at<cv::Point>(i).y + flownet.at<cv::Vec2f>(nonZeroCoord.at<cv::Point>(i))[1]; //y + v
		points2d.push_back(cv::Point2f(x2d, y2d));

		x3d = Xin.at<float>(nonZeroCoord.at<cv::Point>(i));
		y3d = Yin.at<float>(nonZeroCoord.at<cv::Point>(i));
		z3d = Zin.at<float>(nonZeroCoord.at<cv::Point>(i));
		points3d.push_back(cv::Point3f(x3d, y3d, z3d));
	}

	cv::Mat distCoeffs(4, 1, cv::DataType<double>::type);
	distCoeffs.at<double>(0) = 0;
	distCoeffs.at<double>(1) = 0;
	distCoeffs.at<double>(2) = 0;
	distCoeffs.at<double>(3) = 0;

	cv::Mat rvec, tvec;
	cv::solvePnPRansac(points3d, points2d, K, distCoeffs, rvec, t);
	cv::Rodrigues(rvec, R);
	std::cout << "rvec: " << R << std::endl;
	std::cout << "tvec: " << t << std::endl;
	
	/*cv::solvePnP(points3d, points2d, K, distCoeffs, rvec, t);
	cv::Rodrigues(rvec, R);
	std::cout << "rvec: " << R << std::endl;
	std::cout << "tvec: " << t << std::endl;*/
	/// 2. Create array of these 2d points and Xin, Yin, Zin
	/// 3. SolvePnP
	return 0;
}

int test_sparseLidar() {
	std::string mainfolder = "h:/data_kitti_raw/2011_09_26/2011_09_26_drive_0093_sync/";
	std::string im0filename = mainfolder + "image_02/data/0000000005.png";
	std::string im1filename = mainfolder + "image_03/data/0000000005.png";
	std::string flownetfilename = mainfolder + "flownet_stereo/0000000005.flo";
	std::string depthfilename = mainfolder + "proj_depth/velodyne_raw/image_02/0000000005.png";
	std::string cameramatrix = "h:/data_kitti_raw/2011_09_26/2011_09_26_calib/2011_09_26/calib_cam_to_cam.txt";
	std::string outputfilename = mainfolder + "output/output3d";
	sor::ReconFlow *flow = new sor::ReconFlow(32, 12, 32);

	// Load Params
	float lambda, tau, alphaTv, alphaFn, alphaProj, lambdaf, lambdams, lambdasp, scale;
	int nWarpIters, iters, minWidth;
	std::string suffixFor3D = "oursfn";
	lambda = 50.0f;//50
	tau = 0.125f;
	alphaTv = 33.3f;
	alphaFn = 1.0f; //100

	alphaProj = 60.0f;//fix to 60 (slight effect)

	lambdaf = 0.0001f;//0.1 (Fdata)
	lambdams = 0.0f;//100 (Fms)
	lambdasp = 100.0f;

	nWarpIters = 1;
	iters = 1000;
	scale = 2.0f;
	minWidth = 200;

	// Set camera matrices
	cv::Mat R, t, K;
	CalibData *calibData = new CalibData();
	readCalibKitti(cameramatrix, calibData);
	K = calibData->k02;
	//int isSetPose = getRtStereo(cameramatrix, R, t);

	// Check image size and compute pyramid nlevels
	//std::string initialImage = im1filename;
	cv::Mat iset = cv::imread(im1filename);
	int width = iset.cols;
	int height = iset.rows;
	int nLevels = 1;
	int pHeight = (int)((float)height / scale);
	while (pHeight > minWidth) {
		nLevels++;
		pHeight = (int)((float)pHeight / scale);
	}
	std::cout << "Pyramid Levels: " << nLevels << std::endl;
	int stride = flow->iAlignUp(width);
	std::cout << "Stride: " << stride << std::endl;
	std::cout << "Width: " << stride << std::endl;
	cv::Mat isetpad;
	cv::copyMakeBorder(iset, isetpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);

	// Initialize handler matrices for display and output
	cv::Mat uvrgb = cv::Mat(isetpad.size(), CV_32FC3);
	cv::Mat u = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat v = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat X = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat Y = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat Z = cv::Mat(isetpad.size(), CV_32F);

	// Open input images
	cv::Mat i0rgb, i1rgb, flownet, i1rgbpad, i0rgbpad, flownetpad, RR, tt;
	cv::Mat Xin, Yin, Zin, Xinpad, Yinpad, Zinpad, depth, depthMask, depthMaskpad;
	/*std::string flownet2filename = mainfolder + flownetfilename;
	std::string image1filename = mainfolder + im1filename;
	std::string image2filename = mainfolder + im2filename;*/
	i0rgb = cv::imread(im0filename);
	i1rgb = cv::imread(im1filename);

	// Open initial matching (flownet)
	flownet = cv::optflow::readOpticalFlow(flownetfilename);
	if (flownet.empty()) {
		std::cerr << "Flownet file not found." << std::endl;
		return 0;
	}
	else std::cout << "Flownet found." << std::endl;

	// Open initial 3D
	readDepthKitti(depthfilename, depth, depthMask);
	depthTo3d(depth, depthMask, Xin, Yin, Zin, K);

	// Solve pose from 3d-2d matches
	solve2d3dPose(i1rgb, Xin, Yin, Zin, depthMask, flownet, K, R, t);
	
	// Solve relative camera pose
	cv::Mat Rrel, trel;
	int minHessian = 5000;
	sor::CameraPose *camerapose = new sor::CameraPose();
	camerapose->initialize(K, minHessian);
	if (!camerapose->solvePose_8uc3(i0rgb, i1rgb, Rrel, trel)) {
		if (!camerapose->filtered_keypoints_im1.empty()) {
			//drawKeypoints(i0rgb, camerapose->filtered_keypoints_im1, i0rgb, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DEFAULT);
		}
	}
	else std::cerr << "Failed to solve for relative pose. " << std::endl;
	std::cout << "Rrel: " << Rrel << std::endl;
	std::cout << "trel: " << trel << std::endl;

	// Resize images by padding
	cv::copyMakeBorder(i0rgb, i0rgbpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(i1rgb, i1rgbpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(flownet, flownetpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(Xin, Xinpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(Yin, Yinpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(Zin, Zinpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(depthMask, depthMaskpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::Mat flownet2[2];   //split flownet channels
	cv::split(flownetpad, flownet2);

	// Initialize ReconFlow
	flow->initializeR(width, height, 3, nLevels, scale, sor::ReconFlow::METHODR_TVL1_MS_FNSPARSE_LIDAR,
		lambda, 0.0f, lambdaf, lambdams, lambdasp,
		alphaTv, alphaProj, alphaFn,
		tau, nWarpIters, iters);
	flow->setCameraMatrices(K, K);

	// Copy data to GPU
	flow->copyImagesToDevice(i0rgbpad, i1rgbpad);
	flow->copySparseOpticalFlowToDevice(flownet2[0], flownet2[1]); //can set a mask as third argument
	flow->copySparse3dToDevice(Xinpad, Yinpad, Zinpad, depthMaskpad);
	
	// Calculate ReconFlow
	flow->solveR(R, t, 50.0f); //main computation iteration

	// Copy GPU results to CPU
	flow->copyOpticalFlowToHost(u, v, uvrgb); //uvrgb is an optical flow image
	flow->copy3dToHost(X, Y, Z); //3D points

	// Save output 3D as ply file
	std::vector<cv::Vec3b> colorbuffer(stride*height);
	cv::Mat colorMat = cv::Mat(static_cast<int>(colorbuffer.size()), 1, CV_8UC3, &colorbuffer[0]);
	std::vector<cv::Vec3f> buffer(stride*height);
	cv::Mat cloudMat = cv::Mat(static_cast<int>(buffer.size()), 1, CV_32FC3, &buffer[0]);
	colorbuffer.clear();
	buffer.clear();

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < stride; i++) {
			cv::Vec3b rgb = i0rgbpad.at<cv::Vec3b>(j, i);
			colorbuffer.push_back(rgb);

			float x = X.at<float>(j, i);
			float y = Y.at<float>(j, i);
			float z = Z.at<float>(j, i);
			if (((z <= 100) && (z > 10)) && ((x <= 100) && (y > -100)) && ((y <= 100) && (y > -100))) {
				buffer.push_back(cv::Vec3f(x, -y, -z));
			}
			else {
				x = std::numeric_limits<float>::quiet_NaN();
				y = std::numeric_limits<float>::quiet_NaN();
				z = std::numeric_limits<float>::quiet_NaN();
				buffer.push_back(cv::Vec3f(x, y, z));
			}
		}
	}
	cv::viz::WCloud cloud(cloudMat, colorMat);

	std::ostringstream output3d;
	std::cout << outputfilename << suffixFor3D << ".ply";
	output3d << outputfilename << suffixFor3D << ".ply";
	cv::viz::writeCloud(output3d.str(), cloudMat, colorMat);

	cv::imshow("flow", uvrgb);
	cv::waitKey();
	return 0;
	return 0;
}

int test_withoutFlownet() {

	std::string mainfolder = "h:/data_kitti_raw/2011_09_26/2011_09_26_drive_0093_sync/";
	std::string im0filename = mainfolder + "image_02/data/0000000000.png";
	std::string im1filename = mainfolder + "image_03/data/0000000000.png";
	std::string flownetfilename = mainfolder + "flownet_stereo/0000000000.flo";
	std::string cameramatrix = "h:/data_kitti_raw/2011_09_26/2011_09_26_calib/2011_09_26/calib_cam_to_cam.txt";
	std::string outputfilename = mainfolder + "output/output3d";
	sor::ReconFlow *flow = new sor::ReconFlow(32, 12, 32);
	flow->allocTest();

	// Load Params
	float lambda, tau, alphaTv, alphaFn, alphaProj, lambdaf, lambdams, scale;
	int nWarpIters, iters, minWidth;
	std::string suffixFor3D = "oursfn";
	lambda = 50.0f;//50
	tau = 0.125f;
	alphaTv = 33.3f;
	alphaFn = 100.0f; //100

	alphaProj = 0.0f;//fix to 60 (slight effect)

	lambdaf = 0.0f;//0.1 (Fdata)
	lambdams = 0.0f;//100 (Fms)

	nWarpIters = 1;
	iters = 300;
	scale = 2.0f;
	minWidth = 200;

	// Check image size and compute pyramid nlevels
	//std::string initialImage = mainfolder + im1filename;
	cv::Mat iset = cv::imread(im0filename);
	int width = iset.cols;
	int height = iset.rows;
	int nLevels = 1;
	int pHeight = (int)((float)height / scale);
	while (pHeight > minWidth) {
		nLevels++;
		pHeight = (int)((float)pHeight / scale);
	}
	std::cout << "Pyramid Levels: " << nLevels << std::endl;
	int stride = flow->iAlignUp(width);
	std::cout << "Height: " << height << std::endl;
	std::cout << "Width: " << width << std::endl;
	cv::Mat isetpad;
	cv::copyMakeBorder(iset, isetpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);

	// Initialize handler matrices for display and output
	cv::Mat uvrgb = cv::Mat(isetpad.size(), CV_32FC3);
	cv::Mat u = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat v = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat X = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat Y = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat Z = cv::Mat(isetpad.size(), CV_32F);

	// Open input images
	cv::Mat i0rgb, i1rgb, flownet, i1rgbpad, i0rgbpad, flownetpad;
	i0rgb = cv::imread(im0filename);
	i1rgb = cv::imread(im1filename);

	// Solve relative pose
	cv::Mat R, t, K;
	//int isSetPose = getRtStereo(cameramatrix, R, t, K);
	CalibData *calibData = new CalibData();
	readCalibKitti(cameramatrix, calibData);
	K = calibData->k02;
	std::cout << "K: " << K << std::endl;

	int minHessian = 5000;
	sor::CameraPose *camerapose = new sor::CameraPose();
	camerapose->initialize(K, minHessian);
	if (!camerapose->solvePose_8uc3(i0rgb, i1rgb, R, t)) {
		if (!camerapose->filtered_keypoints_im1.empty()) {
			//drawKeypoints(i0rgb, camerapose->filtered_keypoints_im1, i0rgb, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DEFAULT);
		}
	}
	else std::cerr << "Failed to solve for relative pose. " << std::endl;
	std::cout << "Rrel: " << R << std::endl;
	std::cout << "trel: " << t << std::endl;

	// Initialize ReconFlow
	flow->initializeR(width, height, 3, nLevels, scale, sor::ReconFlow::METHODR_TVL1_MS_FNSPARSE,
		lambda, 0.0f, lambdaf, lambdams,
		alphaTv, alphaProj, alphaFn,
		tau, nWarpIters, iters);
	flow->setCameraMatrices(K, K);

	// Open initial matching (flownet)
	flownet = cv::optflow::readOpticalFlow(flownetfilename);
	if (flownet.empty()) {
		std::cerr << "Flownet file not found." << std::endl;
		return 0;
	}

	// Resize images by padding
	cv::copyMakeBorder(i0rgb, i0rgbpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(i1rgb, i1rgbpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(flownet, flownetpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::Mat flownet2[2];   //split flownet channels
	cv::split(flownetpad, flownet2);

	// Copy data to GPU
	flow->copyImagesToDevice(i0rgbpad, i1rgbpad);
	flow->copySparseOpticalFlowToDevice(flownet2[0], flownet2[1]); //can set a mask as third argument

	// Calculate ReconFlow
	flow->solveR(R, t, 50.0f); //main computation iteration

							   // Copy GPU results to CPU
	flow->copyOpticalFlowToHost(u, v, uvrgb); //uvrgb is an optical flow image
	flow->copy3dToHost(X, Y, Z); //3D points

								 // Save output 3D as ply file
	std::vector<cv::Vec3b> colorbuffer(stride*height);
	cv::Mat colorMat = cv::Mat(static_cast<int>(colorbuffer.size()), 1, CV_8UC3, &colorbuffer[0]);
	std::vector<cv::Vec3f> buffer(stride*height);
	cv::Mat cloudMat = cv::Mat(static_cast<int>(buffer.size()), 1, CV_32FC3, &buffer[0]);
	colorbuffer.clear();
	buffer.clear();

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < stride; i++) {
			cv::Vec3b rgb = i0rgbpad.at<cv::Vec3b>(j, i);
			colorbuffer.push_back(rgb);

			float x = X.at<float>(j, i);
			float y = Y.at<float>(j, i);
			float z = Z.at<float>(j, i);
			if (((z <= 75) && (z > 10)) && ((x <= 75) && (y > -75)) && ((y <= 75) && (y > -75))) {
				buffer.push_back(cv::Vec3f(x, -y, -z));
			}
			else {
				x = std::numeric_limits<float>::quiet_NaN();
				y = std::numeric_limits<float>::quiet_NaN();
				z = std::numeric_limits<float>::quiet_NaN();
				buffer.push_back(cv::Vec3f(x, y, z));
			}
		}
	}
	cv::viz::WCloud cloud(cloudMat, colorMat);

	std::ostringstream output3d;
	output3d << outputfilename << suffixFor3D << ".ply";
	cv::viz::writeCloud(output3d.str(), cloudMat, colorMat);

	cv::imshow("flow", uvrgb);
	cv::waitKey();
	return 0;
}

int test_main() {
	std::string mainfolder = "";
	std::string im1filename = "data/im319.png";
	std::string im2filename = "data/im318.png";
	std::string flownetfilename = "data/flo319.flo";
	std::string cameramatrix = mainfolder + "data/calib.txt";
	std::string outputfilename = mainfolder + "data/output/output3d";
	sor::ReconFlow *flow = new sor::ReconFlow(32, 12, 32);

	// Load Params
	float lambda, tau, alphaTv, alphaFn, alphaProj, lambdaf, lambdams, scale;
	int nWarpIters, iters, minWidth;
	std::string suffixFor3D = "oursfn";
	lambda = 50.0f;
	tau = 0.125f;
	alphaTv = 33.3f;
	alphaFn = 100.0f; //1
	alphaProj = 60.0f;
	lambdaf = 0.1f;//0.1
	lambdams = 100.0f;//100
	nWarpIters = 1;
	iters = 10000;
	scale = 2.0f;
	minWidth = 400;

	cv::Mat R, t, K;
	int isSetPose = getRtStereo(cameramatrix, R, t, K);

	// Check image size and compute pyramid nlevels
	std::string initialImage = mainfolder + im1filename;
	cv::Mat iset = cv::imread(initialImage.c_str());
	int width = iset.cols;
	int height = iset.rows;
	int nLevels = 1;
	int pHeight = (int)((float)height / scale);
	while (pHeight > minWidth) {
		nLevels++;
		pHeight = (int)((float)pHeight / scale);
	}
	std::cout << "Pyramid Levels: " << nLevels << std::endl;
	int stride = flow->iAlignUp(width);
	cv::Mat isetpad;
	cv::copyMakeBorder(iset, isetpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);

	// Initialize handler matrices for display and output
	cv::Mat uvrgb = cv::Mat(isetpad.size(), CV_32FC3);
	cv::Mat u = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat v = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat X = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat Y = cv::Mat(isetpad.size(), CV_32F);
	cv::Mat Z = cv::Mat(isetpad.size(), CV_32F);

	// Initialize ReconFlow
	flow->initializeR(width, height, 3, nLevels, scale, sor::ReconFlow::METHODR_TVL1_MS_FNSPARSE,
		lambda, 0.0f, lambdaf, lambdams,
		alphaTv, alphaProj, alphaFn,
		tau, nWarpIters, iters);
	flow->setCameraMatrices(K, K);


	// Open input images
	cv::Mat i0rgb, i1rgb, flownet, i1rgbpad, i0rgbpad, flownetpad, RR, tt;
	std::string flownet2filename = mainfolder + flownetfilename;
	std::string image1filename = mainfolder + im1filename;
	std::string image2filename = mainfolder + im2filename;
	i0rgb = cv::imread(image1filename);
	i1rgb = cv::imread(image2filename);

	// Open initial matching (flownet)
	flownet = cv::optflow::readOpticalFlow(flownet2filename);
	if (flownet.empty()) {
		std::cerr << "Flownet file not found." << std::endl;
		return 0;
	}

	// Resize images by padding
	cv::copyMakeBorder(i0rgb, i0rgbpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(i1rgb, i1rgbpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::copyMakeBorder(flownet, flownetpad, 0, 0, 0, stride - width, cv::BORDER_CONSTANT, 0);
	cv::Mat flownet2[2];   //split flownet channels
	cv::split(flownetpad, flownet2);

	// Copy data to GPU
	flow->copyImagesToDevice(i0rgbpad, i1rgbpad);
	flow->copySparseOpticalFlowToDevice(flownet2[0], flownet2[1]); //can set a mask as third argument

																   // Calculate ReconFlow
	flow->solveR(R, t, 50.0f); //main computation iteration

							   // Copy GPU results to CPU
	flow->copyOpticalFlowToHost(u, v, uvrgb); //uvrgb is an optical flow image
	flow->copy3dToHost(X, Y, Z); //3D points

								 // Save output 3D as ply file
	std::vector<cv::Vec3b> colorbuffer(stride*height);
	cv::Mat colorMat = cv::Mat(static_cast<int>(colorbuffer.size()), 1, CV_8UC3, &colorbuffer[0]);
	std::vector<cv::Vec3f> buffer(stride*height);
	cv::Mat cloudMat = cv::Mat(static_cast<int>(buffer.size()), 1, CV_32FC3, &buffer[0]);
	colorbuffer.clear();
	buffer.clear();

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < stride; i++) {
			cv::Vec3b rgb = i0rgbpad.at<cv::Vec3b>(j, i);
			colorbuffer.push_back(rgb);

			float x = X.at<float>(j, i);
			float y = Y.at<float>(j, i);
			float z = Z.at<float>(j, i);
			if (((z <= 75) && (z > 10)) && ((x <= 75) && (y > -75)) && ((y <= 75) && (y > -75))) {
				buffer.push_back(cv::Vec3f(x, -y, -z));
			}
			else {
				x = std::numeric_limits<float>::quiet_NaN();
				y = std::numeric_limits<float>::quiet_NaN();
				z = std::numeric_limits<float>::quiet_NaN();
				buffer.push_back(cv::Vec3f(x, y, z));
			}
		}
	}
	cv::viz::WCloud cloud(cloudMat, colorMat);

	std::ostringstream output3d;
	output3d << outputfilename << suffixFor3D << ".ply";
	cv::viz::writeCloud(output3d.str(), cloudMat, colorMat);
	return 0;
}

int main(int argc, char **argv)
{
	findCudaDevice(argc, (const char **)argv);
	test_sparseLidar();
	//test_withoutFlownet();
	return 0;
}