#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "image.h"  //写了一个用于加载图片的函数，同时为了不和taichi.h起冲突，专门写为一个文件

#define TC_IMAGE_IO
#include <taichi.h>
using namespace taichi;

const int n = 80 /*grid resolution (cells)*/;
const real dt = 1e-4_f, dx = 1.0_f / n, inv_dx = 1.0_f / dx;
auto particle_mass = 1.0_f, vol = 1.0_f;
auto hardening = 10.0_f, E = 1e4_f, nu = 0.2_f;
real mu_0 = E / (2 * (1 + nu)), lambda_0 = E * nu / ((1 + nu) * (1 - 2 * nu));
using Vec = Vector2; using Mat = Matrix2; bool plastic = true;

struct Particle {
    Vec x, v; Mat F, C; real Jp; int c/*color*/;
    int ptype/*0: fluid   1: jelly   2: snow   3:rock*/;
    Particle(Vec x, int c, Vec v = Vec(0), int ptype = 2) : x(x), v(v), F(1), C(0), Jp(1), c(c), ptype(ptype) {}
};

std::vector<Particle> particles;
Vector3 grid[n + 1][n + 1];

void advance(real dt) {
    std::memset(grid, 0, sizeof(grid));                              // Reset grid
    for (auto& p : particles) {                                             // P2G
        Vector2i base_coord = (p.x * inv_dx - Vec(0.5_f)).cast<int>();//element-wise floor
        Vec fx = p.x * inv_dx - base_coord.cast<real>();
        // Quadratic kernels  [http://mpm.graphics   Eqn. 123, with x=fx, fx-1,fx-2]
        Vec w[3]{ Vec(0.5) * sqr(Vec(1.5) - fx), Vec(0.75) - sqr(fx - Vec(1.0)),
                 Vec(0.5) * sqr(fx - Vec(0.5)) };

        auto e = std::exp(hardening * (1.0_f - p.Jp));
        if (p.ptype == 1 || p.ptype == 3) e = 0.3;
        auto mu = mu_0 * e, lambda = lambda_0 * e;
        if (p.ptype == 0) mu = 0;

        real J = determinant(p.F);         //                         Current volume
        Mat r, s; polar_decomp(p.F, r, s); //Polar decomp. for fixed corotated model
        auto stress =                           // Cauchy stress times dt and inv_dx
            -4 * inv_dx * inv_dx * dt * vol * (2 * mu * (p.F - r) * transposed(p.F) + lambda * (J - 1) * J);
        auto affine = stress + particle_mass * p.C;
        for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) { // Scatter to grid
            auto dpos = (Vec(i, j) - fx) * dx;
            Vector3 mv(p.v * particle_mass, particle_mass); //translational momentum
            grid[base_coord.x + i][base_coord.y + j] +=
                w[i].x * w[j].y * (mv + Vector3(affine * dpos, 0));
        }
    }
    for (int i = 0; i <= n; i++) for (int j = 0; j <= n; j++) { //For all grid nodes
        auto& g = grid[i][j];
        if (g[2] > 0) {                                // No need for epsilon here
            g /= g[2];                                   //        Normalize by mass
            g += dt * Vector3(0, -200, 0);               //                  Gravity
            real boundary = 0.05, x = (real)i / n, y = real(j) / n; //boundary thick.,node coord
            if (x < boundary || x > 1 - boundary || y > 1 - boundary) g = Vector3(0); //Sticky
            if (y < boundary) g[1] = std::max(0.0_f, g[1]);             //"Separate"
        }
    }
    for (auto& p : particles) {                                // Grid to particle
        Vector2i base_coord = (p.x * inv_dx - Vec(0.5_f)).cast<int>();//element-wise floor
        Vec fx = p.x * inv_dx - base_coord.cast<real>();
        Vec w[3]{ Vec(0.5) * sqr(Vec(1.5) - fx), Vec(0.75) - sqr(fx - Vec(1.0)),
                 Vec(0.5) * sqr(fx - Vec(0.5)) };
        p.C = Mat(0); p.v = Vec(0);
        for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
            auto dpos = (Vec(i, j) - fx),
                grid_v = Vec(grid[base_coord.x + i][base_coord.y + j]);
            auto weight = w[i].x * w[j].y;
            p.v += weight * grid_v;                                      // Velocity
            p.C += 4 * inv_dx * Mat::outer_product(weight * grid_v, dpos); // APIC C
        }
        p.x += dt * p.v;                                                // Advection
        auto F = (Mat(1) + dt * p.C) * p.F;                      // MLS-MPM F-update

        if (p.ptype == 0) { p.F = Mat(1) * sqrt(determinant(F)); }
        else if (p.ptype == 1 || p.ptype == 3) { p.F = F; }
        else if (p.ptype == 2) {
            Mat svd_u, sig, svd_v; svd(F, svd_u, sig, svd_v);
            for (int i = 0; i < 2 * int(plastic); i++)                // Snow Plasticity
                sig[i][i] = clamp(sig[i][i], 1.0_f - 2.5e-2_f, 1.0_f + 7.5e-3_f);
            real oldJ = determinant(F); F = svd_u * sig * transposed(svd_v);
            real Jp_new = clamp(p.Jp * oldJ / determinant(F), 0.6_f, 20.0_f);
            p.Jp = Jp_new; p.F = F;
        }

    }
}

void add_object(Vec center, int c, int ptype = 2) {
    for (int i = 0; i < 1000; i++)
        particles.push_back(Particle((Vec::rand() * 2.0_f - Vec(1)) * 0.08_f + center, c, Vec(0.0), ptype));
}
void add_object_rectangle(Vec v1, Vec v2, int c, int num = 500, Vec velocity = Vec(0.0_f), int ptype = 0)
{
    Vec box_min(min(v1.x, v2.x), min(v1.y, v2.y)), box_max(max(v1.x, v2.x), max(v1.y, v2.y));
    int i = 0;
    while (i < num) {
        auto pos = Vec::rand();
        if (pos.x > box_min.x && pos.y > box_min.y && pos.x < box_max.x && pos.y < box_max.y) {
            particles.push_back(Particle(pos, c, velocity, ptype));
            i++;
        }
    }
}
void add_jet() {
    add_object_rectangle(Vec(0.05, 0.85), Vec(0.06, 0.86), 0x87CEFA, 1, Vec(7.0, 0.0), 0);
}
void add_sand() {
    int i = 0;
    int num = 3000;
    while (i < num) {
        auto pos = Vec::rand();
        pos[0] = pos[0] * 0.9 + 0.05;
        pos[1] = pos[1] * 0.2 + 0.25;
        if ((pos[0] - 0.5) * (pos[0] - 0.5) + (pos[1] - 0.45) * (pos[1] - 0.45) > 0.01)
        {
            particles.push_back(Particle(pos, 0xFFA500, Vec(0.0), 1));
            i++;
        }
    }
    add_object_rectangle(Vec(0.05, 0.15), Vec(0.35, 0.25), 0xFFA500, 600, Vec(0.0), 1);
    add_object_rectangle(Vec(0.05, 0.05), Vec(0.53, 0.15), 0xFFA500, 1200, Vec(0.0), 1);
}
void add_rock()
{
    add_object_rectangle(Vec(0.35, 0.15), Vec(0.95, 0.25), 0xDEB887, 3000, Vec(0.0), 3);
    add_object_rectangle(Vec(0.53, 0.05), Vec(0.95, 0.15), 0xDEB887, 1500, Vec(0.0), 3);
}

int water = 0;
void init_Scene()
{
    particles.clear();
    water = 0;
    add_sand();
    add_rock();
}

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX11 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // load our texture
    int empty_width = 0;
    int empty_height = 0;
    int inflow_width = 0;
    int inflow_height = 0;
    int full_width = 0;
    int full_height = 0;
    ID3D11ShaderResourceView* empty_texture = NULL;
    ID3D11ShaderResourceView* inflow_texture = NULL;
    ID3D11ShaderResourceView* full_texture = NULL;
    bool empty_ret = LoadTextureFromFile("empty.jpg", &empty_texture, &empty_width, &empty_height);
    bool inflow_ret = LoadTextureFromFile("inflow.jpg", &inflow_texture, &inflow_width, &inflow_height);
    bool full_ret = LoadTextureFromFile("full.jpg", &full_texture, &full_width, &full_height);
    IM_ASSERT(empty_ret);
    IM_ASSERT(inflow_ret);
    IM_ASSERT(full_ret);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_game_window = false;
    init_Scene();

    // Main loop
    bool done = false; 
    int step = 0;  // 用于fountain
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)break;
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Game Control Window
        ImGui::Begin("Game Control Window");
        ImGui::Text("This is a game called :Where's my water ?");
        if (ImGui::Button("Start Game"))
            show_game_window = true;
        if (ImGui::Button("Quit Game"))
        {
            show_game_window = false;
            init_Scene();
            step = 0;
        }
        ImGui::Text("                ");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // 2. Game Window
        if (show_game_window)
        {
            float s = 400;//window size
            ImGui::Begin("Game Window");
            if (water < 100) {
                ImGui::Text("Baby Crocodile:Can you help me to find my water ? ");
                ImGui::Text("               @(>o<)@");
            }
            else {
                ImGui::Text("Baby Crocodile:I have enough water for a bath!");
                ImGui::Text("               Thank you! (*^o^*)");
            }
            
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImVec2(s, s);      // Resize canvas to what's available
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(0, 0, 0, 255));
            // 3--------4
            // |        |
            // 2---1 6--5
            ImVec2 rect1 = ImVec2(canvas_p0.x + 0.4925 * s, canvas_p0.y + s);
            ImVec2 rect2 = ImVec2(canvas_p0.x + 0.04 * s, canvas_p0.y + s);
            ImVec2 rect3 = ImVec2(canvas_p0.x + 0.04 * s, canvas_p0.y + 0.08 * s);
            ImVec2 rect4 = ImVec2(canvas_p0.x + 0.96 * s, canvas_p0.y + 0.08 * s);
            ImVec2 rect5 = ImVec2(canvas_p0.x + 0.96 * s, canvas_p0.y + s);
            ImVec2 rect6 = ImVec2(canvas_p0.x + 0.5145 * s, canvas_p0.y + s);
            draw_list->AddLine(rect1, rect2, IM_COL32(255, 255, 255, 255));
            draw_list->AddLine(rect2, rect3, IM_COL32(255, 255, 255, 255));
            draw_list->AddLine(rect3, rect4, IM_COL32(255, 255, 255, 255));
            draw_list->AddLine(rect4, rect5, IM_COL32(255, 255, 255, 255));
            draw_list->AddLine(rect5, rect6, IM_COL32(255, 255, 255, 255));

            ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

            //将鼠标点击处附近的sand都删除
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                float mx = io.MousePos.x;
                float my = io.MousePos.y;
                for (int i = 0; i < particles.size(); i++)
                {
                    float px = s * particles[i].x[0] + canvas_p0.x;
                    float py = 1.04 * s - s * particles[i].x[1] + canvas_p0.y;
                    if ((px - mx) * (px - mx) + (py - my) * (py - my) < s * s / 1000 && particles[i].ptype == 1)
                    {
                        particles[i] = particles.back();
                        particles.pop_back();
                    }
                }
            }

            //将所用靠近水管处的water都删除
            for (int i = 0; i < particles.size(); i++)
            {
                if (particles[i].ptype == 0 && (particles[i].x[0] - 0.5) * (particles[i].x[0] - 0.5) + (particles[i].x[1] - 0.05) * (particles[i].x[1] - 0.05) < 0.01 * 0.01)
                {
                    particles[i] = particles.back();
                    particles.pop_back();
                    water++;
                }
            }
            
            advance(dt);

            //画出所有particle
            for (auto p : particles) {
                float px = s * p.x[0] + canvas_p0.x;
                float py = 1.04 * s - s * p.x[1] + canvas_p0.y;
                draw_list->AddCircle(ImVec2(px, py), 1, IM_COL32(p.c >> 16, (p.c >> 8) % 256, p.c % 256, 255), 100, 2.0f);
            }

            //建造fountain
            if (step < 5e3) add_jet();

            //water一旦有，加载inflow图片;water一旦超过100，加载full图片
            if (water == 0)
            {
                ImGui::Image((void*)empty_texture, ImVec2(empty_width, empty_height));
            }
            else
            {
                if (water < 100)
                {
                    ImGui::Image((void*)inflow_texture, ImVec2(inflow_width, inflow_height));
                }
                else
                {
                    ImGui::Image((void*)full_texture, ImVec2(full_width, full_height));
                }
            }
            ImGui::End();
        }
        step++;

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}



// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}
void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}
void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}
void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
