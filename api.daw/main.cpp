#include<iostream>

#include"imgui/imgui.h"
#include"imgui/imgui_impl_glfw.h"
#include"imgui/imgui_impl_opengl3.h"
#include"implot/implot.h"
#include"AL/al.h"
#include"AL/alc.h"

#include<glad/glad.h>
#include<GLFW/glfw3.h>

/*api.daw*/
#include <corecrt_math_defines.h>

#define APPLICATION_NAME "DAW Application"

using namespace std;

#define APPLICATION_WINDOW_HEIGHT 790
#define APPLICATION_WINDOW_WIDTH 790

bool OpenApplicationWindow(GLFWwindow* window)
{
    if (window != NULL)
    {

        glfwMakeContextCurrent(window);
        return true;
    }
    else {
        printf("The application window generated an error.");
        glfwTerminate();
        return false;
    }
}

bool SetApplicationWindowColor()
{
    glClearColor(0.09f, 0.14f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    return true;
}

bool ConfigureApplicationWindow()
{
    if (gladLoadGL() == 1)
    {

        glViewport(0, 0, APPLICATION_WINDOW_WIDTH, APPLICATION_WINDOW_HEIGHT);
        SetApplicationWindowColor();
        return true;
    }
    else {
        return false;
    }
}

GLuint ApplicationWindowGLProgram;
#define APPLICATION_WINDOW_GL_PROGRAM ApplicationWindowGLProgram

typedef struct ShaderInfo {
    const char* vertexShader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform float VertexScale = 1.0f;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x * VertexScale, aPos.y * VertexScale, aPos.z * VertexScale, 1.0f);"
        "}\n\0";
    const char* fragmentShader =
        "#version 330 core\n"
        "out vec4 FragmentColor;\n"
        "uniform vec4 DynamicColor;\n"
        "void main()\n"
        "{\n"
        "   FragmentColor = DynamicColor;"
        "}\n\0";
};

bool ConfigureGLShaders(ShaderInfo shaders = ShaderInfo())
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaders.vertexShader, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &shaders.fragmentShader, NULL);
    glCompileShader(fragmentShader);

    APPLICATION_WINDOW_GL_PROGRAM = glCreateProgram();
    glAttachShader(APPLICATION_WINDOW_GL_PROGRAM, vertexShader);
    glAttachShader(APPLICATION_WINDOW_GL_PROGRAM, fragmentShader);
    glLinkProgram(APPLICATION_WINDOW_GL_PROGRAM);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    return true;
}

GLuint ApplicationWindowVAO;
GLuint ApplicationWindowVBO;
#define APPLICATION_WINDOW_VERTEX_ARRAY_OBJECT ApplicationWindowVAO
#define APPLICATION_WINDOW_VERTEX_BUFFER_OBJECT ApplicationWindowVBO

bool ConfigureApplicationWindowBuffer(GLfloat* vertices = {}, int arrayLength = 0, int numberOfObjects = 1)
{
    if (vertices == NULL)
    {

        vertices = new GLfloat[9]{
            -0.5f, -0.5f * float(sqrt(3)) / 4, 0.0f,
            0.5f, -0.5f * float(sqrt(3)) / 4, 
            0.0f, 0.0f, 
            0.5f * float(sqrt(4)) * 3 / 4, 0.0f };

        arrayLength = 9;
    }

    glGenVertexArrays(numberOfObjects, &APPLICATION_WINDOW_VERTEX_ARRAY_OBJECT);
    glGenBuffers(numberOfObjects, &APPLICATION_WINDOW_VERTEX_BUFFER_OBJECT);

    glBindVertexArray(APPLICATION_WINDOW_VERTEX_ARRAY_OBJECT);
    glBindBuffer(GL_ARRAY_BUFFER, APPLICATION_WINDOW_VERTEX_BUFFER_OBJECT);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * arrayLength, vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*) 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool DrawApplicationWindowBackground()
{
    glUseProgram(APPLICATION_WINDOW_GL_PROGRAM);
    glBindVertexArray(APPLICATION_WINDOW_VERTEX_ARRAY_OBJECT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
}

#define APPLICATION_GLOBAL_SAMPLERATE 100

bool DrawPluginOscilloscope(const char* label, float* yPoints, float* xPoints)
{
    ImPlot::CreateContext();
    if (ImPlot::BeginPlot("Oscilloscope"))
    {

        ImPlot::PlotLine(label, xPoints, yPoints, APPLICATION_GLOBAL_SAMPLERATE);
        ImPlot::EndPlot();
    }

    return true;
}

float VertexScale = 1.0f;
float DynamicColor[4] = { 0.9f, 0.3f, 0.02f, 1.0f };
#define APPLICATION_WINDOW_BACKGROUND_SCALE VertexScale
#define APPLICATION_WINDOW_BACKGROUND_COLOR DynamicColor

bool SetPluginOptions()
{
    glUniform1f(glGetUniformLocation(APPLICATION_WINDOW_GL_PROGRAM, "VertexScale"), APPLICATION_WINDOW_BACKGROUND_SCALE);
    glUniform4f(
        glGetUniformLocation(APPLICATION_WINDOW_GL_PROGRAM, "DynamicColor"), 
        APPLICATION_WINDOW_BACKGROUND_COLOR[0], APPLICATION_WINDOW_BACKGROUND_COLOR[1], APPLICATION_WINDOW_BACKGROUND_COLOR[2], APPLICATION_WINDOW_BACKGROUND_COLOR[3]
    );
    return true;
}

bool ApplicationShouldDrawBackground = false;
#define APPLICATION_SHOULD_DRAW_BACKGROUND ApplicationShouldDrawBackground
bool PluginShouldDrawBackground = false;
#define PLUGIN_SHOULD_DRAW_BACKGROUND PluginShouldDrawBackground

bool ConfigureApplicationWindowFrame()
{
    SetApplicationWindowColor();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("This is a plugin window.");
    ImGui::Text("Welcome to a runtime!");
    ImGui::Checkbox("Oscilloscope.", &PLUGIN_SHOULD_DRAW_BACKGROUND);
    ImGui::SliderFloat("OscScale", &VertexScale, 0.1f, 2.0f);
    ImGui::ColorEdit4("OscColor", DynamicColor);
    
    float amplitudes[APPLICATION_GLOBAL_SAMPLERATE];
    for (int n = 0; n < APPLICATION_GLOBAL_SAMPLERATE; n++)
        amplitudes[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
    
    float samples[APPLICATION_GLOBAL_SAMPLERATE];
    for (int n = 0; n < APPLICATION_GLOBAL_SAMPLERATE; n++)
        samples[n] = n;
    
    const char* oscilloscopeLabel = "Realtime";
    ImGui::PlotLines(oscilloscopeLabel, amplitudes, 100);
    
    if (PLUGIN_SHOULD_DRAW_BACKGROUND)
    {
        DrawPluginOscilloscope(oscilloscopeLabel, amplitudes, samples);
    }

    ImGui::End();
    glUseProgram(APPLICATION_WINDOW_GL_PROGRAM);
    SetPluginOptions();

    if (APPLICATION_SHOULD_DRAW_BACKGROUND)
    {
        DrawApplicationWindowBackground();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return true;
}

bool ReframeApplicationWindow(GLFWwindow* window)
{
    glfwSwapBuffers(window);
    return true;
}

bool ExitGLFW(GLFWwindow* window, int numberOfObjects = 1)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(numberOfObjects, &APPLICATION_WINDOW_VERTEX_ARRAY_OBJECT);
    glDeleteBuffers(numberOfObjects, &APPLICATION_WINDOW_VERTEX_BUFFER_OBJECT);
    glDeleteProgram(APPLICATION_WINDOW_GL_PROGRAM);
    glfwDestroyWindow(window);
    glfwTerminate();

    return true;
}

#define SAMPLE_RATE 44100
#define BUFFER_SECONDS 1
#define WAVE_FREQUENCY 158
#define CHANNEL_COUNT 1
#define BUFFER_LENGTH (SAMPLE_RATE * BUFFER_SECONDS)

int main()
{
    ALCdevice* device;
    ALCcontext* context;

    ALshort monoSamples[BUFFER_LENGTH];
    ALshort stereoSamples[BUFFER_LENGTH * 2];
    ALuint buffer[4], source;

    device = alcOpenDevice(NULL);
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    
    alGenSources(1, &source);
    alGenBuffers(4, buffer);

    if (CHANNEL_COUNT == 1)
    {
        for (int i = 0; i < BUFFER_LENGTH; ++i) {
            monoSamples[i * CHANNEL_COUNT] = sin(2 * M_PI * WAVE_FREQUENCY * i / BUFFER_LENGTH) * SHRT_MAX;
        }

        for (int b = 0; b < 4; b++)
        {
            alBufferData(buffer[b], AL_FORMAT_MONO16, monoSamples, sizeof(monoSamples), BUFFER_LENGTH * CHANNEL_COUNT);
            alSourceQueueBuffers(source, 1, &buffer[b]);
        }
    }

    if (CHANNEL_COUNT == 2)
    {
        for (int i = 0; i < BUFFER_LENGTH; ++i) {
            stereoSamples[i * CHANNEL_COUNT] = sin(2 * M_PI * WAVE_FREQUENCY * i / BUFFER_LENGTH / CHANNEL_COUNT) * SHRT_MAX;
            stereoSamples[i * CHANNEL_COUNT + 1] = -1 * sin(2 * M_PI * WAVE_FREQUENCY * i / BUFFER_LENGTH / CHANNEL_COUNT) * SHRT_MAX;
        }

        for (int b = 0; b < 4; b++)
        {
            alBufferData(buffer[b], AL_FORMAT_STEREO16, stereoSamples, sizeof(stereoSamples), BUFFER_LENGTH * CHANNEL_COUNT);
            alSourceQueueBuffers(source, 1, &buffer[b]);
        }
    }

    //alSourcei(source, AL_BUFFER, buffer);
    //alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcePlay(source);

    ALint iBuffersProcessed = 0;
    ALint iTotalBuffersProcessed = 0;

    printf("End of OpenAL configuration");

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLfloat* vertices = new GLfloat[]{-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, 0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, 0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f};

    GLFWwindow* window = glfwCreateWindow(APPLICATION_WINDOW_WIDTH, APPLICATION_WINDOW_HEIGHT, APPLICATION_NAME, NULL, NULL);

    if (OpenApplicationWindow(window))
    {
        if (ConfigureApplicationWindow())
        {
            ConfigureGLShaders();
            ConfigureApplicationWindowBuffer();
            ReframeApplicationWindow(window);

            /* dearimgui */
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 330");

            glUseProgram(APPLICATION_WINDOW_GL_PROGRAM);
            SetPluginOptions();

            while (!glfwWindowShouldClose(window))
            {
                ConfigureApplicationWindowFrame();
                ReframeApplicationWindow(window);
                glfwPollEvents();

                ALuint uiBufferRemoved = 0;
                alSourceUnqueueBuffers(source, 1, &uiBufferRemoved);

                // Read more samples if there is a buffer found from the removal

                alBufferData(uiBufferRemoved, AL_FORMAT_MONO16, monoSamples, sizeof(monoSamples), BUFFER_LENGTH* CHANNEL_COUNT);

                // Send it back to the source queue for looping?

                alBufferData(buffer[3], AL_FORMAT_MONO16, monoSamples, sizeof(monoSamples), BUFFER_LENGTH* CHANNEL_COUNT);
                alSourceQueueBuffers(source, 1, &buffer[3]);

                iBuffersProcessed--;
            }
        }

        alSourceStop(source);
        alDeleteSources(1, &source);
        alDeleteBuffers(1, buffer);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
        ExitGLFW(window);

        return 0;
    }
    else {
        return -1;
    }
}