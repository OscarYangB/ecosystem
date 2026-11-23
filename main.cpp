#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include "platform_render.h"
#include <iostream>
#include "grid.h"
#include <box2d/box2d.h>
#include "ant.h"
#include "physics.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_mutex.h>
#include <random>

using RandomGeneratorType = std::mt19937;
RandomGeneratorType random_generator;

float random_float(float min_inclusive, float min_exclusive) {
    std::uniform_real_distribution<float> distribution(min_inclusive, min_exclusive);
    return distribution(random_generator);
}

b2Vec2 id_to_dimensions[] = {
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

cv::VideoCapture video;
cv::Mat frame;
cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

static int SDLCALL computer_vision_thread(void* ptr) {
    while (true) {
        video >> frame;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        cv::aruco::ArucoDetector detector(dictionary, detectorParams);
        detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);

        //std::cout << markerIds.size() << "\n";
    }
}

int main() {
    std::random_device device{};
    random_generator = std::mt19937{ device() };

	video = cv::VideoCapture(0);
    video.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    video.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    if (!video.isOpened())
    {
        std::cout << "Could not open video.";
        return -1;
    }
    SDL_CreateThread(computer_vision_thread, "TestThread", (void*)NULL);

    static constexpr char* WINDOW = "window";
    cv::namedWindow(WINDOW, cv::WINDOW_AUTOSIZE);

    start_window();

    init_physics();
    std::vector<Ant> ants = {};
    ants.reserve(50); // BUG: Vectors reallocate causing destructor to be called
    for (int i = 0; i < 50; i++) {
        ants.emplace_back(5.f, 5.f, random_float(0.f, 1920.f), random_float(0.f, 1080.f));
    }


    Uint64 time = SDL_GetTicks();
    
    while (true) {
        Uint64 new_time = SDL_GetTicks();
        double deltatime = (new_time - time) / 1000.f;
        time = new_time;

        start_render();

        //video >> frame;

        //std::vector<int> markerIds;
        //std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        //cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
        //cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
        //cv::aruco::ArucoDetector detector(dictionary, detectorParams);
        //detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);
        //cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

        //for (int i = 0; i < markerIds.size(); i++) {
        //    cv::Point2f center = (markerCorners[i][0] + markerCorners[i][1] + markerCorners[i][2] + markerCorners[i][3]) / 4.f;
        //    center.x = (center.x / frame.cols) * WIDTH;
        //    center.y = (center.y / frame.rows) * HEIGHT;
        //    b2Vec2 dimensions = id_to_dimensions[markerIds[i]];
        //    draw_rect_around_point(center.x, center.y, dimensions.x, dimensions.y);
        //}
        static float timer = 5.f;

        update_physics(deltatime);
        for (Ant& ant : ants) {
            b2Body_ApplyForceToCenter(ant.body_id, ant.force_direction * 5000.f, true);
            timer += deltatime;
            if (timer > 2.f) {
                timer = 0.f;
                ant.force_direction = {random_float(-1.f, 1.f), random_float(-1.f, 1.f) };
            }

            b2Pos position = b2Body_GetPosition(ant.body_id);
            b2Rot rotation = b2Body_GetRotation(ant.body_id);
            b2Vec2 velocity = b2Body_GetLinearVelocity(ant.body_id);
            render_entity(position.x, position.y, atan2(velocity.y, velocity.x));
        }

        std::cout << 1.f / deltatime << "\n";


        /*for (int i = 0; i < GRID_WIDTH; i++) {
            for (int j = 0; j < GRID_HEIGHT; j++) {
                cv::Vec3b intensity = frame.at<cv::Vec3b>(i, j);
            }
        }*/

        end_render();

        //cv::imshow(WINDOW, frame);
        //char c = (char)cv::waitKey(1);
    }
}
