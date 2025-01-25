#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"

#include <iostream>
#include <ANN/ANN.h>	
//#include <ANNx.h>	
//#include <ANNperf.h>
using namespace Ubpa;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
gl::Texture2D loadTexture(char const* path);
gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources);

float dot(pointf3 p, normalf n);

// settings
unsigned int scr_width = 800;
unsigned int scr_height = 600;
float displacement_bias = 0.f;
float displacement_scale = 1.f;
float displacement_lambda = 1.0f;
float rotation_rate = 10.0f;
bool have_denoise = false;

// camera
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW8 - denoise", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    gl::Enable(gl::Capability::DepthTest);

    // build and compile our shader zprogram
    // ------------------------------------
    gl::Shader vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3_denoise.vert");
    gl::Shader fs(gl::ShaderType::FragmentShader, "../data/shaders/light.frag");
    gl::Program program(&vs, &fs);
    rgbf ambient{ 0.2f,0.2f,0.2f };
    program.SetTex("albedo_texture", 0);
    program.SetTex("displacementmap", 1);
    program.SetVecf3("point_light_pos", { 0,5,0 });
    program.SetVecf3("point_light_radiance", { 100,100,100 });
    program.SetVecf3("ambient_irradiance", ambient);
    program.SetFloat("roughness", 0.5f );
    program.SetFloat("metalness", 0.f);

    // load model
    // ------------------------------------------------------------------
    auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj", true);
    // world space positions of our cubes
    pointf3 instancePositions[] = {
        pointf3(0.0f,  0.0f,  0.0f),
        pointf3(2.0f,  5.0f, -15.0f),
        pointf3(-1.5f, -2.2f, -2.5f),
        pointf3(-3.8f, -2.0f, -12.3f),
        pointf3(2.4f, -0.4f, -3.5f),
        pointf3(-1.7f,  3.0f, -7.5f),
        pointf3(1.3f, -2.0f, -2.5f),
        pointf3(1.5f,  2.0f, -2.5f),
        pointf3(1.5f,  0.2f, -1.5f),
        pointf3(-1.3f,  1.0f, -1.5f)
    };

    // load and create a texture 
    // -------------------------
    gl::Texture2D spot_albedo = loadTexture("../data/textures/spot_albedo.png");

    gl::Texture2D displacementmap = genDisplacementmap(spot);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        gl::ClearColor({ ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit); // also clear the depth buffer now!

        program.SetVecf3("camera_pos", camera.Position);

        // bind textures on corresponding texture units
        program.Active(0, &spot_albedo);
        program.Active(1, &displacementmap);

        // pass projection matrix to shader (note that in this case it could change every frame)
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
        program.SetMatf4("projection", projection);

        // camera/view transformation
        program.SetMatf4("view", camera.GetViewMatrix());
        program.SetFloat("displacement_bias", displacement_bias);
        program.SetFloat("displacement_scale", displacement_scale);
        program.SetFloat("displacement_lambda", displacement_lambda);
        program.SetBool("have_denoise", have_denoise);

        // render spots
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            float angle = 20.0f * i + rotation_rate * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            program.SetMatf4("model", model);
            spot->va->Draw(&program);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    delete spot;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        have_denoise = 1;// !have_denoise;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        displacement_lambda += 0.01;
        std::cout << displacement_lambda << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        displacement_lambda -= 0.01;
        std::cout << displacement_lambda << std::endl;
    }
        
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    gl::Viewport({ 0, 0 }, width, height);
    scr_width = width;
    scr_height = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

gl::Texture2D loadTexture(char const* path)
{
    gl::Texture2D tex;
    tex.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat, gl::MinFilter::Linear, gl::MagFilter::Linear);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    gl::PixelDataFormat c2f[4] = {
        gl::PixelDataFormat::Red,
        gl::PixelDataFormat::Rg,
        gl::PixelDataFormat::Rgb,
        gl::PixelDataFormat::Rgba
    };
    gl::PixelDataInternalFormat c2if[4] = {
        gl::PixelDataInternalFormat::Red,
        gl::PixelDataInternalFormat::Rg,
        gl::PixelDataInternalFormat::Rgb,
        gl::PixelDataInternalFormat::Rgba
    };
    if (data)
    {
        tex.SetImage(0, c2if[nrChannels - 1], width, height, c2f[nrChannels - 1], gl::PixelDataType::UnsignedByte, data);
        tex.GenerateMipmap();
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return tex;
}

gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources)
{
    int DataSize = 1024 * 1024;
    float*  displacementData = new float[DataSize];
    memset(displacementData, 0, DataSize * sizeof(float));

    // TODO: HW8 - 1_denoise | genDisplacementmap
    
    // 1. set displacementData with resources's positions, indices, normals, ...
    size_t nV = resources->positions.size();//3225
    size_t nF = resources->indices.size() / 3;//5856

    std::vector<size_t> AdjacentNum;
    std::vector<float> dataii;
    std::vector<float> dataij;

    AdjacentNum.resize(nV, 0);
    dataii.resize(nV, 0);
    dataij.resize(nV, 0);

    for (int i = 0; i < nV; i++)
    {
        dataii[i] = dot(resources->positions[i], resources->normals[i]);
    }

    for (int i = 0; i < nF; i++)
    {
        std::vector<unsigned> ind;
        std::vector<pointf3> pos;
        std::vector<normalf> nor;
        for (int j = 0; j < 3; j++)
        {
            unsigned index = resources->indices[3 * i + j];
            ind.push_back(index);
            pos.push_back(resources->positions[index]);
            nor.push_back(resources->normals[index]);
            AdjacentNum[index] += 1;
        }
        for (int j = 0; j < 3; j++)
        {
            int j1 = (j + 1) % 3;
            int j2 = (j + 2) % 3;
            dataij[ind[j]] += (dot(pos[j1], nor[j]) + dot(pos[j2], nor[j])) / 2;
        }
    }
    std::vector<int>overlap3(nV, 0);
    for (int i = 0; i < nV; i++)
    {
        for (int j = i + 1; j < nV; j++)
        {
            for (int k = j + 1; k < nV; k++)
            {
                if ((resources->positions[i] - resources->positions[j]).norm() < 1e-7 && (resources->positions[j] - resources->positions[k]).norm() < 1e-7)
                {
                    float data_sum = dataij[i] + dataij[j] + dataij[k];
                    int AdjacentNum_sum = AdjacentNum[i] + AdjacentNum[j] + AdjacentNum[k];
                    dataij[i] = data_sum;
                    dataij[j] = data_sum;
                    dataij[k] = data_sum;
                    AdjacentNum[i] = AdjacentNum_sum;
                    AdjacentNum[j] = AdjacentNum_sum;
                    AdjacentNum[k] = AdjacentNum_sum;
                    overlap3[i] = 1;
                    overlap3[j] = 1;
                    overlap3[k] = 1;
                }
            }
        }
    }

    for (int i = 0; i < nV; i++)
    {
        if (overlap3[i] == 0)
        {
            for (int j = i + 1; j < nV; j++)
            {
                if ((resources->positions[i] - resources->positions[j]).norm() < 1e-7)
                {
                    float data_sum = dataij[i] + dataij[j];
                    int AdjacentNum_sum = AdjacentNum[i] + AdjacentNum[j];
                    dataij[i] = data_sum;
                    dataij[j] = data_sum;
                    AdjacentNum[i] = AdjacentNum_sum;
                    AdjacentNum[j] = AdjacentNum_sum;// std::cout << i << std::endl; std::cout << j << std::endl;
                }
            }
        }
    }
    std::vector<int>marked(DataSize, 0);
    for (int i = 0; i < nV; i++)
    {
        pointf2 tex = resources->texcoords[i];
        tex[0] = round(std::clamp(tex[0], 0.0f, 1.0f) * 1023);
        tex[1] = round(std::clamp(tex[1], 0.0f, 1.0f) * 1023);
        int num = static_cast<int>(tex[1] * 1024 + tex[0]);
        marked[num] = 1;
        displacementData[num] = dataii[i] - dataij[i] / AdjacentNum[i];

    }

     // 2. interpolate adjacent points

    int arrnum = 0;
    int dim = 2;
    constexpr size_t K = 1;

    ANNpointArray p = annAllocPts(DataSize, dim);

    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 1024; j++)
        {
            if (marked[j * 1024 + i] == 1)
            {
                p[arrnum][0] = i;
                p[arrnum][1] = j;
                arrnum++;
            }
        }
    }

    ANNbd_tree tree(p, arrnum, dim);
    ANNpoint queryPt = annAllocPt(2);

    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 1024; j++)
        {
            if (marked[j * 1024 + i] == 0)
            {
                ANNidx idxArr[K];
                ANNdist distArr[K];
                queryPt[0] = i;
                queryPt[1] = j;
                tree.annkSearch(queryPt, K, idxArr, distArr);
                int num = static_cast<int>(p[idxArr[0]][1] * 1024 + p[idxArr[0]][0]);
                displacementData[j * 1024 + i] = displacementData[num];
            }
        }
    }
    annDeallocPts(p);
    annDeallocPt(queryPt);

    // 3. change global variable: displacement_bias, displacement_scale, displacement_lambda
    float DataMax = displacementData[0];
    float DataMin = displacementData[0];
    for (int j = 1; j < DataSize; j++)
    {
        if (DataMax < displacementData[j])DataMax = displacementData[j];
        if (DataMin > displacementData[j])DataMin = displacementData[j];
    }
    for (int j = 0; j < DataSize; j++)
    {
        displacementData[j] = (displacementData[j] - DataMin) / (DataMax - DataMin);
    }
    displacement_scale = DataMax - DataMin;
    displacement_bias = DataMin;
    displacement_lambda = 0.9f;


    gl::Texture2D displacementmap;
    displacementmap.SetImage(0, gl::PixelDataInternalFormat::Red, 1024, 1024, gl::PixelDataFormat::Red, gl::PixelDataType::Float, displacementData);
    displacementmap.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
        gl::MinFilter::Linear, gl::MagFilter::Linear);
    stbi_uc* stbi_data = new stbi_uc[1024 * 1024];
    for (size_t i = 0; i < 1024 * 1024; i++)
        stbi_data[i] = static_cast<stbi_uc>(std::clamp(displacementData[i] * 255.f, 0.f, 255.f));
    stbi_write_png("../data/1_denoise_displacement_map.png", 1024, 1024, 1, stbi_data, 1024);
    delete[] stbi_data;
    delete[] displacementData;
    return displacementmap;
}

float dot(pointf3 p, normalf n)
{
    return p[0] * n[0] + p[1] * n[1] + p[2] * n[2];
}