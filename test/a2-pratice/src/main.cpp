#include "cgmath.h"    // slee's simple math library
#include "cgut.h"      // slee's OpenGL utility
#include "sphere.h"    // 구 메시 생성 함수

//*************************************
// global constants
static const char* window_name = "cgbase - sphere";
static const char* vert_shader_path = "shaders/transform.vert";
static const char* frag_shader_path = "shaders/transform.frag";

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2 window_size = cg_default_window_size();

//*************************************
// OpenGL objects
GLuint program = 0;

//*************************************
// global variables
int frame = 0;

// scene object
mesh* p_mesh = nullptr;

// 추가: 회전 및 렌더링 모드 관련 전역 변수
bool rotating = true;
float sphere_angle = 0.0f;
int color_mode = 0;   // 0: (tc.xy, 0, 1), 1: (tc.xxx, 1), 2: (tc.yyy, 1)
bool wireframe = false;

//*************************************
// update(): view_projection 매트릭스와 구의 회전 업데이트
void update()
{
    // aspect correction 및 CVV 변환
    float aspect = window_size.x / float(window_size.y);
    // 과제 지시서의 방법: x 축과 y 축의 스케일을 각각 std::min(1/aspect,1)와 std::min(aspect,1)로 설정
    float scale_x = std::min(1.0f / aspect, 1.0f);
    float scale_y = std::min(aspect, 1.0f);
    mat4 aspect_matrix = mat4::scale(scale_x, scale_y, 1.0f);
    // 변환 매트릭스 (RHS → LHS, 과제 Eq.(4) 참고)
    mat4 cvt_matrix = mat4{
         0,  1, 0, 0,
         0,  0, 1, 0,
        -1,  0, 0, 1,
         0,  0, 0, 1
    };
    mat4 view_projection_matrix = aspect_matrix * cvt_matrix;
    GLint uloc = glGetUniformLocation(program, "view_projection_matrix");
    if (uloc > -1)
        glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);

    // 구 회전 업데이트: dt만큼 누적, 'R'키로 토글됨
    double t = glfwGetTime();
    static double last_t = t;
    float dt = float(t - last_t);
    last_t = t;
    if (rotating)
        sphere_angle += dt; // 필요에 따라 배속 곱셈 가능
}

//*************************************
// render(): 구 모델 행렬 및 모드 uniform 업데이트 후 드로우 호출
void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);

    if (p_mesh && p_mesh->vertex_array)
        glBindVertexArray(p_mesh->vertex_array);

    // 구 모델 행렬: 회전 (z축 기준) 적용
    mat4 model_matrix = mat4::rotate(vec3(0, 0, 1), sphere_angle);
    GLint uloc = glGetUniformLocation(program, "model_matrix");
    if (uloc > -1)
        glUniformMatrix4fv(uloc, 1, GL_TRUE, model_matrix);

    // 색상 모드 uniform 설정
    uloc = glGetUniformLocation(program, "mode");
    if (uloc > -1)
        glUniform1i(uloc, color_mode);

    // 인덱스 버퍼를 사용하여 구 드로우
    glDrawElements(GL_TRIANGLES, GLsizei(p_mesh->index_list.size()), GL_UNSIGNED_INT, nullptr);

    glfwSwapBuffers(window);
}

void reshape(GLFWwindow* window, int width, int height)
{
    window_size = ivec2(width, height);
    glViewport(0, 0, width, height);
}

void print_help()
{
    printf("[help]\n");
    printf("- press ESC or 'q' to terminate the program\n");
    printf("- press F1 or 'h' to see help\n");
    printf("- press 'D' to toggle through texture coordinate modes\n");
    printf("- press 'R' to stop/resume rotation\n");
    printf("- press 'W' to toggle wireframe mode\n");
    printf("\n");
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
            glfwSetWindowShouldClose(window, GL_TRUE);
        else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)
            print_help();
        else if (key == GLFW_KEY_D)
        {
            // 색상 모드 순환: 0 -> 1 -> 2 -> 0
            color_mode = (color_mode + 1) % 3;
            printf("> color_mode = %d\n", color_mode);
        }
        else if (key == GLFW_KEY_R)
        {
            rotating = !rotating;
            printf("> rotation %s\n", rotating ? "resumed" : "stopped");
        }
        else if (key == GLFW_KEY_W)
        {
            wireframe = !wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            printf("> wireframe mode %s\n", wireframe ? "ON" : "OFF");
        }
    }
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        dvec2 pos;
        glfwGetCursorPos(window, &pos.x, &pos.y);
        printf("> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y));
    }
}

void motion(GLFWwindow* window, double x, double y)
{
    // 필요 시 구현
}

bool user_init()
{
    print_help();
    glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // 구 메시 생성 (과제 지시: longitude 72, latitude 36)
    p_mesh = create_sphere_mesh(72, 36);
    if (p_mesh == nullptr)
    {
        printf("Unable to create sphere mesh\n");
        return false;
    }
    return true;
}

void user_finalize()
{
    // p_mesh의 소멸자에서 GPU 버퍼 삭제 처리됨.
}

int main(int argc, char* argv[])
{
    if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) {
        glfwTerminate();
        return 1;
    }
    if (!cg_init_extensions(window)) {
        glfwTerminate();
        return 1;
    }

    if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) {
        glfwTerminate();
        return 1;
    }
    if (!user_init()) {
        printf("Failed to user_init()\n");
        glfwTerminate();
        return 1;
    }

    glfwSetWindowSizeCallback(window, reshape);
    glfwSetKeyCallback(window, keyboard);
    glfwSetMouseButtonCallback(window, mouse);
    glfwSetCursorPosCallback(window, motion);

    for (frame = 0; !glfwWindowShouldClose(window); frame++) {
        glfwPollEvents();
        update();
        render();
    }

    user_finalize();
    cg_destroy_window(window);
    return 0;
}
