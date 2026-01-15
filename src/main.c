#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

double start_time = 0.0;
double current_time = 0.0;
double final_time = 0.0;
int game_state = 0;
int total_moves = 0;
int postProcessEffect = 0;

ma_engine audio_engine;

void printHelp() {
    printf("\n");
    printf("=======================================================\n");
    printf("                 RUBIKOVA KOCKA       \n");
    printf("=======================================================\n");

    printf(" [ GLAVNE OPCIJE ]\n");
    printf("   [S]       -> Promesaj kocku (Shuffle)\n");
    printf("   [SPACE]   -> Automatsko resavanje (Auto Solve)\n");
    printf("   [H]       -> Prikazi ovu pomoc\n");
    printf("   [ESC]     -> Izlaz iz programa\n");
    printf("-------------------------------------------------------\n");

    printf(" [ POST-PROCESSING EFEKTI (Shaderi) ]\n");
    printf("   [1]       -> Normalan prikaz (Normal Mapping + PBR)\n");
    printf("   [2]       -> Efekat: Inverzija boja\n");
    printf("   [3]       -> Efekat: Vinjeta (Zatamnjeni uglovi)\n");
    printf("   [4]       -> Efekat: Crno-Belo (Grayscale)\n");
    printf("-------------------------------------------------------\n");

    printf(" [ KONTROLE KOCKE  ]\n");
    printf("   I / K     -> Rotacija po vertikali (Y osa)\n");
    printf("   J / L     -> Rotacija po horizontali (X osa)\n");
    printf("   U / O     -> Rotacija po dubini (Z osa)\n");
    printf("-------------------------------------------------------\n");

}

char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) { printf("GRESKA: Nije moguce otvoriti fajl: %s\n", path); return NULL; }
    fseek(file, 0, SEEK_END); long length = ftell(file); fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length + 1); fread(buffer, 1, length, file);
    buffer[length] = '\0'; fclose(file); return buffer;
}

unsigned int createShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type); glShaderSource(shader, 1, &source, NULL); glCompileShader(shader);
    int success; char infoLog[512]; glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(shader, 512, NULL, infoLog); printf("Shader Error: %s\n", infoLog); }
    return shader;
}

unsigned int createProgram(const char* vPath, const char* fPath) {
    char* vSource = readFile(vPath); char* fSource = readFile(fPath);
    if (!vSource || !fSource) return 0;
    unsigned int vShader = createShader(vSource, GL_VERTEX_SHADER);
    unsigned int fShader = createShader(fSource, GL_FRAGMENT_SHADER);
    unsigned int program = glCreateProgram();
    glAttachShader(program, vShader); glAttachShader(program, fShader); glLinkProgram(program);
    glDeleteShader(vShader); glDeleteShader(fShader); free(vSource); free(fSource);
    return program;
}

unsigned int loadTexture(char const * path) {
    unsigned int textureID; glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1) ? GL_RED : (nrComponents == 3 ? GL_RGB : GL_RGBA);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else { printf("Texture failed: %s\n", path); stbi_image_free(data); }
    return textureID;
}

unsigned int loadCubemap(char** faces) {
    unsigned int textureID; glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

typedef struct { char axis; int layer; float dir; } Move;
Move history[2000]; int history_count = 0;
int animating = 0, solving = 0, shuffling = 0, shuffle_moves = 0;
float anim_angle = 0.0f, anim_dir = 1.0f; char anim_axis = 'y'; int anim_layer = 0;
float animation_speed = 9.0f;
float cube_yaw = 45.0f, cube_pitch = -30.0f, cam_dist = 8.0f;
double last_x, last_y; int first_mouse = 1;

typedef struct { vec3 colors[6]; } Cubie;
Cubie cubies[3][3][3]; mat4 cubie_mats[3][3][3];

void init_cubes() {
    float cols[6][3] = {{0,0.6,0}, {0,0,0.8}, {0.8,0,0}, {1,0.5,0}, {0.9,0.9,0.9}, {0.9,0.9,0}};
    float blk[3] = {0.1,0.1,0.1};
    for(int x=0; x<3; x++) for(int y=0; y<3; y++) for(int z=0; z<3; z++) {
        glm_mat4_identity(cubie_mats[x][y][z]);
        glm_translate(cubie_mats[x][y][z], (vec3){(float)x-1, (float)y-1, (float)z-1});
        for(int f=0; f<6; f++) glm_vec3_copy(blk, cubies[x][y][z].colors[f]);
        if(z==0) glm_vec3_copy(cols[0], cubies[x][y][z].colors[0]);
        if(z==2) glm_vec3_copy(cols[1], cubies[x][y][z].colors[1]);
        if(x==0) glm_vec3_copy(cols[2], cubies[x][y][z].colors[2]);
        if(x==2) glm_vec3_copy(cols[3], cubies[x][y][z].colors[3]);
        if(y==0) glm_vec3_copy(cols[4], cubies[x][y][z].colors[4]);
        if(y==2) glm_vec3_copy(cols[5], cubies[x][y][z].colors[5]);
    }
}

void rotate_layer_fixed(char axis, int layer, float angle) {
    mat4 rot; glm_mat4_identity(rot); vec3 ax = {0};
    if(axis=='x') ax[0]=1; if(axis=='y') ax[1]=1; if(axis=='z') ax[2]=1;
    glm_rotate(rot, angle, ax);
    for(int x=0; x<3; x++) for(int y=0; y<3; y++) for(int z=0; z<3; z++) {
        vec4 pos = {0,0,0,1}; glm_mat4_mulv(cubie_mats[x][y][z], (vec4){0,0,0,1}, pos);
        int in = 0; float eps = 0.1f;
        if(axis=='x' && fabsf(pos[0]-(float)layer)<eps) in=1;
        if(axis=='y' && fabsf(pos[1]-(float)layer)<eps) in=1;
        if(axis=='z' && fabsf(pos[2]-(float)layer)<eps) in=1;
        if(in) { mat4 t; glm_mat4_mul(rot, cubie_mats[x][y][z], t); glm_mat4_copy(t, cubie_mats[x][y][z]); }
    }
}

void trigger(char ax, int l, float d, int rec) {
    animating=1; anim_axis=ax; anim_layer=l; anim_dir=d; anim_angle=0;
    if(rec && history_count<2000) { history[history_count++] = (Move){ax, l, d}; if(game_state == 2) total_moves++; }
    ma_engine_play_sound(&audio_engine, "res/sounds/move.wav", NULL);
}

void key_cb(GLFWwindow* w, int k, int s, int a, int m) {
    if(a==GLFW_PRESS) {
        if(k==GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, 1);
        if(k==GLFW_KEY_H) printHelp();
        if(k==GLFW_KEY_1) postProcessEffect = 0;
        if(k==GLFW_KEY_2) postProcessEffect = 1;
        if(k==GLFW_KEY_3) postProcessEffect = 2;
        if(k==GLFW_KEY_4) postProcessEffect = 3;

        if(!animating) {
            if(k==GLFW_KEY_I) trigger('y', 1, -1, 1); if(k==GLFW_KEY_K) trigger('y', -1, 1, 1);
            if(k==GLFW_KEY_J) trigger('x', -1, 1, 1); if(k==GLFW_KEY_L) trigger('x', 1, -1, 1);
            if(k==GLFW_KEY_U) trigger('z', 1, -1, 1); if(k==GLFW_KEY_O) trigger('z', -1, 1, 1);
            if(k==GLFW_KEY_S && !shuffling && !solving) { shuffling=1; shuffle_moves=20; game_state=1; total_moves=0; }
            if(k==GLFW_KEY_SPACE && history_count>0 && !shuffling) { solving=1; game_state=3; }
        }
    }
}

void mouse_cb(GLFWwindow* w, double x, double y) {
    if(glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS) {
        if(first_mouse) { last_x=x; last_y=y; first_mouse=0; }
        cube_yaw += (x-last_x)*0.5f; cube_pitch += (last_y-y)*0.5f;
        last_x=x; last_y=y;
        if(cube_pitch>89) cube_pitch=89; if(cube_pitch<-89) cube_pitch=-89;
    } else first_mouse=1;
}

const char* skyboxVertSrc = "#version 330 core\nlayout (location=0) in vec3 aPos;\nout vec3 TexCoords;\nuniform mat4 projection;\nuniform mat4 view;\nvoid main(){\nTexCoords=aPos;\ngl_Position=(projection*view*vec4(aPos,1.0)).xyww;\n}\0";
const char* skyboxFragSrc = "#version 330 core\nout vec4 FragColor;\nin vec3 TexCoords;\nuniform samplerCube skybox;\nvoid main(){\nFragColor=texture(skybox,TexCoords);\n}\n\0";

int main() {
    srand(time(NULL)); init_cubes();
    if (ma_engine_init(NULL, &audio_engine) != MA_SUCCESS) return -1;
    glfwInit(); glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rubik's Cube Pro Graphics", NULL, NULL);
    glfwMakeContextCurrent(window); glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_cb); glfwSetKeyCallback(window, key_cb);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    unsigned int cubeProg = createProgram("res/shaders/cube.vert", "res/shaders/cube.frag");
    unsigned int screenProg = createProgram("res/shaders/screen.vert", "res/shaders/screen.frag");

    unsigned int skyProg = glCreateProgram();
    unsigned int sv = createShader(skyboxVertSrc, GL_VERTEX_SHADER);
    unsigned int sf = createShader(skyboxFragSrc, GL_FRAGMENT_SHADER);
    glAttachShader(skyProg, sv); glAttachShader(skyProg, sf); glLinkProgram(skyProg);

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO); glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO); glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float))); glEnableVertexAttribArray(2);

    float quadVerts[] = { -1,1,0,1, -1,-1,0,0, 1,-1,1,0, -1,1,0,1, 1,-1,1,0, 1,1,1,1 };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float))); glEnableVertexAttribArray(1);

    float skyboxVertices[] = { -1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,1,-1,-1,1,-1,-1,-1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,-1,1,1,1,1,1,1,1,1,1,-1,1,-1,-1,-1,-1,1,-1,1,1,1,1,1,1,1,1,1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,1,1,1,1,1,-1,1,1,-1,1,-1,-1,-1,-1,-1,-1,1,1,-1,-1,1,-1,-1,-1,-1,1,1,-1,1 };
    unsigned int skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO); glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO); glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0); glEnableVertexAttribArray(0);

    unsigned int fbo; glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned int texColorBuffer; glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    unsigned int rbo; glGenRenderbuffers(1, &rbo); glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) printf("FBO Error!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int cubeTexture = loadTexture("res/textures/container.jpg");
    unsigned int normalMap = loadTexture("res/textures/normal_map.png");
    char* faces[] = {"res/textures/skybox/right.jpg", "res/textures/skybox/left.jpg", "res/textures/skybox/top.jpg", "res/textures/skybox/bottom.jpg", "res/textures/skybox/front.jpg", "res/textures/skybox/back.jpg"};
    unsigned int cubemapTexture = loadCubemap(faces);

    glUseProgram(cubeProg);
    glUniform1i(glGetUniformLocation(cubeProg, "texture1"), 0);
    glUniform1i(glGetUniformLocation(cubeProg, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(cubeProg, "skybox"), 2);
    glUseProgram(skyProg); glUniform1i(glGetUniformLocation(skyProg, "skybox"), 0);
    glUseProgram(screenProg); glUniform1i(glGetUniformLocation(screenProg, "screenTexture"), 0);

    while (!glfwWindowShouldClose(window)) {
        if(!animating) {
            if(shuffling) { if(shuffle_moves>0) { trigger("xyz"[rand()%3], rand()%3-1, (rand()%2)*2-1, 1); shuffle_moves--; animation_speed=20; } else { shuffling=0; animation_speed=9; game_state=2; start_time=glfwGetTime(); } }
            else if(solving && history_count>0) { Move m = history[--history_count]; trigger(m.axis, m.layer, -m.dir, 0); animation_speed=20; }
            else { solving=0; if(game_state==2 && history_count==0) { game_state=0; final_time = glfwGetTime()-start_time; } }
        }
        if(animating) { anim_angle+=animation_speed; if(anim_angle>=90) { rotate_layer_fixed(anim_axis, anim_layer, glm_rad(90*anim_dir)); animating=0; } }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 view, proj; glm_mat4_identity(view); glm_mat4_identity(proj);
        mat4 camRot; glm_mat4_identity(camRot);
        glm_rotate(camRot, glm_rad(cube_pitch), (vec3){1,0,0}); glm_rotate(camRot, glm_rad(cube_yaw), (vec3){0,1,0});
        vec4 rCamPos; glm_mat4_mulv(camRot, (vec4){0,0,cam_dist,1}, rCamPos);
        glm_lookat((vec3){rCamPos[0],rCamPos[1],rCamPos[2]}, (vec3){0,0,0}, (vec3){0,1,0}, view);
        glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f, proj);

        glUseProgram(cubeProg);
        glUniformMatrix4fv(glGetUniformLocation(cubeProg, "view"), 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(glGetUniformLocation(cubeProg, "projection"), 1, GL_FALSE, (float*)proj);

        float timeVal = (float)glfwGetTime();
        float lightRadius = 15.0f;
        float lightX = sin(timeVal) * lightRadius;
        float lightZ = cos(timeVal) * lightRadius;
        float lightY = 10.0f;

        glUniform3f(glGetUniformLocation(cubeProg, "lightPos"), lightX, lightY, lightZ);        glUniform3f(glGetUniformLocation(cubeProg, "viewPos"), rCamPos[0], rCamPos[1], rCamPos[2]);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glBindVertexArray(cubeVAO);

        for(int x=0; x<3; x++) for(int y=0; y<3; y++) for(int z=0; z<3; z++) {
            mat4 model; glm_mat4_copy(cubie_mats[x][y][z], model);
            if(animating) {
                vec4 p={0,0,0,1}; glm_mat4_mulv(model, p, p);
                int in=0; float eps=0.1f;
                if(anim_axis=='x' && fabsf(p[0]-anim_layer)<eps) in=1;
                if(anim_axis=='y' && fabsf(p[1]-anim_layer)<eps) in=1;
                if(anim_axis=='z' && fabsf(p[2]-anim_layer)<eps) in=1;
                if(in) {
                    mat4 ar; glm_mat4_identity(ar); vec3 ax={0};
                    if(anim_axis=='x') ax[0]=1; if(anim_axis=='y') ax[1]=1; if(anim_axis=='z') ax[2]=1;
                    glm_rotate(ar, glm_rad(anim_angle*anim_dir), ax);
                    mat4 t; glm_mat4_mul(ar, model, t); glm_mat4_copy(t, model);
                }
            }
            glm_scale(model, (vec3){0.95f, 0.95f, 0.95f});
            glUniformMatrix4fv(glGetUniformLocation(cubeProg, "model"), 1, GL_FALSE, (float*)model);
            for(int f=0; f<6; f++) {
                glVertexAttrib3fv(3, cubies[x][y][z].colors[f]);
                glDrawArrays(GL_TRIANGLES, f*6, 6);
            }
        }

        glDepthFunc(GL_LEQUAL); glUseProgram(skyProg);
        mat4 viewNoTrans; glm_mat4_copy(view, viewNoTrans); viewNoTrans[3][0]=0; viewNoTrans[3][1]=0; viewNoTrans[3][2]=0;
        glUniformMatrix4fv(glGetUniformLocation(skyProg, "view"), 1, GL_FALSE, (float*)viewNoTrans);
        glUniformMatrix4fv(glGetUniformLocation(skyProg, "projection"), 1, GL_FALSE, (float*)proj);
        glBindVertexArray(skyVAO); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36); glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1,1,1,1); glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screenProg);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glUniform1i(glGetUniformLocation(screenProg, "effectType"), postProcessEffect);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window); glfwPollEvents();
    }
    ma_engine_uninit(&audio_engine); glfwTerminate(); return 0;
}