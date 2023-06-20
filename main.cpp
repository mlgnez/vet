// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include<iostream>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>
#include <list>
#include <vector>
#include <iterator>
#include "./file.cpp"
#include <optional>


using namespace std;

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

std::vector<File*> files = std::vector<File*>();

File* currentFile = nullptr;

static char currentText[99999];

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


string read_file(std::string path) {

    try {
        std::cout << path;
        std::ifstream inputFile(path);  // Replace "example.txt" with your file's name or path

         std::string line;

         std::string lines;

         if (inputFile.is_open()) {


             while (std::getline(inputFile, line)) {
                 lines += line + "\n";
             }

             inputFile.close();

             return lines;
         }
         else {
             std::cout << "Unable to open the file." << std::endl;
             return "File couldn't be open. Make sure its not open anywhere else.";
         }
    }
    catch (exception) {
        return "File couldn't be open. Make sure its not open anywhere else.";
    }
}

void add_file(std::string file_name, std::string file_path) {

    std::cout << "adding file " + file_path;

    File* newFile = new File(file_name, read_file(file_path), file_path);

    files.push_back(newFile);

}

std::string getFileNameFromDirectory(const std::string& directoryPath) {
    size_t lastSlashIndex = directoryPath.find_last_of("/\\");
    if (lastSlashIndex != std::string::npos) {
        return directoryPath.substr(lastSlashIndex + 1);
    }
    return directoryPath;
}

template <typename T>
std::optional<int> indexOf(std::vector<T> vec, T item) {
    auto iterator = find(vec.begin(), vec.end(), item);

    if (iterator != vec.end()) {
        int i = iterator - vec.begin();

        return std::make_optional(i);
    }

    return std::nullopt;
}

void removePreviousFileAndGoToElementBefore() { //yikes

    auto idx = indexOf<File*>(files, currentFile);

    if (idx.has_value()) {

        files.erase(files.begin() + idx.value_or(0));

        delete(currentFile);

        currentFile = nullptr;
    }

    

}


// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"vet", WS_OVERLAPPEDWINDOW, 100, 100, 1400, 1000, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //dictionary["main.py"] = "C:/Users/njv10/Downloads/write_vet_test/main.py";

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //--------------------CODE WINDOW--------------------//

        bool my_tool_active2 = true;

        std::string menu_action = "";

        ImGui::Begin("Code", &my_tool_active2, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(1400, 1000));

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open..", "Ctrl+O")) {
                    menu_action = "open_new_file";
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    menu_action = "save_current_file";
                }
                if (ImGui::MenuItem("Close", "Ctrl+W")) {
                    menu_action = "close_current_file";
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        //--------------------POPUP WINDOW--------------------//

        if (menu_action == "open_new_file") {
            ImGui::OpenPopup("open_new_file");
        }

        if (menu_action == "save_current_file") {
            ImGui::OpenPopup("save_current_file");
        }

        if (menu_action == "close_current_file") {
            ImGui::OpenPopup("close_current_file");
        }

        if (ImGui::BeginPopup("open_new_file", ImGuiWindowFlags_MenuBar))
        {
            static char dir[99999];

            ImGui::Text("Input the directory to the file you'd like to edit below.");

            ImGui::InputText("Directory:", dir, IM_ARRAYSIZE(dir));

            if (ImGui::Button("Open")) {

                if (dir != "") {
                    std::string file_name = getFileNameFromDirectory(dir);

                    add_file(file_name, dir);
                }
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("save_current_file", ImGuiWindowFlags_MenuBar))
        {
            if (currentFile != nullptr) {
                static char dir[500];
                strcpy_s(dir, currentFile->getPath().c_str());

                ImGui::Text("Input the directory you'd like to save the current file to.");

                ImGui::Text("Directory:");

                ImGui::InputText("Directory", dir, IM_ARRAYSIZE(dir));

                if (ImGui::Button("Save")) {

                    if (dir != "") {
                        currentFile->setPath(dir);
                        currentFile->saveToFile();
                    }
                }
            }
            else {
                ImGui::Text("Open a file before trying to close one.");
            }
            

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("close_current_file", ImGuiWindowFlags_MenuBar))
        {
            if (currentFile != nullptr) {
                ImGui::Text("Save the current file before closing?");

                if (ImGui::Button("Save File?")) {

                    currentFile->saveToFile();
                    removePreviousFileAndGoToElementBefore();
                    strcpy_s(currentText, "");
                }

                if (ImGui::Button("Don't Save")) {

                    removePreviousFileAndGoToElementBefore();
                    strcpy_s(currentText, "");
                }
            }
            else {
                ImGui::Text("Open a file before trying to close one.");
            }

            ImGui::EndPopup();
        }

        //--------------------POPUP WINDOW--------------------//

        //--------------------TAB WINDOW--------------------//

        ImGui::BeginTabBar("1");

        for (File* file : files) {
            if (file != nullptr) {
                if (currentFile != nullptr) {
                    if (currentFile->getPath() == file->getPath()) {
                        currentFile->compareContent(currentText);
                    }
                }
                else {
                    //file->compareContent(currentText);
                }
            }

            if (ImGui::TabItemButton(file->getName().c_str())){ //file->isSaved ? 0 : ImGuiTabItemFlags_UnsavedDocument) not adding this yet because it is very buggy when changing files, aka currenttext messes with content 
                if (currentFile != nullptr) {
                    currentFile->saveInMemory(currentText); //save old file stuff
                }

                //std::ifstream inputFile(file->getPath());  // Replace "example.txt" with your file's name or path
                //if (inputFile.is_open()) {
                //    
                //}
                //else {
                //    ImGui::Text("Unable to open file. Uh oh!");
                //}

                currentFile = file; //set new file
                strcpy_s(currentText, file->getContent().c_str());
                /*inputFile.close();*/

                
            }
        }

        //--------------------TAB WINDOW--------------------//

        ImGui::InputTextMultiline("", currentText, IM_ARRAYSIZE(currentText), ImVec2(1370, 900), ImGuiInputTextFlags_AllowTabInput);        

        ImGui::EndTabBar();

        ImGui::End();

        //--------------------CODE WINDOW--------------------//

        //if (ImGui::Begin("Code")) {

            //ImGui::SetNextWindowPos(ImVec2(0, 0));
            //ImGui::SetNextWindowSize(ImVec2(500, 1500));

            

            //ImGui::Text("Text");

            
            

        //}ImGui::End();


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
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
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    for (File* file : files) {
        delete(file);
    }

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
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
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
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
