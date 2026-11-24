#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include "platform_render.h"
#include <iostream>
#include <box2d/box2d.h>
#include "ant.h"
#include "physics.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_mutex.h>
#include <random>

struct Animation {
    std::vector<SDL_Texture*> textures = {};
    int index = 0;
    bool forward = true;
    float framerate = 6.f;
    float timer = 0;

    inline SDL_Texture* get_texture() {
        return textures[index];
    }

    inline void update(float deltatime) {
        timer += deltatime;
        if (timer > 1.f / framerate) {
            timer = 0.f;
            if (forward) {
                index++;
                if (index > textures.size() - 1) {
                    forward = false;
                    index = textures.size() - 2;
                }
            }
            else {
                index--;
                if (index < 0) {
                    forward = true;
                    index = 2;
                }
            }
        }
    }
};

using RandomGeneratorType = std::mt19937;
RandomGeneratorType random_generator;

float random_float(float min_inclusive, float min_exclusive) {
    std::uniform_real_distribution<float> distribution(min_inclusive, min_exclusive);
    return distribution(random_generator);
}

b2Vec2 tree_positions[] = {
    { 400.f, 400.f },
    { 700.f, 700.f }
};

struct Monument {
    b2Vec2 position{};
    int frames_last_seen{};
    Animation animation{};
};

constexpr size_t NUMBER_OF_MONUMENTS = sizeof(MONUMENT_DIMENSIONS) / sizeof(MONUMENT_DIMENSIONS[0]);
Monument monuments[NUMBER_OF_MONUMENTS]{};

cv::VideoCapture video;
cv::Mat frame;
cv::Mat cropped_frame;
cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

struct DetectedStructure {
    b2Vec2 position;
    int id;
};

std::vector<DetectedStructure> detected_structures{};
bool detected_strucutures_dirty = false;
SDL_Mutex* mutex = nullptr;

static int SDLCALL computer_vision_thread(void* ptr) {
    while (true) {
        video >> frame;
        static constexpr float crop_ratio = 0.7f;
        //static constexpr float crop_ratio = 0.7f;
        //cropped_frame = frame(cv::Rect(50, 50, 720, ));
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        cv::aruco::ArucoDetector detector(dictionary, detectorParams);
        detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);

        SDL_LockMutex(mutex);
        detected_structures.clear();
        for (int i = 0; i < markerIds.size(); i++) {
            cv::Point2f center = (markerCorners[i][0] + markerCorners[i][1] + markerCorners[i][2] + markerCorners[i][3]) / 4.f;
            center.x = (center.x / frame.cols) * WIDTH;
            center.y = (center.y / frame.rows) * HEIGHT;
            detected_structures.emplace_back(DetectedStructure{b2Vec2{ center.x, center.y }, markerIds[i]});
        }
        detected_strucutures_dirty = true;
        SDL_UnlockMutex(mutex);

    }
}

struct Tree {
    static constexpr float TREE_SIZE = 700.f;
    Animation animation{};
    b2BodyId body_id;
    float scale;
    bool dead = false;

    void init(float x, float y) {
        scale = random_float(0.8f, 1.2f);
        body_id = create_static_physics_entity(x, y, random_float(0.f, 360.f), TREE_SIZE * scale);
        animation.textures = { tree_1, tree_2, tree_3 };
    }
};

std::vector<Tree> trees = {};

int main() {
    mutex = SDL_CreateMutex();

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

    for (Monument& monument : monuments) {
        monument.animation.textures = { circle_1, circle_2, circle_3 };
        monument.frames_last_seen = 300;
    }
    
    init_physics();
    std::vector<Ant> ants = {};
    for (int i = 0; i < 50; i++) {
        Ant ant{};
        ant.init(5.f, 5.f, random_float(0.f, 1920.f), random_float(0.f, 1080.f));
        ants.emplace_back(ant);
    }

    for (const b2Vec2& tree_position : tree_positions) {
        Tree tree{};
        tree.init(tree_position.x, tree_position.y);
        trees.emplace_back(tree);
    }

    Uint64 time = SDL_GetTicks();
    
    while (true) {
        Uint64 new_time = SDL_GetTicks();
        double deltatime = (new_time - time) / 1000.f;
        time = new_time;

        start_render();

        SDL_LockMutex(mutex);
        bool should_refresh_monuments = detected_strucutures_dirty;
        if (detected_strucutures_dirty) {
            for (int i = 0; i < detected_structures.size(); i++) {
                monuments[detected_structures[i].id].frames_last_seen = 0;
                monuments[detected_structures[i].id].position = detected_structures[i].position;
            }
            detected_strucutures_dirty = false;
        }
        SDL_UnlockMutex(mutex);

        if (should_refresh_monuments) {
            for (int i = 0; i < NUMBER_OF_MONUMENTS; i++) {
                if (monuments[i].frames_last_seen == 0) {
                    move_monument(i, monuments[i].position);

                    for (Ant& ant : ants) {
                        if (b2Distance(b2Body_GetPosition(ant.body_id), monuments[i].position) < std::max(MONUMENT_DIMENSIONS[i].x / 2.f, MONUMENT_DIMENSIONS[i].y / 2.f)) {
                            ant.dead = true;
                        }
                    }
                    for (Tree& tree : trees) {
                        if (b2Distance(b2Body_GetPosition(tree.body_id), monuments[i].position) < std::max(MONUMENT_DIMENSIONS[i].x, MONUMENT_DIMENSIONS[i].y)) {
                            tree.dead = true;
                        }
                    }
                }
            }
        }

        static float timer = 5.f;

        update_physics(deltatime);
        for (Ant& ant : ants) {
            b2Vec2 forward_direction = b2RotateVector(b2Body_GetRotation(ant.body_id), b2Vec2{1.f, 0.f});
            b2Vec2 force_position = b2Body_GetPosition(ant.body_id) + forward_direction * 50.f;
            b2Body_ApplyForce(ant.body_id, ant.force_direction * 2000.f, force_position, true);
            //DrawLine(force_position.x, force_position.y, force_position.x + ant.force_direction.x * 50.f, force_position.y + ant.force_direction.y * 50.f);
            timer += deltatime;
            if (timer > 10.f) {
                timer = 0.f;
                ant.force_direction = {random_float(-1.f, 1.f), random_float(-1.f, 1.f) };
            }

            b2Pos position = b2Body_GetPosition(ant.body_id);
            render_entity(position.x, position.y, b2Rot_GetAngle(b2Body_GetRotation(ant.body_id)), 100, 100, jermaine);
        }

        for (Tree& tree : trees) {
            tree.animation.update(deltatime);
            b2Pos position = b2Body_GetPosition(tree.body_id);
            render_entity(position.x, position.y, b2Rot_GetAngle(b2Body_GetRotation(tree.body_id)), Tree::TREE_SIZE, Tree::TREE_SIZE, tree.animation.get_texture());
        }

        static constexpr size_t FRAMES_UNTIL_DISABLE = 5;
        if (should_refresh_monuments) {
            for (int i = 0; i < NUMBER_OF_MONUMENTS; i++) {
                if (monuments[i].frames_last_seen > FRAMES_UNTIL_DISABLE) {
                    continue;
                }
                monuments[i].frames_last_seen++;
                if (monuments[i].frames_last_seen == FRAMES_UNTIL_DISABLE) {
                    disable_monument(i);
                }
            }
        }

        for (int i = 0; i < NUMBER_OF_MONUMENTS; i++) {
            if (is_monument_enabled(i)) {
                monuments[i].animation.update(deltatime);
                render_entity(monuments[i].position.x, monuments[i].position.y, 0.f, MONUMENT_DIMENSIONS[i].x, MONUMENT_DIMENSIONS[i].y, monuments[i].animation.get_texture());
            }
        }

        auto remove_iter = std::remove_if(ants.begin(), ants.end(), [](const Ant& ant) {return ant.dead; });
        ants.erase(remove_iter, ants.end());
        auto remove_iter_tree = std::remove_if(trees.begin(), trees.end(), [](const Tree& tree) {return tree.dead; });
        trees.erase(remove_iter_tree, trees.end());


        //std::cout << 1.f / deltatime << "\n";

        end_render();

        //cv::imshow(WINDOW, frame);
        //char c = (char)cv::waitKey(1);
    }
}
