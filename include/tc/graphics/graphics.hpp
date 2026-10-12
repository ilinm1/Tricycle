#pragma once

#include <filesystem>
#include "tc/misc/mat3.hpp"
#include "buffer.hpp"
#include "layer.hpp"
#include "rectangle_packer.hpp"
#include "texture.hpp"

#define TCG_IMAGE_CHANNELS 4 //rgba, just to avoid magic numbers
#define TCG_BUFFER_SIZE (TCG_VERT_SIZE * 3 * 1000000) //68.6 Mbs, up to a million triangles
#define TCG_IMAGE_EXTS { ".png", ".jpeg", ".bmp" }
#define TCG_SSBO_BINDING 1

namespace Tc::Graphics
{
    //window methods

    void SetWindowName(std::string name);

    //sets size of the window's content area in screen coordinates (generally this size doesn't have to be the same as size of the window in pixels but it usually is)
    void SetWindowSize(int width, int height);

    //gets size of the window's framebuffer in pixels
    std::tuple<int, int> GetWindowSize();

    void SetWindowFullscreen(bool fullscreen);

    //input methods

    //returns cursor position IN PIXELS
    //to get cursor position in NDC/in-world coordinates use 'PointFromPixels'
    Vec2 GetCursorPos();

    //'key' is one of GLFW's key IDs
    bool IsKeyPressed(int key);

    //'button' is one of GLFW's button IDs
    bool IsMouseButtonPressed(int button);

    std::string GetClipboardContents();

    //opens a file picker window with the title 'title'
    //if the purpose of the picker is to select the file for writing then 'write' may be set, in which case various additional settings will be applied (ovewrite warnings, ignoring readonly files, etc.)
    //if user has chosen a valid file then method will return true and set 'path' reference to the path of the selected file
    bool OpenFilePicker(std::string title, bool write, std::filesystem::path& path);

    //layer methods

    void AddLayer(Layer* layerPtr);

    void RemoveLayer(Layer* layerPtr);

    void SetLayerHeight(Layer* layerPtr, unsigned int height);

    void ClearLayers();

    bool IsLayerOutOfView(Layer* layerPtr);

    //camera methods

    //in in-world meters
    void SetCameraPosition(Vec2 position);

    //in in-world meters
    void SetCameraSize(Vec2 size);

    //in radians
    void SetCameraRotation(float rotation);

    //1 is 100% of normal camera size
    void SetCameraScale(float zoom);

    //converts a point in NDC/world coordinates to pixels
    Vec2 PointToPixels(Vec2 point, bool inWorld);

    //converts a point in pixels to NDC/world coordinates
    Vec2 PointFromPixels(Vec2 point, bool inWorld);

    //unlike 'PointToPixels' doesn't account for camera's position, rotation, different coordinate centers, etc, only converting the actual dimensions of the object
    Vec2 SizeToPixels(Vec2 size, bool inWorld);

    //unlike 'PointFromPixels' doesn't account for camera's position, rotation, different coordinate centers, etc, only converting the actual dimensions of the object
    Vec2 SizeFromPixels(Vec2 size, bool inWorld);

    //texture methods

    //'GL_NEAREST' - no filtering, 'GL_LINEAR' - linear interpolation
    void SetTextureFilter(unsigned int minification, unsigned int magnification);

    //loads textures from the specified paths
    std::vector<Texture> LoadTextures(std::vector<std::filesystem::path> paths);

    //barebones BDF font loader
    BitmapFont& LoadBdfFont(std::filesystem::path path);

    //loads all the textures from the specified path (recursively)
    std::vector<Texture> LoadTexturesFromPath(std::filesystem::path path);

    //finds texture by path if it's already loaded or loads it if not
    Texture ResolveTexture(std::filesystem::path path);

    //finds font by path if it's already loaded or loads it if not
    BitmapFont& ResolveFont(std::filesystem::path path);

    //writes atlas as a .bmp image (for debugging purposes)
    void SaveAtlas(std::filesystem::path path);

    //init, update

    void Initialize(int windowWidth, int windowHeight, std::string windowName, bool fullscreen = false, bool resizable = true);

    //blocks the current thread
    void UpdateLoop();

    void Shutdown();

    //globals

    inline GLFWwindow* Window;

    //vertex array object
    inline unsigned int Vao;

    //vertex buffer object, vertex buffer copy, shader storage buffer object
    inline Buffer Vbo, VboCopy, Ssbo;

    //shader uniform handles
    inline unsigned int UniformNdcMatrix;

    //camera data
    inline Vec2 CameraPosition; //camera's center
    inline Vec2 CameraSize = Vec2(1); //two times the distance from the camera's center to it's x/y boundary
    inline float CameraRotation;
    inline float CameraScale = 1;
    inline bool ClippingEnabled = true; //if enabled, layers which are out of camera's view will not be drawn

    //coordinate transformation matrices
    inline Mat3 WorldToNDCMatrix;
    inline Mat3 NDCToWorldMatrix;
    inline Mat3 NDCToPixelMatrix;
    inline Mat3 PixelToNDCMatrix;

    //texture data
    inline unsigned int Atlas; //opengl texture id
    inline RectanglePacker AtlasPacker;
    inline unsigned int AtlasWidth, AtlasHeight;
    inline unsigned char* AtlasData;
    inline std::vector<Texture> Textures = { Texture {} }; //zero index is reserved as an invalid texture, so drawing commands will ignore it
    inline std::vector<BitmapFont> Fonts;
    inline std::vector<TextureDimensions> TextureDimensionsVector = { TextureDimensions {} }; //texture positions and sizes relative to atlas, storing them separately from other texture data since it must be sent to the fragment shader
    inline std::vector<unsigned int> TexturesToUpdate; //indices of newly added/moved textures which require their data to be resent to the GPU

    //layers
    inline unsigned int LastLayerId = 0; //for logging purpouses, so that each layer has a unique ID
    inline std::vector<Layer*> Layers;
}
