/////////////////////////////////////////////////////////////////////////////////////
//
// This code is used to teach the course "Advanced Graphics" in Centennial college
// Developed by Alireza Moghaddam on Feb. 2025 
//
////////////////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <vector>
#include <iostream>

enum GameObject_Type {
    PLAYER,
    ENEMY,
    BULLET,
    OBSTACLE
};

struct GameObject {
    glm::vec3 location;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 moving_direction;
    GLfloat velocity;
    GLfloat collider_dimension;
    int living_time;
    int life_span;
    int type;
    bool isAlive;
    bool isCollided;
};

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer };
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[2];

const GLuint NumVertices = 28;

// Camera and movement variables
float ground_height = 0.8f;
float crouch_height = 0.4f;
float gravity = -15.0f;
float jump_velocity = 6.0f;
float vertical_velocity = 0.0f;
bool is_jumping = false;
bool is_crouching = false;

float travel_speed = 8.0f;
float current_speed = 0.0f;
float max_speed = 8.0f;
float acceleration = 20.0f;
float deceleration = 15.0f;
float mouse_sensitivity = 0.01f;
float player_collider_size = 0.5f;
glm::vec3 movement_direction(0.0f);

int x0 = 0;
int y_0 = 0;

glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, ground_height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);
glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);

int oldTimeSinceStart = 0;
int deltaTime;

const int Num_Obstacles = 20;
float obstacle_data[Num_Obstacles][3];
std::vector<GameObject> sceneGraph;

float randomFloat(float a, float b)
{
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

void init(void)
{
    up_vector = glm::normalize(up_vector);
    forward_vector = glm::normalize(forward_vector);
    looking_dir_vector = glm::normalize(looking_dir_vector);
    side_vector = glm::normalize(side_vector);

    for (int i = 0; i < Num_Obstacles; i++)
    {
        obstacle_data[i][0] = randomFloat(-50, 50);
        obstacle_data[i][1] = randomFloat(-50, 50);
        obstacle_data[i][2] = randomFloat(0.1f, 10.0f);

        GameObject go;
        go.location = glm::vec3(obstacle_data[i][0], obstacle_data[i][1], 0);
        go.rotation = glm::vec3(0, 0, 0);
        go.scale = glm::vec3(obstacle_data[i][2], obstacle_data[i][2], obstacle_data[i][2]);
        go.collider_dimension = go.scale.x;
        go.isAlive = true;
        go.living_time = 0;
        go.isCollided = false;
        go.velocity = 0;
        go.type = OBSTACLE;
        go.moving_direction = glm::vec3(0, 0, 0);
        go.life_span = -1;
        sceneGraph.push_back(go);
    }

    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "triangles.vert" },
        { GL_FRAGMENT_SHADER, "triangles.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders(shaders);
    glUseProgram(program);

    GLfloat vertices[NumVertices][3] = {
        { -100.0, -100.0, 0.0 }, { 100.0, -100.0, 0.0 }, { 100.0, 100.0, 0.0 }, { -100.0, 100.0, 0.0 },
        { -0.5, -0.5 ,0.01 }, { 0.5, -0.5 ,0.01 }, { 0.5, 0.5 ,0.01 }, { -0.5, 0.5 ,0.01 },
        { -0.5, -0.5, 1.01 }, { 0.5, -0.5, 1.01 }, { 0.5, 0.5, 1.01 }, { -0.5, 0.5, 1.01 },
        { 0.5, -0.5 , 0.01 }, { 0.5, 0.5 , 0.01 }, { 0.5, 0.5 ,1.01 }, { 0.5, -0.5 ,1.01 },
        { -0.5, -0.5, 0.01 }, { -0.5, 0.5 , 0.01 }, { -0.5, 0.5 ,1.01 }, { -0.5, -0.5 ,1.01 },
        { -0.5, 0.5 , 0.01 }, { 0.5, 0.5 , 0.01 }, { 0.5, 0.5 ,1.01 }, { -0.5, 0.5 ,1.01 },
        { -0.5, -0.5 , 0.01 }, { 0.5, -0.5 , 0.01 }, { 0.5, -0.5 ,1.01 }, { -0.5, -0.5 ,1.01 },
    };

    GLfloat textureCoordinates[28][2] = {
        0.0f, 0.0f, 200.0f, 0.0f, 200.0f, 200.0f, 0.0f, 200.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    GLint width1, height1;
    unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);
    GLint width2, height2;
    unsigned char* textureData2 = SOIL_load_image("apple.png", &width2, &height2, 0, SOIL_LOAD_RGB);

    glGenBuffers(2, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindAttribLocation(program, 0, "vPosition");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
    glBindAttribLocation(program, 1, "vTexCoord");
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(1);

    location = glGetUniformLocation(program, "model_matrix");
    cam_mat_location = glGetUniformLocation(program, "camera_matrix");
    proj_mat_location = glGetUniformLocation(program, "projection_matrix");

    glGenTextures(2, texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void drawCube(glm::vec3 scale)
{
    model_view = glm::scale(model_view, scale);
    glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glDrawArrays(GL_QUADS, 4, 24);
}

bool isColliding(GameObject one, GameObject two) {
    return glm::abs(one.location.x - two.location.x) <= (one.collider_dimension / 2 + two.collider_dimension / 2) &&
        glm::abs(one.location.y - two.location.y) <= (one.collider_dimension / 2 + two.collider_dimension / 2) &&
        glm::abs(one.location.z - two.location.z) <= (one.collider_dimension / 2 + two.collider_dimension / 2);
}

void checkCollisions() {
    for (int i = 0; i < sceneGraph.size(); i++) {
        for (int j = 0; j < sceneGraph.size(); j++) {
            if (i != j && sceneGraph[i].isAlive && sceneGraph[j].isAlive &&
                !(sceneGraph[i].type == OBSTACLE && sceneGraph[j].type == OBSTACLE) &&
                isColliding(sceneGraph[i], sceneGraph[j])) {
                sceneGraph[i].isCollided = true;
                sceneGraph[j].isCollided = true;
            }
        }
    }
}

void updateSceneGraph() {
    checkCollisions();
    for (int i = 0; i < sceneGraph.size(); i++) {
        GameObject& go = sceneGraph[i];
        if (go.life_span > 0 && go.isAlive && go.living_time >= go.life_span) {
            go.isAlive = false;
        }
        if (go.life_span > 0 && go.isAlive && go.living_time < go.life_span) {
            go.location += ((GLfloat)deltaTime) * go.velocity * glm::normalize(go.moving_direction);
            go.living_time += deltaTime;
        }
    }
}

void updatePhysics()
{
    float dt = (float)deltaTime / 1000.0f;

    if (is_jumping) {
        vertical_velocity += gravity * dt;
        cam_pos.z += vertical_velocity * dt;
        if (cam_pos.z <= (is_crouching ? crouch_height : ground_height)) {
            cam_pos.z = (is_crouching ? crouch_height : ground_height);
            vertical_velocity = 0.0f;
            is_jumping = false;
        }
    }

    float speed_change = (movement_direction == glm::vec3(0.0f)) ? deceleration : acceleration;
    current_speed = glm::clamp(current_speed + (movement_direction == glm::vec3(0.0f) ? -speed_change : speed_change) * dt, 0.0f, max_speed);

    glm::vec3 new_pos = cam_pos + movement_direction * current_speed * dt;
    bool collision = false;
    for (const auto& obj : sceneGraph) {
        if (obj.isAlive && obj.type == OBSTACLE) {
            float distance = glm::length(new_pos - obj.location);
            if (distance < (player_collider_size + obj.collider_dimension / 2)) {
                collision = true;
                break;
            }
        }
    }

    if (!collision) {
        cam_pos = new_pos;
    }
    movement_direction = glm::vec3(0.0f);
}

void draw_level()
{
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glDrawArrays(GL_QUADS, 0, 4);
    updateSceneGraph();

    for (int i = 0; i < sceneGraph.size(); i++) {
        GameObject go = sceneGraph[i];
        if (go.isAlive && !go.isCollided) {
            model_view = glm::translate(model_view, go.location);
            model_view = glm::rotate(model_view, 0.0f, unit_z_vector);
            glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

            if (go.type == OBSTACLE || go.type == BULLET) {
                drawCube(go.scale);
            }

            model_view = glm::mat4(1.0);
            glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
        }
    }
}

void display(void)
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    model_view = glm::mat4(1.0);
    glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

    glm::vec3 look_at = cam_pos + looking_dir_vector;
    glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
    glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

    glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

    draw_level();
    glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
    float dt = (float)deltaTime / 1000.0f;

    if (key == 'a') movement_direction -= side_vector;
    if (key == 'd') movement_direction += side_vector;
    if (key == 'w') movement_direction += forward_vector;
    if (key == 's') movement_direction -= forward_vector;
    if (key == 'f') {
        GameObject go;
        go.location = cam_pos;
        go.rotation = glm::vec3(0, 0, 0);
        go.scale = glm::vec3(0.07, 0.07, 0.07);
        go.collider_dimension = go.scale.x;
        go.isAlive = true;
        go.living_time = 0;
        go.isCollided = false;
        go.velocity = 0.01;
        go.type = BULLET;
        go.moving_direction = looking_dir_vector;
        go.life_span = 4000;
        sceneGraph.push_back(go);
    }
    if (key == ' ' && !is_jumping) {
        is_jumping = true;
        vertical_velocity = jump_velocity;
        if (is_crouching) {
            vertical_velocity *= 1.2f;
            is_crouching = false;
        }
    }
    if (key == 'c' && !is_jumping) {
        is_crouching = !is_crouching;
        cam_pos.z = is_crouching ? crouch_height : ground_height;
        max_speed = is_crouching ? 4.0f : 8.0f;
    }

    if (movement_direction != glm::vec3(0.0f)) {
        movement_direction = glm::normalize(movement_direction);
    }
}

void mouse(int x, int y)
{
    int delta_x = x - x0;
    forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
    looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
    side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
    up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
    x0 = x;

    int delta_y = y - y_0;
    glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
    glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
    GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

    if (dot_product > 0) {
        up_vector = tmp_up_vec;
        looking_dir_vector = tmp_looking_dir;
    }
    y_0 = y;
}

void idle()
{
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = timeSinceStart - oldTimeSinceStart;
    oldTimeSinceStart = timeSinceStart;
    updatePhysics();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("Camera and Projection");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(mouse);

    glutMainLoop();
    return 0;
}