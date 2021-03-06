//
//  main.cpp
//  Moving_Object_Tracking
//
//  Created by Noah on 2015/12/26.
//  Copyright © 2015年 Noah. All rights reserved.
//

#include <iostream>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//our sensitivity value to be used in the absdiff() function
const static int SENSITIVITY_VALUE = 30;
//size of blur used to smooth the intensity image output from absdiff() function
const static int BLUR_SIZE = 5;
//we'll have just one object to search for
//and keep track of its position.
int theObject[2] = {0,0};
//bounding rectangle of the object, we will use the center of this as its position.
Rect objectBoundingRectangle = Rect(0,0,0,0);


//int to string helper function
string intToString(int number){
    
    //this function has a number input and string output
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void searchForMovement(Mat thresholdImage, Mat &cameraFeed){
    
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    //findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
    findContours(thresholdImage, contours, hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE );// retrieves external contours
    
    if(contours.size() > 0){
        //the largest contour is found at the end of the contours vector
        //we will simply assume that the biggest contour is the object we are looking for.
        vector< vector<Point> > largestContourVec;
        largestContourVec.push_back(contours.at(contours.size()-1));
        
        //vector<Point> tempPoints =largestContourVec.at(0);
        //tprintf("there are %lu points\n",largestContourVec.at(0).size());
        objectBoundingRectangle = boundingRect(largestContourVec.at(0));
        int xpos = objectBoundingRectangle.x+objectBoundingRectangle.width/2;
        int ypos = objectBoundingRectangle.y+objectBoundingRectangle.height/2;
        
        //update the objects positions by changing the 'theObject' array values
        theObject[0] = xpos , theObject[1] = ypos;
        
        //make some temp x and y variables so we dont have to type out so much
        int x = theObject[0];
        int y = theObject[1];
        
        //draw some crosshairs around the object
        circle(cameraFeed,Point(x,y),40,Scalar(0,255,0),7);
        line(cameraFeed,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
        line(cameraFeed,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
        line(cameraFeed,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
        line(cameraFeed,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
        
        //write the position of the object to the screen
        putText(cameraFeed,"Tracking object at (" + intToString(x)+","+intToString(y)+")",Point(x,y),1,1,Scalar(255,0,0),2);
    }
    
}

int main(){
    
    bool debugMode = false;
    bool trackingEnabled = false;
    bool pause = false;
    
    Mat Image,debugImage;;
    Mat currentMat,prevMat;
    Mat currentDiff,prevDiff;
    Mat thresholdImage;
    
    VideoCapture capture;
    
    capture.open(0);
    
    if(!capture.isOpened()){
        cout<<"ERROR ACQUIRING VIDEO FEED\n";
        getchar();
        return -1;
    }
    
    while(1){

        capture.read(Image);
        Image.copyTo(currentMat);
        cv::cvtColor(currentMat,currentMat, COLOR_BGR2GRAY);
        
        if (prevMat.empty())
        {
            currentMat.copyTo(prevMat);
            continue;
        }
        
        cv::absdiff(currentMat, prevMat, currentDiff);
        
        //threshold intensity image at a given sensitivity
        cv::threshold(currentDiff,currentDiff,SENSITIVITY_VALUE,255,THRESH_BINARY);
        
        //blur the image to get rid of the noise.
        cv::blur(currentDiff,currentDiff,cv::Size(BLUR_SIZE,BLUR_SIZE));

        //threshold again to obtain binary image from blur output
        cv::threshold(currentDiff,currentDiff,SENSITIVITY_VALUE,255,THRESH_BINARY);

        if (prevDiff.empty())
        {
            currentDiff.copyTo(prevDiff);
        }

        cv::bitwise_and(currentDiff, prevDiff, thresholdImage);
        if(debugMode == true){
            //show the threshold image after it's been "blurred"
            flip(thresholdImage, debugImage, 1);
            imshow("Final Threshold Image",debugImage);
            
        }
        else {
            //if not in debug mode, destroy the windows so we don't see them anymore
            cv::destroyWindow("Final Threshold Image");
        }
        
        //if tracking enabled, search for contours in our thresholded image
        if(trackingEnabled){
            searchForMovement(thresholdImage,Image);
        }
        
        flip(Image, Image, 1);
        //show our captured frame
        imshow("Frame1",Image);
        
        currentMat.copyTo(prevMat);
        currentDiff.copyTo(prevDiff);
        
        switch(waitKey(10)){
                
            case 27: //'esc' key has been pressed, exit program.
                capture.release();
                return 0;
            case 116: //'t' has been pressed. this will toggle tracking
                trackingEnabled = !trackingEnabled;
                if(trackingEnabled == false) cout<<"Tracking disabled."<<endl;
                else cout<<"Tracking enabled."<<endl;
                break;
            case 100: //'d' has been pressed. this will debug mode
                debugMode = !debugMode;
                if(debugMode == false) cout<<"Debug mode disabled."<<endl;
                else cout<<"Debug mode enabled."<<endl;
                break;
            case 112: //'p' has been pressed. this will pause/resume the code.
                pause = !pause;
                if(pause == true){
                    cout<<"Code paused, press 'p' again to resume"<<endl;
                    while (pause == true){
                        //stay in this loop until
                        if (!currentMat.empty())
                        {
                            currentMat.release();
                            prevMat.release();
                            currentDiff.release();
                            prevDiff.release();
                        }
                        switch (waitKey()){
                                //a switch statement inside a switch statement? Mindblown.
                            case 112:
                                //change pause back to false
                                pause = false;
                                cout<<"Code Resumed"<<endl;
                                break;
                        }
                    }
                }
        }
        //capture.release();
    }
    return 0;
    
}
