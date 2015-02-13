#include "stdafx.h"

#include "opencv2/highgui/highgui.hpp"

#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

#include <stdio.h>

#include <stdlib.h>



using namespace cv;

using namespace std;


Mat src; Mat frameDest;

/// Function header

/**

Function that tracks the hand gestures and counts the number of fingers
*/

void trackCount(Mat &src);

/**

Function that detects skin color

@param src the input image
@param dst the output image


*/
void mySkinDetect(Mat& src, Mat& dst);

/**

Function that calculates the difference between two points

@param a the first point
@param b the second point

*/

float distanceP2P(Point a, Point b);

/*
Function that calculates the angle between three points 

@param s the first point 
@param f the end point
@param e the intersection point of s and f

*/

float getAngle(Point s, Point f, Point e);


/**

Function that returns the maximum of 3 integers

@param a first integer

@param b second integer

@param c third integer

 */

int myMax(int a, int b, int c);



/**

Function that returns the minimum of 3 integers

@param a first integer

@param b second integer

@param c third integer

 */

int myMin(int a, int b, int c);



/** @function main */

int main()
{
    /// Load source image and convert it to gray

    VideoCapture cap(0);

    // if not successful, exit program

    if (!cap.isOpened())

    {

        cout << "Cannot open the video cam" << endl;

        return -1;

    }

    // read a new frame from video

    bool bSuccess0 = cap.read(src);

    //if not successful, break loop

    if (!bSuccess0)

    {

        cout << "Cannot read a frame from video stream" << endl;

    }

    while (1)
    {
        // read a new frame from video

        bool bSuccess = cap.read(src);

        

        //if not successful, break loop

        if (!bSuccess)

        {

            cout << "Cannot read a frame from video stream" << endl;

            //break;

        }


        frameDest = Mat::zeros(src.rows, src.cols, CV_8UC1); //Returns a zero array of same size as src mat, and of type CV_8UC1

		//Detect skin color
		mySkinDetect(src, frameDest);

		//Performs sequence of erosion-dilation in order to remove noise and smooth binary image
		int erosion_size = 3;

		cv::Mat element1 = cv::getStructuringElement(cv::MORPH_ERODE,

                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),

                                                 cv::Point(erosion_size, erosion_size) );

		cv::Mat element2 = cv::getStructuringElement(cv::MORPH_DILATE,

                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),

                                                 cv::Point(erosion_size, erosion_size) );

		erode(frameDest, frameDest, element1);

		dilate(frameDest, frameDest, element2);

		//Draws contours and defects, counts number of fingers
        trackCount(frameDest);

        //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop

        if (waitKey(30) == 27)

        {

            cout << "esc key is pressed by user" << endl;

            //break;

        }

    }

    cap.release();

    return 0;

}


//Function that returns the maximum of 3 integers

int myMax(int a, int b, int c) {

    int m = a;

    (void)((m < b) && (m = b));

    (void)((m < c) && (m = c));

    return m;

}


//Function that returns the minimum of 3 integers

int myMin(int a, int b, int c) {

    int m = a;

    (void)((m > b) && (m = b));

    (void)((m > c) && (m = c));

    return m;

}

//Function that returns the distance between two points

float distanceP2P(Point a, Point b){

    float d= sqrt(fabs( pow(a.x-b.x,2) + pow(a.y-b.y,2) )) ;

    return d;

}

//Function that finds the angle between three points

float getAngle(Point s, Point f, Point e){

    float l1 = distanceP2P(f,s);

    float l2 = distanceP2P(f,e);

    float dot=(s.x-f.x)*(e.x-f.x) + (s.y-f.y)*(e.y-f.y);

    float angle = acos(dot/(l1*l2));

    angle=angle*180/3.14;

    return angle;

}


//Function that detects whether a pixel belongs to the skin based on RGB values

void mySkinDetect(Mat& src, Mat& dst) {

    //Surveys of skin color modeling and detection techniques:

    //Vezhnevets, Vladimir, Vassili Sazonov, and Alla Andreeva. "A survey on pixel-based skin color detection techniques." Proc. Graphicon. Vol. 3. 2003.

    //Kakumanu, Praveen, Sokratis Makrogiannis, and Nikolaos Bourbakis. "A survey of skin-color modeling and detection methods." Pattern recognition 40.3 (2007): 1106-1122.

    for (int i = 0; i < src.rows; i++){

        for (int j = 0; j < src.cols; j++){

            //For each pixel, compute the average intensity of the 3 color channels

            Vec3b intensity = src.at<Vec3b>(i,j); //Vec3b is a vector of 3 uchar (unsigned character)

            int B = intensity[0]; int G = intensity[1]; int R = intensity[2];

            if ((R > 95 && G > 40 && B > 20) && (myMax(R,G,B) - myMin(R,G,B) > 15) && (abs(R-G) > 15) && (R > G) && (R > B)){

                dst.at<uchar>(i,j) = 255;

            }

        }

    }

}

//Function that draws contours and defects, counts number of fingers

void trackCount(Mat &src)
{
    vector<vector<Point> > contours;

    vector<Vec4i> hierarchy;


    findContours(frameDest, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    
    /// Find the convex hull object for each contour

    vector<vector<Point> >hull( contours.size() );

    /// Draw contours + hull results

    Mat drawing = Mat::zeros(frameDest.size(), CV_8UC3 );

    int count = 0; //Counter for the number of fingers

    if(contours.size()>0)
    {
        vector<std::vector<int> >hull( contours.size() );

        vector<vector<Vec4i>> convDef(contours.size() );

        vector<vector<Point>> hull_points(contours.size());

        for( int i = 0; i < contours.size(); i++ )

        {
            if(contourArea(contours[i])>8000) //Only consider large enough objects

            {

                convexHull(contours[i], hull[i], false);

                convexityDefects(contours[i],hull[i], convDef[i]);

                
				//Fills hull_points with Point objects from ints in hull matrix
                for(int k=0;k<hull[i].size();k++)

                {
                    int ind=hull[i][k];

                    hull_points[i].push_back(contours[i][ind]);

                }

                for(int k=0;k<convDef[i].size();k++)

                {

                    if(convDef[i][k][3]>20*256) //only draw defects if they are a certain depth

                    {

                        int ind_0=convDef[i][k][0]; //start point of defect

                        int ind_1=convDef[i][k][1];//end point of defect

                        int ind_2=convDef[i][k][2];//depth of defect

                        cv::circle(drawing,contours[i][ind_0],5,Scalar(0,255,0),-1); //draw start

                        cv::circle(drawing,contours[i][ind_1],5,Scalar(0,255,0),-1);//draw end

                        cv::circle(drawing,contours[i][ind_2],5,Scalar(0,0,255),-1);//draw depth

                        cv::line(drawing,contours[i][ind_2],contours[i][ind_0],Scalar(0,0,255),1);//draw line from depth to start

                        cv::line(drawing,contours[i][ind_2],contours[i][ind_1],Scalar(0,0,255),1);//draw line from depth to end

						//if the angle between the start, depth, and end of a defect is within a certain range, count it as a finger
                        if (getAngle(contours[i][ind_0], contours[i][ind_2], contours[i][ind_1]) > 15 && getAngle(contours[i][ind_0], contours[i][ind_2], contours[i][ind_1]) < 100){

                            count = count + 1;

                        }

                    }

                }

                drawContours( drawing, contours, i, Scalar(255,255,255), 1, 8, vector<Vec4i>(), 0, Point() );

                drawContours( drawing, hull_points, i, Scalar(255,255,255), 1, 8, vector<Vec4i>(), 0, Point() );

            }

        }

    }
    count = count + 1; //Number of defects is one less than number of fingers, so add 1

    string counts = std::to_string(count);

	//Output number of fingers on image
    cv::putText(drawing, counts, cvPoint(30,30),

            FONT_HERSHEY_COMPLEX, 0.8, cvScalar(200,200,250), 1, CV_AA);

    imshow("Tracking", drawing );

}