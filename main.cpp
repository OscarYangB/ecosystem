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

b2Vec2 tree_positions[] = {
    { 400.f, 400.f },
    { 500.f, 440.f },
    { 660.f, 420.f },
    { 720.f, 430.f },
    { 800.f, 460.f },

    { 1000.f, 1000.f },
    { 1200.f, 900.f }
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
cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

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
    static constexpr float TREE_SIZE = 350.f;
    Animation animation{};
    b2BodyId body_id;
    float scale;
    bool dead = false;
    float grow_timer;
    float death_timer;
    float r;
    float g;
    float b;

    inline void reset_grow_timer(size_t number_of_trees) {
        grow_timer = random_float(40.f, 120.f) * std::pow(1.1f, number_of_trees);
    }

    inline void init(float x, float y) {
        scale = random_float(0.8f, 1.2f);
        body_id = create_static_physics_entity(x, y, random_float(0.f, 360.f), (TREE_SIZE * scale) / 2.f);
        animation.textures = { tree_1, tree_2, tree_3 };
        animation.framerate = random_float(4.f, 8.f);
        reset_grow_timer(0);
        death_timer = random_float(60.f, 180.f);
        r = random_float(0.9f, 1.f);
        g = random_float(0.9f, 1.f);
        b = random_float(0.9f, 1.f);
    }
};

std::vector<Tree> trees = {};

int main() {
    //for (int i = 0; i < 10; i++) {
    //    cv::Mat markerImage;
    //    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    //    cv::aruco::generateImageMarker(dictionary, i, 300, markerImage, 1);
    //    cv::imwrite("marker" + std::to_string(i) + ".png", markerImage);
    //}
    //return 0;

    mutex = SDL_CreateMutex();

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

        update_physics(deltatime);
        for (Ant& ant : ants) {
            ant.direction_timer -= deltatime;
            if (ant.direction_timer < 0.f) {
                ant.direction_timer = random_float(3.f, 6.f);                
                ant.intended_direction = b2Normalize(b2RotateVector(b2MakeRot(random_float(-degtorad(30), degtorad(30))), b2Body_GetLinearVelocity(ant.body_id)));
            }

            ant.velocity = exponential_decay(ant.velocity, ant.intended_direction, 1, deltatime);
            b2Body_SetLinearVelocity(ant.body_id, ant.velocity * ant.speed);
            b2Vec2 position = b2Body_GetPosition(ant.body_id);

            render_entity(position.x, position.y, std::atan2(ant.velocity.y, ant.velocity.x), 50, 50, jermaine);
        }
        auto events = b2World_GetContactEvents(world_id);
        for (int i = 0; i < events.beginCount; i++) {
            auto body_a = b2Shape_GetBody(events.beginEvents[i].shapeIdA);
            auto body_b = b2Shape_GetBody(events.beginEvents[i].shapeIdB);
            for (Ant& ant : ants) {
                if (B2_ID_EQUALS(ant.body_id, body_a) || B2_ID_EQUALS(ant.body_id, body_b)) {
                    ant.intended_direction = b2RotateVector(b2MakeRot(random_float(-degtorad(90), degtorad(270))), ant.intended_direction);
                }
            }
        }
        

        for (int i = 0; i < trees.size(); i++) {
            trees[i].animation.update(deltatime);
            b2Pos position = b2Body_GetPosition(trees[i].body_id);
            SDL_SetTextureColorMod(trees[i].animation.get_texture(), trees[i].r * 255, trees[i].g * 255, trees[i].b * 255);
            const float tree_size = Tree::TREE_SIZE * trees[i].scale;
            render_entity(position.x, position.y, b2Rot_GetAngle(b2Body_GetRotation(trees[i].body_id)), tree_size, tree_size, trees[i].animation.get_texture());

            trees[i].grow_timer -= deltatime;
            if (trees[i].grow_timer < 0.f) {
                trees[i].reset_grow_timer(trees.size());
                trees.emplace_back();              
                trees[trees.size() - 1].init(position.x + random_float(-tree_size/4.f, tree_size/4.f), position.y + random_float(-tree_size/4.f, tree_size/4.f));
            }

            trees[i].death_timer -= deltatime;
            if (trees[i].death_timer < 0.f) {
                trees[i].dead = true;
            }
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
