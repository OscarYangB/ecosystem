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
#include "opencv2/geometry.hpp"

static constexpr char* WINDOW = "window";

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
    { 1200.f, 900.f },

    { 1500.f, 200.f },
    { 1400.f, 100.f },
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
cv::aruco::ArucoDetector detector;

struct DetectedStructure {
    b2Vec2 position;
    int id;
};

std::vector<DetectedStructure> detected_structures{};
bool detected_strucutures_dirty = false;
SDL_Mutex* mutex = nullptr;

cv::Mat perspective_matrix;
bool apply_perspective = false;

void test() {
    video = cv::VideoCapture(0);
    video.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    video.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    if (!video.isOpened())
    {
        std::cout << "Could not open video.";
    }
    //detectorParams.minMarkerPerimeterRate = 0.003;
    //detectorParams.minOtsuStdDev = 1.f;
    detector = cv::aruco::ArucoDetector(dictionary, detectorParams);

    static constexpr char* WINDOW = "window";
    cv::namedWindow(WINDOW, cv::WINDOW_AUTOSIZE);

    while (true) {
        video >> frame;

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);
        cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

        cv::imshow(WINDOW, frame);
        char c = (char)cv::waitKey(16);
    }


}

static int SDLCALL computer_vision_thread(void* ptr) {
    while (true) {
        video >> frame;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);

        SDL_LockMutex(mutex);
        detected_structures.clear();
        for (int i = 0; i < markerIds.size(); i++) {
            cv::Point2f center = (markerCorners[i][0] + markerCorners[i][1] + markerCorners[i][2] + markerCorners[i][3]) / 4.f;

            if (apply_perspective) {
                std::array<cv::Point2f, 1> output = { center };
                cv::perspectiveTransform(std::array<cv::Point2f, 1>{center}, output, perspective_matrix);
                center = output[0];
                center.x *= get_screen_width();
                center.y *= get_screen_height();;
            }
            else {
                center.x = (center.x / frame.cols) * WIDTH;
                center.y = (center.y / frame.rows) * HEIGHT;
            }

            detected_structures.emplace_back(DetectedStructure{b2Vec2{ center.x, center.y }, markerIds[i]});
        }
        detected_strucutures_dirty = true;
        SDL_UnlockMutex(mutex);

    }
}

enum class Species {
    Deer,
    Frog,
    Spider,
};

struct Ant {
    float health;
    float speed;
    float cooldown_timer = 0.f;
    b2BodyId body_id;
    b2Vec2 intended_direction = { 1.f, 1.f };
    b2Vec2 velocity = { 1.f, 1.f };
    float direction_timer = 0.f;
    bool dead = false;
    Animation animation{};
    Species species;

    inline void init(float x, float y, Species species) {
        this->species = species;
        body_id = create_physics_entity(x, y, 20.f);
        health = random_float(60.f, 180.f);
        speed = random_float(8.f, 12.f) * 4.f;
        if (species == Species::Spider) {
            animation.textures = { spider_1, spider_2, spider_3 };
        }
        else if (species == Species::Deer) {
            animation.textures = { deer_1, deer_2, deer_3, deer_4 };
        }
        else if (species == Species::Frog) {
            animation.textures = { frog_1, frog_2, frog_3 };
        }
    }

    inline void destroy() {
        destroy_physics_entity(body_id);
    }
};


struct Food {
    b2Vec2 position;
    float r;
    float g;
    float b;
    int type;
    float rotation;
};

struct Tree {
    static constexpr float TREE_SIZE = 250.f;
    static constexpr int HEALTH_MORE_THAN_FRUIT = 2;
    Animation animation{};
    b2BodyId body_id;
    float scale;
    bool dead = false;
    float grow_timer;
    float death_timer;
    float r;
    float g;
    float b;
    std::vector<Food> food = {};
    int health;

    inline void reset_grow_timer(size_t number_of_trees) {
        grow_timer = random_float(30.f, 100.f);
    }

    inline void init(float x, float y) {
        scale = random_float(0.8f, 1.2f);
        body_id = create_static_physics_entity(x, y, random_float(0.f, 360.f), (TREE_SIZE * scale) / 2.f);
        animation.textures = { tree_1, tree_2, tree_3 };
        animation.framerate = random_float(4.f, 8.f);
        reset_grow_timer(0);
        death_timer = random_float(60.f, 140.f);
        r = random_float(0.9f, 1.f);
        g = random_float(0.9f, 1.f);
        b = random_float(0.9f, 1.f);
        

        int fruit = random_integer(3, 6);
        health = fruit + HEALTH_MORE_THAN_FRUIT;
        for (int i = 0; i < fruit; i++) {
            float variance = TREE_SIZE / 4.f;
            Food new_food = { b2Vec2{ random_float(x - variance, x + variance), random_float(y - variance, y + variance) },
                random_float(0.9f, 1.f), random_float(0.9f, 1.f), random_float(0.9f, 1.f), random_integer(0, 10), random_float(0.f, 2.f*3.14)};
            food.emplace_back(new_food);
        }
    }

    inline void destroy() {
        destroy_physics_entity(body_id);
    }
};

std::vector<Tree> trees = {};
std::vector<Ant> ants = {};

b2Vec2 get_closest_tree_position(b2Vec2 position) {
    float smallest_distance = 500000.f;
    b2Vec2 closest_tree = {0.f, 0.f};
    for (Tree& tree : trees) {
        b2Vec2 tree_position = b2Body_GetPosition(tree.body_id);
        float distance = b2Distance(tree_position, position);
        if (distance < smallest_distance) {
            smallest_distance = distance;
            closest_tree = tree_position;
        }
    }
    return closest_tree;
}

b2Vec2 get_closest_ant_position(b2Vec2 position, Ant* ignore) {
    float smallest_distance = 500000.f;
    b2Vec2 closest_ant = { 1.f, 0.f};
    for (Ant& ant : ants) {
        if (&ant == ignore) {
            continue;
        }
        b2Vec2 ant_position = b2Body_GetPosition(ant.body_id);
        float distance = b2Distance(closest_ant, position);
        if (distance < smallest_distance) {
            smallest_distance = distance;
            closest_ant = ant_position;
        }
    }
    return closest_ant;
}

void calibrate() {
    draw_white();
    start_render();
    const float code_size = 500.f;
    const float margin = 10.f;
    const int screen_width = get_screen_width() - margin;
    const int screen_height = get_screen_height() - margin;
    render_code(margin, margin, code_size, code_size, code_1);
    render_code(screen_width - code_size, margin, code_size, code_size, code_2);
    render_code(margin, screen_height - code_size, code_size, code_size, code_3);
    render_code(screen_width - code_size, screen_height - code_size, code_size, code_size, code_4);
    end_render();
    draw_black();

    SDL_Delay(5000.f);

    cv::Point2f top_left = { -1.f, -1.f };
    cv::Point2f top_right = { -1.f, -1.f };
    cv::Point2f bottom_left = { -1.f, -1.f };
    cv::Point2f bottom_right = { -1.f, -1.f };
    bool found[4] = {false, false, false, false};

    video >> frame;

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    detector.detectMarkers(frame, markerCorners, markerIds, rejectedCandidates);
    for (int i = 0; i < markerIds.size(); i++) {
        if (markerIds[i] == 1) {
            top_left = markerCorners[i][0];
            found[0] = true;
        }
        else if (markerIds[i] == 2) {
            top_right = markerCorners[i][1];
            found[1] = true;
        }
        else if (markerIds[i] == 3) {
            bottom_left = markerCorners[i][3];
            found[2] = true;
        }
        else if (markerIds[i] == 4) {
            bottom_right = markerCorners[i][2];
            found[3] = true;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (!found[i]) {
            apply_perspective = false;
            return;
        }
    }

    apply_perspective = true;
    cv::Point2f corners[4] = { top_left, top_right, bottom_left, bottom_right };
    cv::Point2f unit_square[4] = { { 0.f,0.f },{1.f, 0.f},{0.f, 1.f},{1.f, 1.f} };
    perspective_matrix = cv::getPerspectiveTransform(corners, unit_square);
}

int main() {
    //test();

    //for (int i = 0; i < 10; i++) {
    //    cv::Mat markerImage;
    //    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    //    cv::aruco::generateImageMarker(dictionary, i, 400, markerImage, 1);
    //    cv::imwrite("marker" + std::to_string(i) + ".png", markerImage);
    //}
    //return 0;
    video = cv::VideoCapture(0);
    video.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    video.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    if (!video.isOpened())
    {
        std::cout << "Could not open video.";
        return -1;
    }
    //detectorParams.minMarkerPerimeterRate = 0.003;
    //detectorParams.minOtsuStdDev = 1.f;
    detector = cv::aruco::ArucoDetector(dictionary, detectorParams);
    start_window();
    calibrate();

    mutex = SDL_CreateMutex();

    SDL_CreateThread(computer_vision_thread, "TestThread", (void*)NULL);

    for (Monument& monument : monuments) {
        monument.animation.textures = { circle_1, circle_2, circle_3 };
        monument.frames_last_seen = 300;
    }
    
    init_physics();
    for (int i = 0; i < 10; i++) {
        Ant ant{};
        ant.init(random_float(0.f, 1920.f), random_float(0.f, 1080.f), Species::Spider);
        ants.emplace_back(ant);
    }

    for (const b2Vec2& tree_position : tree_positions) {
        Tree tree{};
        tree.init(tree_position.x, tree_position.y);
        trees.emplace_back(tree);
    }

    Uint64 time = SDL_GetTicks();
    
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            continue;
        }

        Uint64 new_time = SDL_GetTicks();
        double deltatime = (new_time - time) / 1000.f;
        time = new_time;

        deltatime *= 1.f;

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
            b2Vec2 position = b2Body_GetPosition(ant.body_id);
            ant.animation.update(deltatime);

            ant.health -= deltatime;
            if (ant.health <= 0.f) ant.dead = true;
            ant.cooldown_timer += deltatime;
            ant.direction_timer -= deltatime;
            if (ant.direction_timer < 0.f) {
                ant.direction_timer = random_float(3.f, 6.f);                
                ant.intended_direction = b2Normalize(b2RotateVector(b2MakeRot(random_float(-degtorad(30), degtorad(30))), b2Body_GetLinearVelocity(ant.body_id)));
            }
            if (ant.health < 20.f) {
                ant.intended_direction = b2Normalize(get_closest_tree_position(position) - position) * 2.f;
            }
            else if (ant.cooldown_timer > 40.f && ant.health > 60.f) {
                ant.intended_direction = b2Normalize(get_closest_ant_position(position, &ant) - position) * 2.f;
            }

            ant.velocity = exponential_decay(ant.velocity, ant.intended_direction, 1, deltatime);
            b2Body_SetLinearVelocity(ant.body_id, ant.velocity * ant.speed);
            ant.animation.framerate = b2Length(ant.velocity * ant.speed) / 4.5f;

            render_entity(position.x, position.y, std::atan2(ant.velocity.y, ant.velocity.x), 50, 50, ant.animation.get_texture());
        }

        std::vector<b2Vec2> new_ants{};
        auto events = b2World_GetContactEvents(world_id);
        for (int i = 0; i < events.beginCount; i++) {
            auto body_a = b2Shape_GetBody(events.beginEvents[i].shapeIdA);
            auto body_b = b2Shape_GetBody(events.beginEvents[i].shapeIdB);
            std::vector<Ant*> hit_ants{};
            std::vector<Tree*> hit_trees{};
            for (Ant& ant : ants) {
                if (B2_ID_EQUALS(ant.body_id, body_a) || B2_ID_EQUALS(ant.body_id, body_b)) {
                    ant.intended_direction = b2RotateVector(b2MakeRot(random_float(-degtorad(90), degtorad(270))), ant.intended_direction);
                    hit_ants.emplace_back(&ant);
                }
            }
            for (Tree& tree : trees) {
                if (B2_ID_EQUALS(tree.body_id, body_a) || B2_ID_EQUALS(tree.body_id, body_b)) {
                    hit_trees.emplace_back(&tree);
                }
            }
            if (hit_ants.size() == 2) { // Collision was between 2 ants
                if (random_float(0.f, 1.f) > 0.4f && hit_ants[0]->cooldown_timer > 7.f && hit_ants[1]->cooldown_timer > 7.f) {
                    b2Vec2 position = b2Lerp(b2Body_GetPosition(body_a), b2Body_GetPosition(body_b), 0.5);
                    hit_ants[0]->health /= 1.2f;
                    hit_ants[1]->health /= 1.2f;
                    new_ants.emplace_back(position);
                }
                hit_ants[0]->cooldown_timer = 0.f;
                hit_ants[1]->cooldown_timer = 0.f;
            }
            if (hit_trees.size() == 1 && hit_ants.size() == 1) {
                if (hit_trees[0]->food.size() > 0) {
                    hit_trees[0]->food.erase(hit_trees[0]->food.end() - 1, hit_trees[0]->food.end());
                    hit_ants[0]->health += random_float(60.f, 80.f);
                }
                hit_trees[0]->health--;
                if (hit_trees[0]->health <= 0) {
                    hit_trees[0]->dead = true;
                }
            }
        }
        for (const b2Vec2& new_ant : new_ants) {
            ants.emplace_back();
            ants[ants.size() - 1].init(new_ant.x, new_ant.y, Species::Spider);
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

            for (Food& food : trees[i].food) {
                SDL_Texture* food_texture = fruit[food.type];
                SDL_SetTextureColorMod(food_texture, food.r * 255, food.g * 255, food.b * 255);
                render_entity(food.position.x, food.position.y, food.rotation, 15, 15, food_texture);
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

        for (Ant& ant : ants) {
            if (ant.dead) {
                ant.destroy();
            }
        }
        for (Tree& tree : trees) {
            if (tree.dead) {
                tree.destroy();
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
