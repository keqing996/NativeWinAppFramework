
#include <array>
#include <Infra/Windows/Window.h>
#include <Infra/Windows/Glad/Gl.h>

const char *vertexShaderSource ="#version 330 core\n"
                                "layout (location = 0) in vec3 aPos;\n"
                                "layout (location = 1) in vec3 aColor;\n"
                                "out vec3 ourColor;\n"
                                "void main()\n"
                                "{\n"
                                "   gl_Position = vec4(aPos, 1.0);\n"
                                "   ourColor = aColor;\n"
                                "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 ourColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(ourColor, 1.0f);\n"
                                   "}\n\0";

const std::array<float, 18> vertices = {
        // positions         // colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
};

int main()
{
    Infra::Window window(800, 600, "TestOpengl");
    window.CreateOpenGLContext();

    ::gladLoaderLoadGL();

    ::glViewport(0, 0, 800, 600);
    ::glEnable(GL_DEPTH);

    unsigned int vertexShader = ::glCreateShader(GL_VERTEX_SHADER);
    ::glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    ::glCompileShader(vertexShader);

    unsigned int fragmentShader = ::glCreateShader(GL_FRAGMENT_SHADER);
    ::glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    ::glCompileShader(fragmentShader);

    unsigned int shaderProgram = ::glCreateProgram();
    ::glAttachShader(shaderProgram, vertexShader);
    ::glAttachShader(shaderProgram, fragmentShader);
    ::glLinkProgram(shaderProgram);

    ::glDeleteShader(vertexShader);
    ::glDeleteShader(fragmentShader);

    unsigned int vertexArray;
    ::glGenVertexArrays(1, &vertexArray);
    ::glBindVertexArray(vertexArray);

    unsigned int vertexBuffer;
    ::glGenBuffers(1, &vertexBuffer);
    ::glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    ::glBufferData(GL_ARRAY_BUFFER, vertices.size(), vertices.data(), GL_STATIC_DRAW);

    ::glEnableVertexAttribArray(0);
    ::glEnableVertexAttribArray(1);
    ::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    ::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    while (true)
    {

#pragma region [Window Loop]

        window.EventLoop();

        bool shouldWindowClose = false;
        while (window.HasEvent())
        {
            auto event = window.PopEvent();
            if (event.type == Infra::WindowEvent::Type::Close)
            {
                shouldWindowClose = true;
                break;
            }
        }

        if (shouldWindowClose)
            break;

#pragma endregion

#pragma region [Render Loop]

        ::glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ::glBindVertexArray(vertexArray);
        ::glUseProgram(shaderProgram);
        ::glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        ::glDrawArrays(GL_TRIANGLES, 0, 3);

#pragma endregion

        window.SwapBuffer();
    }

    ::glDeleteVertexArrays(1, &vertexArray);
    ::glDeleteBuffers(1, &vertexBuffer);
    ::glDeleteProgram(shaderProgram);

    return 0;
}