#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include "platform_render.h"
#include <iostream>
#include "vector2.h"

Vector2 id_to_dimensions[] = {
    {100,100},
    {100,50},
    {50,100},
    {75,75},
    {150,150},
    {150,200},
    {100,300},
    {300,100},
    {400,200},
    {50,200},
};


int main() {
	cv::VideoCapture video(0);

    if (!video.isOpened())
    {
        std::cout << "Could not open video.";
        return -1;
    }

    static constexpr char* WINDOW = "window";
    cv::namedWindow(WINDOW, cv::WINDOW_AUTOSIZE);

    start_window();

    cv::Mat frame;
    while (true) {
        start_render();

        video >> frame;

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
        cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
        cv::aruco::ArucoDetector detector(dictionary, detectorParams);
        detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);
        cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

        for (int i = 0; i < markerIds.size(); i++) {
            cv::Point2f center = (markerCorners[i][0] + markerCorners[i][1] + markerCorners[i][2] + markerCorners[i][3]) / 4.f;
            center.x = (center.x / frame.cols) * WIDTH;
            center.y = (center.y / frame.rows) * HEIGHT;
            Vector2 dimensions = id_to_dimensions[markerIds[i]];
            draw_rect_around_point(center.x, center.y, dimensions.x, dimensions.y);
        }

        end_render();

        cv::imshow(WINDOW, frame);
        char c = (char)cv::waitKey(16);
    }
}
