#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Shaders
const char* vertexShaderPath = "res/shaders/vert.glsl";
const char* fragmentShaderPath = "res/shaders/frag.glsl";

// --- STRUKTURE I GLOBALI ZA LOGIKU KOCKE ---

typedef struct {
    char axis;      // 'x', 'y', 'z'
    int layer;      // -1, 0, 1
    float dir;      // 1.0f ili -1.0f
} Move;

#define MAX_HISTORY 2000
Move history[MAX_HISTORY];
int history_count = 0;

// Stanja igre
int animating = 0;
int solving = 0;
int shuffling = 0;
int shuffle_moves_remaining = 0;

// Parametri trenutne animacije
float animation_angle = 0.0f;
char animation_axis = 'y';
int animation_layer = 0;
float animation_dir = 1.0f;
float animation_speed = 9.0f; // Brzina rotacije

// --- HELPER FUNKCIJE ---

char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

unsigned int createShader(const char* path, GLenum type) {
    char* source = readFile(path);
    if (!source) return 0;
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char**)&source, NULL);
    glCompileShader(shader);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error (%s): %s\n", path, infoLog);
    }
    free(source);
    return shader;
}

unsigned int createProgram(const char* vPath, const char* fPath) {
    unsigned int vShader = createShader(vPath, GL_VERTEX_SHADER);
    unsigned int fShader = createShader(fPath, GL_FRAGMENT_SHADER);
    unsigned int program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Program linking error: %s\n", infoLog);
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return program;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

// Kamera
float cube_yaw = 45.0f;
float cube_pitch = -30.0f;
double last_x, last_y;
int first_mouse = 1;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (first_mouse) {
            last_x = xpos;
            last_y = ypos;
            first_mouse = 0;
        }

        float xoffset = xpos - last_x;
        float yoffset = last_y - ypos;
        last_x = xpos;
        last_y = ypos;

        float sensitivity = 0.5f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        cube_yaw += xoffset;
        cube_pitch += yoffset;

        if (cube_pitch > 89.0f) cube_pitch = 89.0f;
        if (cube_pitch < -89.0f) cube_pitch = -89.0f;
    } else {
        first_mouse = 1;
    }
}

typedef struct {
    vec3 face_colors[6];
} Cubie;

Cubie cubies[3][3][3];
mat4 cubie_matrices[3][3][3];

void init_cubes() {
    float default_colors[] = {
        0.0f, 1.0f, 0.0f, // Nazad - Zelena
        0.0f, 0.0f, 1.0f, // Napred - Plava
        1.0f, 0.0f, 0.0f, // Levo - Crvena
        1.0f, 0.5f, 0.0f, // Desno - Narandzasta
        1.0f, 1.0f, 1.0f, // Dole - Bela
        1.0f, 1.0f, 0.0f  // Gore - Zuta
    };

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                glm_mat4_identity(cubie_matrices[x][y][z]);
                glm_translate(cubie_matrices[x][y][z], (vec3){(float)x - 1, (float)y - 1, (float)z - 1});

                for (int f = 0; f < 6; f++) {
                    glm_vec3_copy((vec3){0.1f, 0.1f, 0.1f}, cubies[x][y][z].face_colors[f]);
                }

                if (z == 0) glm_vec3_copy((vec3){default_colors[0], default_colors[1], default_colors[2]}, cubies[x][y][z].face_colors[0]);
                if (z == 2) glm_vec3_copy((vec3){default_colors[3], default_colors[4], default_colors[5]}, cubies[x][y][z].face_colors[1]);
                if (x == 0) glm_vec3_copy((vec3){default_colors[6], default_colors[7], default_colors[8]}, cubies[x][y][z].face_colors[2]);
                if (x == 2) glm_vec3_copy((vec3){default_colors[9], default_colors[10], default_colors[11]}, cubies[x][y][z].face_colors[3]);
                if (y == 0) glm_vec3_copy((vec3){default_colors[12], default_colors[13], default_colors[14]}, cubies[x][y][z].face_colors[4]);
                if (y == 2) glm_vec3_copy((vec3){default_colors[15], default_colors[16], default_colors[17]}, cubies[x][y][z].face_colors[5]);
            }
        }
    }
}

// Trajna rotacija matrica nakon završetka animacije
void rotate_layer_fixed(char axis, int layer, float angle) {
    mat4 rot;
    glm_mat4_identity(rot);
    vec3 axis_vec = {0};
    if (axis == 'x') axis_vec[0] = 1.0f;
    if (axis == 'y') axis_vec[1] = 1.0f;
    if (axis == 'z') axis_vec[2] = 1.0f;
    glm_rotate(rot, angle, axis_vec);

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                vec4 pos = {0, 0, 0, 1};
                glm_mat4_mulv(cubie_matrices[x][y][z], (vec4){0, 0, 0, 1}, pos);

                int in_layer = 0;
                float epsilon = 0.1f;
                if (axis == 'x' && fabsf(pos[0] - (float)layer) < epsilon) in_layer = 1;
                if (axis == 'y' && fabsf(pos[1] - (float)layer) < epsilon) in_layer = 1;
                if (axis == 'z' && fabsf(pos[2] - (float)layer) < epsilon) in_layer = 1;

                if (in_layer) {
                    mat4 temp;
                    glm_mat4_mul(rot, cubie_matrices[x][y][z], temp);
                    glm_mat4_copy(temp, cubie_matrices[x][y][z]);
                }
            }
        }
    }
}

// Funkcija za pokretanje animacije
void trigger_move(char axis, int layer, float dir, int record) {
    animating = 1;
    animation_axis = axis;
    animation_layer = layer;
    animation_dir = dir;
    animation_angle = 0.0f;

    if (record && history_count < MAX_HISTORY) {
        history[history_count].axis = axis;
        history[history_count].layer = layer;
        history[history_count].dir = dir;
        history_count++;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    // Ako se nešto dešava, ignoriši input (osim ako nije prekid)
    if (animating) return;

    // Ručne kontrole (snimamo potez -> 1)
    if (key == GLFW_KEY_U) trigger_move('y', 1, -1.0f, 1);
    if (key == GLFW_KEY_D) trigger_move('y', -1, 1.0f, 1);
    if (key == GLFW_KEY_L) trigger_move('x', -1, 1.0f, 1);
    if (key == GLFW_KEY_R) trigger_move('x', 1, -1.0f, 1);
    if (key == GLFW_KEY_F) trigger_move('z', 1, -1.0f, 1);
    if (key == GLFW_KEY_B) trigger_move('z', -1, 1.0f, 1);

    // Shuffle (Mešanje) - Pokreće mod mešanja
    if (key == GLFW_KEY_S && !solving && !shuffling) {
        shuffling = 1;
        shuffle_moves_remaining = 20; // Broj nasumičnih poteza
    }

    // Solve (Rešavanje) - Pokreće mod rešavanja
    if (key == GLFW_KEY_SPACE && !solving && !shuffling && history_count > 0) {
        solving = 1;
    }
}

int main() {
    srand(time(NULL)); // Inicijalizacija random generatora
    init_cubes();
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Rubik's Cube Solver", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = createProgram(vertexShaderPath, fragmentShaderPath);

    // Definicija kocke (geometrija)
    float vertices[] = {
        // Pozicija          // Boja (placeholder - ne koristi se jer šaljemo ručno)
        -0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,

        -0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,

        -0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,

         0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,

        -0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f, -0.48f, -0.48f,  0.0f, 0.0f, 0.0f,

        -0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
         0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f,  0.48f,  0.48f,  0.0f, 0.0f, 0.0f,
        -0.48f,  0.48f, -0.48f,  0.0f, 0.0f, 0.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Pozicija (Attribute 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Boja (Attribute 1) - UKLONJENO da bi radilo ručno bojenje u petlji
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // --- LOGIKA AUTOMATIZACIJE (Shuffle & Solve) ---
        if (!animating) {
            if (shuffling) {
                if (shuffle_moves_remaining > 0) {
                    char axes[] = {'x', 'y', 'z'};
                    int layers[] = {-1, 0, 1};
                    float dirs[] = {1.0f, -1.0f};

                    // Generiši random potez i snimi ga (record = 1)
                    trigger_move(axes[rand() % 3], layers[rand() % 3], dirs[rand() % 2], 1);
                    shuffle_moves_remaining--;
                    animation_speed = 15.0f; // Brže mešanje
                } else {
                    shuffling = 0;
                    animation_speed = 9.0f; // Vrati normalnu brzinu
                }
            }
            else if (solving) {
                if (history_count > 0) {
                    // Uzmi poslednji potez
                    Move last = history[history_count - 1];
                    history_count--; // Ukloni iz istorije

                    // Izvrši suprotan potez (ne snimaj ga ponovo u istoriju -> record = 0)
                    trigger_move(last.axis, last.layer, -last.dir, 0);
                    animation_speed = 15.0f; // Brže rešavanje
                } else {
                    solving = 0;
                    animation_speed = 9.0f;
                    printf("Cube Solved!\n");
                }
            }
        }

        // --- LOGIKA ANIMACIJE ---
        if (animating) {
            animation_angle += animation_speed;
            if (animation_angle >= 90.0f) {
                // Završi rotaciju: primeni trajnu promenu na matrice
                rotate_layer_fixed(animation_axis, animation_layer, glm_rad(90.0f * animation_dir));
                animating = 0;
                animation_angle = 0.0f;
            }
        }

        // --- RENDERING ---
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        mat4 view, projection;
        glm_mat4_identity(view);
        glm_mat4_identity(projection);

        glm_translate(view, (vec3){0.0f, 0.0f, -8.0f});
        glm_rotate(view, glm_rad(cube_pitch), (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(view, glm_rad(cube_yaw), (vec3){0.0f, 1.0f, 0.0f});

        glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)projection);

        glBindVertexArray(VAO);

        // Iscrtavanje svih 27 kockica
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                for (int z = 0; z < 3; z++) {
                    mat4 model;
                    glm_mat4_copy(cubie_matrices[x][y][z], model);

                    // Ako je animacija u toku, primeni privremenu rotaciju na odgovarajući sloj
                    if (animating) {
                        // Moramo proveriti da li trenutna kockica pripada sloju koji se vrti.
                        // Koristimo trenutnu poziciju iz matrice modela.
                        vec4 pos = {0, 0, 0, 1};
                        glm_mat4_mulv(model, (vec4){0, 0, 0, 1}, pos);

                        int in_layer = 0;
                        float epsilon = 0.1f;

                        // Provera da li je centar kocke blizu sloja koji rotiramo (-1, 0, 1)
                        if (animation_axis == 'x' && fabsf(pos[0] - (float)animation_layer) < epsilon) in_layer = 1;
                        if (animation_axis == 'y' && fabsf(pos[1] - (float)animation_layer) < epsilon) in_layer = 1;
                        if (animation_axis == 'z' && fabsf(pos[2] - (float)animation_layer) < epsilon) in_layer = 1;

                        if (in_layer) {
                            mat4 anim_rot;
                            glm_mat4_identity(anim_rot);
                            vec3 ax = {0};
                            if (animation_axis == 'x') ax[0] = 1.0f;
                            if (animation_axis == 'y') ax[1] = 1.0f;
                            if (animation_axis == 'z') ax[2] = 1.0f;

                            // Rotiramo oko centra sveta (0,0,0) jer su kocke već tamo pozicionirane
                            glm_rotate(anim_rot, glm_rad(animation_angle * animation_dir), ax);

                            mat4 temp;
                            glm_mat4_mul(anim_rot, model, temp);
                            glm_mat4_copy(temp, model);
                        }
                    }

                    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);

                    for (int f = 0; f < 6; f++) {
                        // Ovde ručno šaljemo boju za svako lice
                        glVertexAttrib3fv(1, cubies[x][y][z].face_colors[f]);
                        glDrawArrays(GL_TRIANGLES, f * 6, 6);
                    }
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}