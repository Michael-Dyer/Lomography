#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <time.h>
#include <vector>
#include <conio.h>
#include <dirent.h>
#include <windows.h>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/utils/filesystem.hpp>

#include <sys/stat.h>
#include <sys/types.h>

#define G_SIZE 256

using namespace cv;
using namespace std;

Mat og_img;

//keeping global 
int color_slider = 10;
int halo_slider = 100;

//making global 
Mat color_merged;
Mat final;
Mat foo;

static void color_on_trackbar(int, void*) {
	//create Mat arr of 3 color bands to be merged back together.
	Mat Bands[3];
	
	

	
	
	//1x256 lookup table for im
	Mat color_lut(1, 256, CV_8UC1);

	//calculate e for the lut math
	double e = exp(1);

	float s = (float)color_slider / 100;

	/*
	//make sure s isn't lower than .08
	if (s < 0.08) {
		s = 0.08;
	}
	*/

	//populate lut
	for (int i = 0; i < 256; i++) {

		//e's exponent
		float e_exp = (((float(i) / 256) - 0.5) / s);
		
		uchar lut_value = round(256 / (1 + pow(e,-e_exp )));
		
		color_lut.at<uchar>(i) = lut_value;
	}

	//split, apply lut, merge
	
	split(og_img, Bands);
	//split into bgr
	vector<Mat> channels = { Bands[0],Bands[1],Bands[2] };

	LUT(Bands[2], color_lut, Bands[2]);

	merge(channels, color_merged);
	
	imshow("lomography", color_merged);
	
}

//helper function for halo_on_trackbar
bool within_distance(int x_org, int y_org, int x, int y, int r);

static void halo_on_trackbar(int, void*) {
	//takes halo slider as input


	//find the maximum radius by getting the furthest it could go out
	int max_radius = min(og_img.rows, og_img.cols);
	float radius_precent = ((float)halo_slider / 100);

	int radius = (max_radius * radius_precent)/2;
	//keep radius above 0 to prevent errors
	if (radius == 0) {
		radius = 1;
	}

	//if color_merged doesn't exist, take in og_img instead
	if (color_merged.empty()) {
		
		og_img.copyTo(color_merged);
	}

	//make 3 channel mat and populate with 0.75(gray)
	Mat halo_mat(og_img.rows, og_img.cols, CV_32FC3);

	for (int i = 0; i < halo_mat.rows; i++) {
		for (int j = 0; j < halo_mat.cols; j++) {
			halo_mat.at<Vec3f>(i, j)[0] = 0.75;
			halo_mat.at<Vec3f>(i, j)[1] = 0.75;
			halo_mat.at<Vec3f>(i, j)[2] = 0.75;
		}
	}

	//place white cirlce in halo mat

	//get midpoints of mat
	int x_start = round(halo_mat.cols / 2);
	int y_start = round(halo_mat.rows / 2);

	
	for (int i = 0; i < halo_mat.rows; i++) {
		for (int j = 0; j < halo_mat.cols; j++) {
			
			if (within_distance(x_start, y_start, i, j, radius)) {
				halo_mat.at<Vec3f>(i, j)[0] = 1.0;
				halo_mat.at<Vec3f>(i, j)[1] = 1.0;
				halo_mat.at<Vec3f>(i, j)[2] = 1.0;
			}

		}
	}
	
	
	blur(halo_mat, halo_mat, Size(radius, radius));

	

	//convert so it can be multiplied 
	color_merged.convertTo(final, CV_32FC3);
	

	multiply(final, halo_mat, final);

	//convert to viewable format
	final.convertTo(final, CV_8UC3);

	//imshow("lomography", color_merged);
	imshow("lomography", final);

}

int main(int argc, const char** argv) {


	uchar input = 'y';

	int histo_method = 1;


	Mat image;

	try {


		//these are the keys for the comand line parser
		String keys =
			"{h help  |      | show help message}"     // optional, show help optional
			"{@dir | <none>        | Input file}";



		CommandLineParser parser(argc, argv, keys);
		if (parser.has("h") || parser.has("help")) {
			cout << "To run program, execute; program_name input file";
			cout << " adjust sliders to change image" << endl;
			cout << "press s to save" << endl;
			cout << "press q to exit image" << endl;
			parser.printMessage();
			return 0;
		}



		//gets mandatory value, the original file from the parser
		String og_file = parser.get<String>(0);
		//og_file = og_file + ".png";




		og_img = imread(og_file, IMREAD_COLOR);



		if (og_img.empty())
		{
			std::cout << "Could not read the image: " << og_file << std::endl;
			return 1;
		}




		

		//this stuff is for display~~
		namedWindow("lomography", WINDOW_AUTOSIZE); // Create a window for display.
		moveWindow("lomography", 0, 0);


		createTrackbar("color", "lomography", &color_slider, 20, color_on_trackbar);
		createTrackbar("halo", "lomography", &halo_slider, 100, halo_on_trackbar);

		if (final.empty()) {
			og_img.copyTo(final);
		}
		
		
		do {
			imshow("lomography", final);
			input = waitKey();

			if (input == 's') {
				imwrite("saved.jpg", final);
				cout << "saved image as saved.jpg" << endl;
			}


		} while (input != 'q' && input != 'Q');
		//allow s to save the displayed picture
		

	


		cout << "end";


	}
	catch (std::string& str) {
		std::cerr << "Error: " << argv[0] << ": " << str << std::endl;
		return (1);
	}
	catch (cv::Exception& e) {
		std::cerr << "Error: " << argv[0] << ": " << e.msg << std::endl;
		return (1);
	}


	return (0);
}

//implement distance formula
bool within_distance(int x_org, int y_org, int x, int y, int r) {
	int in_sqrt = pow((x_org - x), 2) + pow((y_org - y), 2);
	
	if (r >= sqrt(in_sqrt)) {
		return true;
	}
	else {
		return false;
	}

}