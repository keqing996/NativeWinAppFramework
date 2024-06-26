#include <iostream>
#include <array>
#include <format>
#include "NativeWinApp/Window.h"
#include "NativeWinApp/Glad/Gl.h"

const char* vertexShaderSource ="#version 330 core\n"
                                "layout (location = 0) in vec3 aPos;\n"
                                "layout (location = 1) in vec3 aColor;\n"
                                "out vec3 ourColor;\n"
                                "void main()\n"
                                "{\n"
                                "   gl_Position = vec4(aPos, 1.0);\n"
                                "   ourColor = aColor;\n"
                                "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 ourColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(ourColor, 1.0f);\n"
                                   "}\n\0";

const std::array<float, 18> vertices = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
};

void DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

static char* infoLog = new char[1024];

int main()
{
    NWA::Window window(800, 600, "TestOpenGL");
    window.CreateOpenGLContext();

    ::gladLoaderLoadGL();

    ::glViewport(0, 0, 800, 600);
    ::glEnable(GL_DEBUG_OUTPUT);
    ::glDebugMessageCallback(&DebugMessageCallback, nullptr);

    unsigned int vertexShader = ::glCreateShader(GL_VERTEX_SHADER);
    ::glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    ::glCompileShader(vertexShader);
    {
        int success;
        ::glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            ::glGetShaderInfoLog(vertexShader, 1024, nullptr, infoLog);
            std::cout << "Vertex Shader Compile Error : " << infoLog << std::endl;
        }
    }

    unsigned int fragmentShader = ::glCreateShader(GL_FRAGMENT_SHADER);
    ::glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    ::glCompileShader(fragmentShader);
    {
        int success;
        ::glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            ::glGetShaderInfoLog(fragmentShader, 1024, nullptr, infoLog);
            std::cout << "Fragment Shader Compile Error : " << infoLog << std::endl;
        }
    }

    unsigned int shaderProgram = ::glCreateProgram();
    ::glAttachShader(shaderProgram, vertexShader);
    ::glAttachShader(shaderProgram, fragmentShader);
    ::glLinkProgram(shaderProgram);
    {
        int success;
        ::glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            ::glGetProgramInfoLog(shaderProgram, 1024, nullptr, infoLog);
            std::cout << "Program Linking Error : " << infoLog << std::endl;
        }
    }

    unsigned int vertexArray;
    ::glGenVertexArrays(1, &vertexArray);
    ::glBindVertexArray(vertexArray);

    unsigned int vertexBuffer;
    ::glGenBuffers(1, &vertexBuffer);
    ::glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    ::glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    ::glEnableVertexAttribArray(0);
    ::glEnableVertexAttribArray(1);
    ::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    ::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;

        ::glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT);

        ::glBindVertexArray(vertexArray);
        ::glUseProgram(shaderProgram);
        ::glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        ::glDrawArrays(GL_TRIANGLES, 0, 3);

        window.SwapBuffer();
    }

    ::glDeleteShader(vertexShader);
    ::glDeleteShader(fragmentShader);
    ::glDeleteVertexArrays(1, &vertexArray);
    ::glDeleteBuffers(1, &vertexBuffer);
    ::glDeleteProgram(shaderProgram);

    return 0;
}

void DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::string messageSource;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        messageSource = "OpenGL";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        messageSource = "Windows";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        messageSource = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        messageSource = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        messageSource = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
    default:
        messageSource = "Other";
        break;
    }

    std::string messageType;
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        messageType = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        messageType = "Deprecated Behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        messageType = "Undefined Behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        messageType = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        messageType = "Performance";
        break;
    case GL_DEBUG_TYPE_OTHER:
    default:
        messageType = "Other";
        break;
    }

    std::string messageSeverity;
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        messageSeverity = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        messageSeverity = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        messageSeverity = "Low";
        break;
    default:
        messageSeverity = "Other";
        break;
    }

    if (type == GL_DEBUG_TYPE_ERROR)
        std::cout << std::format("[GLErrorCallback][{}][{}][{}] {}", messageSource, messageType, messageSeverity, message) << std::endl;
    else
        std::cout << std::format("[GLErrorCallback][{}][{}][{}] {}", messageSource, messageType, messageSeverity, message) << std::endl;
}