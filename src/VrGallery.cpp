/*

    Copyright © 2018 Jonathan Hale <squareys@googlemail.com>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*/

#include <memory>

#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/StaticArray.h>
#include <Corrade/Interconnect/Receiver.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Arguments.h>

#include <Magnum/Magnum.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Geometry/Intersection.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Cylinder.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Text/Alignment.h>
#include <Magnum/Trade/MeshData3D.h>

#include <Magnum/OvrIntegration/Context.h>
#include <Magnum/OvrIntegration/Enums.h>
#include <Magnum/OvrIntegration/OvrIntegration.h>
#include <Magnum/OvrIntegration/Session.h>

#include <Magnum/Ui/Anchor.h>
#include <Magnum/Ui/Button.h>
#include <Magnum/Ui/Input.h>
#include <Magnum/Ui/Label.h>
#include <Magnum/Ui/Modal.h>
#include <Magnum/Ui/Plane.h>
#include <Magnum/Ui/UserInterface.h>

#include <Leap.h>

namespace Magnum {

using namespace Math::Literals;

namespace {

constexpr const Float WidgetHeight{36.0f};
constexpr const Float LabelHeight{24.0f};
constexpr const Vector2 ButtonSize{96.0f, WidgetHeight};
constexpr const Vector2 LabelSize{72.0f, LabelHeight};

struct BaseUiPlane: Ui::Plane {
    explicit BaseUiPlane(Ui::UserInterface& ui):
        Ui::Plane{ui, Ui::Snap::Top|Ui::Snap::Bottom|Ui::Snap::Left|Ui::Snap::Right, 0, 50, 640},
        buttonDefault{*this, {Ui::Snap::Top|Ui::Snap::Left, Range2D::fromSize(Vector2::yAxis(-36.0f), ButtonSize)},
            "Default", Ui::Style::Default},
        buttonPrimary{*this, {Ui::Snap::Right, buttonDefault, ButtonSize},
            "Primary", Ui::Style::Primary},
        buttonDanger{*this, {Ui::Snap::Right, buttonPrimary, ButtonSize},
            "Danger", Ui::Style::Danger},
        buttonSuccess{*this, {Ui::Snap::Right, buttonDanger, ButtonSize},
            "Success", Ui::Style::Success},
        buttonWarning{*this, {Ui::Snap::Right, buttonSuccess, ButtonSize},
            "Warning", Ui::Style::Warning},
        buttonFlat{*this, {Ui::Snap::Right, buttonWarning, {60.0f, WidgetHeight}},
            "Flat", Ui::Style::Flat},

        inputDefault{*this, {Ui::Snap::Top|Ui::Snap::Left,
            Range2D::fromSize(Vector2::yAxis(-282.0f), ButtonSize)},
            "Default", 8, Ui::Style::Default},
        inputDanger{*this, {Ui::Snap::Right, inputDefault, ButtonSize},
            "Danger", 8, Ui::Style::Danger},
        inputSuccess{*this, {Ui::Snap::Right, inputDanger, ButtonSize},
            "Success", 8, Ui::Style::Success},
        inputWarning{*this, {Ui::Snap::Right, inputSuccess, ButtonSize},
            "Warning", 8, Ui::Style::Warning},
        inputFlat{*this, {Ui::Snap::Right, inputWarning, ButtonSize},
            "Flat", 8, Ui::Style::Flat},

        modalDefault{*this, {Ui::Snap::Top|Ui::Snap::Left,
            Range2D::fromSize(Vector2::yAxis(-414.0f), ButtonSize)},
            "Default »"},
        modalDanger{*this, {Ui::Snap::Right, modalDefault, ButtonSize},
            "Danger »"},
        modalSuccess{*this, {Ui::Snap::Right, modalDanger, ButtonSize},
            "Success »"},
        modalWarning{*this, {Ui::Snap::Right, modalSuccess, ButtonSize},
            "Warning »"},
        modalInfo{*this, {Ui::Snap::Right, modalWarning, ButtonSize},
            "Info »"}
    {
        Ui::Button
            buttonDefaultDisabled{*this, {Ui::Snap::Bottom, buttonDefault, ButtonSize},
                "Default", Ui::Style::Default},
            buttonPrimaryDisabled{*this, {Ui::Snap::Bottom, buttonPrimary, ButtonSize},
                "Primary", Ui::Style::Primary},
            buttonDangerDisabled{*this, {Ui::Snap::Bottom, buttonDanger, ButtonSize},
                "Danger", Ui::Style::Danger},
            buttonSuccessDisabled{*this, {Ui::Snap::Bottom, buttonSuccess, ButtonSize},
                "Success", Ui::Style::Success},
            buttonWarningDisabled{*this, {Ui::Snap::Bottom, buttonWarning, ButtonSize},
                "Warning", Ui::Style::Warning},
            buttonFlatDisabled{*this, {Ui::Snap::Bottom, buttonFlat, ButtonSize},
                "Flat", Ui::Style::Flat};

        Ui::Widget::disable({
            buttonPrimaryDisabled,
            buttonDangerDisabled,
            buttonSuccessDisabled,
            buttonWarningDisabled,
            buttonFlatDisabled,
            buttonDefaultDisabled});

        Ui::Label
            labelDefault{*this, {Ui::Snap::Top|Ui::Snap::Left, Range2D::fromSize(Vector2::yAxis(-172.0f), LabelSize)},
                "Default", Text::Alignment::LineCenterIntegral, Ui::Style::Default},
            labelPrimary{*this, {Ui::Snap::Right, labelDefault, LabelSize},
                "Primary", Text::Alignment::LineCenterIntegral, Ui::Style::Primary},
            labelDanger{*this, {Ui::Snap::Right, labelPrimary, LabelSize},
                "Danger", Text::Alignment::LineCenterIntegral, Ui::Style::Danger},
            labelSuccess{*this, {Ui::Snap::Right, labelDanger, LabelSize},
                "Success", Text::Alignment::LineCenterIntegral, Ui::Style::Success},
            labelWarning{*this, {Ui::Snap::Right, labelSuccess, LabelSize},
                "Warning", Text::Alignment::LineCenterIntegral, Ui::Style::Warning},
            labelInfo{*this, {Ui::Snap::Right, labelWarning, LabelSize},
                "Info", Text::Alignment::LineCenterIntegral, Ui::Style::Info},
            labelDim{*this, {Ui::Snap::Right, labelInfo, LabelSize},
                "Dim", Text::Alignment::LineCenterIntegral, Ui::Style::Dim};

        Ui::Label
            labelDefaultDisabled{*this, {Ui::Snap::Bottom, labelDefault, LabelSize},
                "Default", Text::Alignment::LineCenterIntegral, Ui::Style::Default},
            labelPrimaryDisabled{*this, {Ui::Snap::Bottom, labelPrimary, LabelSize},
                "Primary", Text::Alignment::LineCenterIntegral, Ui::Style::Primary},
            labelDangerDisabled{*this, {Ui::Snap::Bottom, labelDanger, LabelSize},
                "Danger", Text::Alignment::LineCenterIntegral, Ui::Style::Danger},
            labelSuccessDisabled{*this, {Ui::Snap::Bottom, labelSuccess, LabelSize},
                "Success", Text::Alignment::LineCenterIntegral, Ui::Style::Success},
            labelWarningDisabled{*this, {Ui::Snap::Bottom, labelWarning, LabelSize},
                "Warning", Text::Alignment::LineCenterIntegral, Ui::Style::Warning},
            labelInfoDisabled{*this, {Ui::Snap::Bottom, labelInfo, LabelSize},
                "Info", Text::Alignment::LineCenterIntegral, Ui::Style::Info},
            labelDimDisabled{*this, {Ui::Snap::Bottom, labelDim, LabelSize},
                "Dim", Text::Alignment::LineCenterIntegral, Ui::Style::Dim};

        Ui::Widget::disable({
            labelDefaultDisabled,
            labelPrimaryDisabled,
            labelDangerDisabled,
            labelSuccessDisabled,
            labelWarningDisabled,
            labelInfoDisabled,
            labelDimDisabled});

        Ui::Input
            inputDefaultDisabled{*this, {Ui::Snap::Bottom, inputDefault, ButtonSize},
                "Default", 32, Ui::Style::Default},
            inputDangerDisabled{*this, {Ui::Snap::Bottom, inputDanger, ButtonSize},
                "Danger", 32, Ui::Style::Danger},
            inputSuccessDisabled{*this, {Ui::Snap::Bottom, inputSuccess, ButtonSize},
                "Success", 32, Ui::Style::Success},
            inputWarningDisabled{*this, {Ui::Snap::Bottom, inputWarning, ButtonSize},
                "Warning", 32, Ui::Style::Warning},
            inputFlatDisabled{*this, {Ui::Snap::Bottom, inputFlat, ButtonSize},
                "Flat", 32, Ui::Style::Flat};

        Ui::Widget::disable({
            inputDefaultDisabled,
            inputDangerDisabled,
            inputSuccessDisabled,
            inputWarningDisabled,
            inputFlatDisabled});

        Ui::Label{*this, {Ui::Snap::Top|Ui::Snap::Left|Ui::Snap::InsideX, buttonDefault, LabelSize},
            "Buttons", Text::Alignment::LineLeft, Ui::Style::Dim};
        Ui::Label{*this, {Ui::Snap::Top|Ui::Snap::Left|Ui::Snap::InsideX, labelDefault, LabelSize},
            "Labels", Text::Alignment::LineLeft, Ui::Style::Dim};
        Ui::Label{*this, {Ui::Snap::Top|Ui::Snap::Left|Ui::Snap::InsideX, inputDefault, LabelSize},
            "Inputs", Text::Alignment::LineLeft, Ui::Style::Dim};
        Ui::Label{*this, {Ui::Snap::Top|Ui::Snap::Left|Ui::Snap::InsideX, modalDefault, LabelSize},
            "Modals", Text::Alignment::LineLeft, Ui::Style::Dim};
    }

    Ui::Button buttonDefault,
        buttonPrimary,
        buttonDanger,
        buttonSuccess,
        buttonWarning,
        buttonFlat;

    Ui::Input inputDefault,
        inputDanger,
        inputSuccess,
        inputWarning,
        inputFlat;

    Ui::Button modalDefault,
        modalDanger,
        modalSuccess,
        modalWarning,
        modalInfo;
};

struct ModalUiPlane: Ui::Plane, Interconnect::Receiver {
    explicit ModalUiPlane(Ui::UserInterface& ui, Ui::Style style):
        Ui::Plane{ui, {{}, {320.0f, 240.0f}}, 2, 3, 128},
        message{*this, {{}, Range2D::fromSize(Vector2::yAxis(20.0f), {})},
            "This is a modal dialog.", Text::Alignment::LineCenterIntegral, style},
        close{*this, {Ui::Snap::Bottom|Ui::Snap::Right, ButtonSize},
            "Close", style == Ui::Style::Info ? Ui::Style::Default : style}
    {
        Ui::Modal{*this, Ui::Snap::Top|Ui::Snap::Bottom|Ui::Snap::Left|Ui::Snap::Right|Ui::Snap::NoSpaceX|Ui::Snap::NoSpaceY, style};

        Ui::Label{*this, {Ui::Snap::Left|Ui::Snap::Top, Range2D::fromSize(Vector2::xAxis(10.0f), {{}, WidgetHeight})},
            "Modal", Text::Alignment::LineLeft, style};
    }

    Ui::Label message;
    Ui::Button close;
};

}

class VrGallery: public Platform::Application {
    public:
        explicit VrGallery(const Arguments& arguments);

    private:
        void drawEvent() override;
        void keyPressEvent(KeyEvent& event) override;
        void drawBone(const Leap::Bone& bone, bool start, bool end, const Color4& color);

        OvrIntegration::Context _ovrContext;
        std::unique_ptr<OvrIntegration::Session> _session;

        /* Leap hand rendering */
        Containers::StaticArray<4, GL::Buffer> _buffers{Containers::DirectInit, NoCreate};
        GL::Mesh _cylinder{NoCreate};
        GL::Mesh _sphere{NoCreate};
        Shaders::Phong _shader{NoCreate};

        /* Oculus VR rendering */
        GL::Framebuffer _mirrorFramebuffer{NoCreate};
        GL::Texture2D* _mirrorTexture;

        OvrIntegration::LayerEyeFov* _layer;
        OvrIntegration::PerformanceHudMode _curPerfHudMode{
            OvrIntegration::PerformanceHudMode::Off};

        /* Per eye view members */
        GL::Texture2D _depth[2]{GL::Texture2D{NoCreate},
                                GL::Texture2D{NoCreate}};
        std::unique_ptr<OvrIntegration::TextureSwapChain> _textureSwapChain[2];
        GL::Framebuffer _framebuffer[2]{GL::Framebuffer{NoCreate},
                                        GL::Framebuffer{NoCreate}};
        Matrix4 _projectionMatrix[2];

        /* Ui */
        Containers::Optional<Ui::UserInterface> _ui;
        Containers::Optional<BaseUiPlane> _baseUiPlane;
        Containers::Optional<ModalUiPlane> _defaultModalUiPlane,
            _dangerModalUiPlane,
            _successModalUiPlane,
            _warningModalUiPlane,
            _infoModalUiPlane;

        /* Leap Motion input */
        Leap::Controller _controller;
        bool _isPressed[2]{false, false};
};

VrGallery::VrGallery(const Arguments& arguments): Platform::Application(arguments, NoCreate) {
    /* Connect to an active Oculus session */
    _session = _ovrContext.createSession();

    if(!_session) {
        Error() << "No HMD connected.";
        exit();
        return;
    }

    /* Get the HMD display resolution */
    const Vector2i resolution = _session->resolution()/2;

    /* Create a context with the HMD display resolution */
    Configuration conf;
    conf.setTitle("Magnum VR UI Gallery")
        .setSize(resolution)
        .setSampleCount(16)
        .setSRGBCapable(true);
    if(!tryCreateContext(conf))
        createContext(conf.setSampleCount(0));

    /* The oculus sdk compositor does some "magic" to reduce latency. For
       that to work, VSync needs to be turned off. */
    if(!setSwapInterval(0))
        Error() << "Could not turn off VSync.";

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    // FIXME: Magnum::Ui does not support sRGB yet
    // GL::Renderer::enable(GL::Renderer::Feature::FramebufferSRGB);

    _session->configureRendering();

    /* Setup mirroring of oculus sdk compositor results to a texture which can
       later be blitted onto the default framebuffer */
    _mirrorTexture = &_session->createMirrorTexture(resolution, OvrIntegration::MirrorOption::LeftEyeOnly);
    _mirrorFramebuffer = GL::Framebuffer(Range2Di::fromSize({}, resolution));
    _mirrorFramebuffer.attachTexture(GL::Framebuffer::ColorAttachment(0), *_mirrorTexture, 0)
                      .mapForRead(GL::Framebuffer::ColorAttachment(0));

    /* Setup compositor layers */
    _layer = &_ovrContext.compositor().addLayerEyeFov();
    _layer->setFov(*_session.get())
           .setHighQuality(true);

    /* Setup per-eye views */
    for(Int eye: {0, 1}) {
        _projectionMatrix[eye] = _session->projectionMatrix(eye, 0.001f, 25.0f);

        const Vector2i textureSize = _session->fovTextureSize(eye);
        _textureSwapChain[eye] =_session->createTextureSwapChain(textureSize);

        /* Create the framebuffer which will be used to render to the current
           texture of the texture set later. */
        _framebuffer[eye] = GL::Framebuffer{{{}, textureSize}};
        _framebuffer[eye].mapForDraw(GL::Framebuffer::ColorAttachment(0));

        /* Setup depth attachment */
        _depth[eye] = GL::Texture2D{};
        _depth[eye].setMinificationFilter(GL::SamplerFilter::Linear)
                   .setWrapping(GL::SamplerWrapping::ClampToEdge)
                   .setStorage(1, GL::TextureFormat::DepthComponent32F, textureSize);

        _layer->setColorTexture(eye, *_textureSwapChain[eye])
               .setViewport(eye, {{}, textureSize});
    }

    /* Ui setup */

    /* Enable blending with premultiplied alpha for Ui rendering */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);

    Ui::StyleConfiguration style = Ui::mcssDarkStyleConfiguration();
    GL::Renderer::setClearColor(0x22272e_rgbf);

    /* Create the UI. It should get at least some screen space, but also
       shouldn't be tucked away into a small corner on ultra-dense displays. */
    _ui.emplace(Vector2{1024.0f, 1024.0f}, Vector2i(1024, 1024), style, "»");

    /* Create base UI plane */
    _baseUiPlane.emplace(*_ui);

    /* Create modals */
    _defaultModalUiPlane.emplace(*_ui, Ui::Style::Default);
    _dangerModalUiPlane.emplace(*_ui, Ui::Style::Danger);
    _successModalUiPlane.emplace(*_ui, Ui::Style::Success);
    _warningModalUiPlane.emplace(*_ui, Ui::Style::Warning);
    _infoModalUiPlane.emplace(*_ui, Ui::Style::Info);
    Interconnect::connect(_baseUiPlane->modalDefault, &Ui::Button::tapped, *_defaultModalUiPlane, &Ui::Plane::activate);
    Interconnect::connect(_baseUiPlane->modalDanger, &Ui::Button::tapped, *_dangerModalUiPlane, &Ui::Plane::activate);
    Interconnect::connect(_baseUiPlane->modalSuccess, &Ui::Button::tapped, *_successModalUiPlane, &Ui::Plane::activate);
    Interconnect::connect(_baseUiPlane->modalWarning, &Ui::Button::tapped, *_warningModalUiPlane, &Ui::Plane::activate);
    Interconnect::connect(_baseUiPlane->modalInfo, &Ui::Button::tapped, *_infoModalUiPlane, &Ui::Plane::activate);

    Interconnect::connect(_defaultModalUiPlane->close, &Ui::Button::tapped, *_defaultModalUiPlane, &Ui::Plane::hide);
    Interconnect::connect(_dangerModalUiPlane->close, &Ui::Button::tapped, *_dangerModalUiPlane, &Ui::Plane::hide);
    Interconnect::connect(_successModalUiPlane->close, &Ui::Button::tapped, *_successModalUiPlane, &Ui::Plane::hide);
    Interconnect::connect(_warningModalUiPlane->close, &Ui::Button::tapped, *_warningModalUiPlane, &Ui::Plane::hide);
    Interconnect::connect(_infoModalUiPlane->close, &Ui::Button::tapped, *_infoModalUiPlane, &Ui::Plane::hide);

    /* Leap Motion input setup */
    _controller.setPolicy(Leap::Controller::PolicyFlag::POLICY_OPTIMIZE_HMD);

    /* Leap Motion hands rendering */
    Containers::Array<char> indexData;
    MeshIndexType indexType;
    UnsignedInt indexStart, indexEnd;
    {
        /* Setup cylinder mesh */
        const Trade::MeshData3D cylinder = Primitives::cylinderSolid(2, 16, 0.5f);
        _buffers[0] = Buffer{};
        _buffers[0].setData(MeshTools::interleave(cylinder.positions(0), cylinder.normals(0)), GL::BufferUsage::StaticDraw);

        std::tie(indexData, indexType, indexStart, indexEnd) = MeshTools::compressIndices(cylinder.indices());
        _buffers[1] = Buffer{};
        _buffers[1].setData(indexData, GL::BufferUsage::StaticDraw);

        _cylinder = GL::Mesh{cylinder.primitive()};
        _cylinder.setCount(cylinder.indices().size())
                 .addVertexBuffer(_buffers[0], 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
                 .setIndexBuffer(_buffers[1], 0, indexType, indexStart, indexEnd);
    }
    {
        /* Setup sphere mesh */
        const Trade::MeshData3D sphere = Primitives::uvSphereSolid(16, 16);
        _buffers[2] = Buffer{};
        _buffers[2].setData(MeshTools::interleave(sphere.positions(0), sphere.normals(0)), GL::BufferUsage::StaticDraw);

        std::tie(indexData, indexType, indexStart, indexEnd) = MeshTools::compressIndices(sphere.indices());
        _buffers[3] = Buffer{};
        _buffers[3].setData(indexData, GL::BufferUsage::StaticDraw);

        _sphere = GL::Mesh{sphere.primitive()};
        _sphere.setCount(sphere.indices().size())
               .addVertexBuffer(_buffers[2], 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
               .setIndexBuffer(_buffers[3], 0, indexType, indexStart, indexEnd);
    }

    /* Setup shader */
    _shader = Shaders::Phong{};
    _shader.setSpecularColor(Color3(1.0f))
           .setShininess(20)
           .setLightPosition({0.0f, 5.0f, 5.0f});
}

void VrGallery::drawEvent() {
    Containers::Optional<Leap::Frame> frame;
    if(_controller.isConnected()) frame.emplace(_controller.frame());

    /* Get orientation and position of the hmd. */
    const std::array<DualQuaternion, 2> poses = _session->pollEyePoses().eyePoses();
    auto headPose = _session->headPoseState();
    const Matrix4 invertedHeadPose = headPose.pose().toMatrix();

    Vector3 indexTipPosition[2];

    /* Draw the scene for both eyes */
    for(Int eye: {0, 1}) {
        /* Switch to eye render target and bind render textures */
        _framebuffer[eye]
            .attachTexture(GL::Framebuffer::ColorAttachment(0), _textureSwapChain[eye]->activeTexture(), 0)
            .attachTexture(GL::Framebuffer::BufferAttachment::Depth, _depth[eye], 0)
            /* Clear with the standard grey so that at least that will be visible in
            case the scene is not correctly set up */
            .clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth)
            .bind();

        /* Render scene */
        const Matrix4 viewProjMatrix = _projectionMatrix[eye]*poses[eye].inverted().toMatrix();

        Renderer::setDepthFunction(Renderer::DepthFunction::Always);
        _ui->setViewProjectionMatrix(viewProjMatrix*Matrix4::translation({0.2f, -0.6f, -0.4f})*Matrix4::scaling(Vector3{0.5f}));
        _ui->draw();
        Renderer::setDepthFunction(Renderer::DepthFunction::Less);

        /* Render scene */
        if (frame) {
            /* Leap Motion bones are always relative to view */
            const Matrix4 toWorldSpace = invertedHeadPose*Matrix4::rotationX(-90.0_degf)*Matrix4::scaling(0.001f*Vector3{-1.0f, 1.0f, -1.0f});
            _shader.setProjectionMatrix(viewProjMatrix*toWorldSpace);

            for(const Leap::Hand& hand : frame.value().hands()) {
                const Color4 handColor = hand.isRight() ? Color4{1.0f, 0.0f, 0.0f, 1.0f} : Color4{0.0f, 1.0f, 1.0f, 1.0f};

                for(const Leap::Finger& finger : hand.fingers()) {
                    for(int b = 0; b < 4; ++b) {
                        /* Leave out first bones of ring and middle finger, looks better */
                        if(b == 0 && (finger.type() == Leap::Finger::Type::TYPE_MIDDLE || finger.type() == Leap::Finger::Type::TYPE_RING)) continue;

                        const auto& bone = finger.bone(Leap::Bone::Type(b));
                        /* Only draw end on last bones */
                        drawBone(bone, true, b == 3, handColor);
                    }
                }

                indexTipPosition[hand.isRight() ? 0 : 1] = toWorldSpace.transformPoint(Vector3::from(hand.fingers().fingerType(Leap::Finger::Type::TYPE_INDEX).frontmost().tipPosition().toFloatPointer()));
            }
        }

        /* Commit changes and use next texture in chain */
        _textureSwapChain[eye]->commit();

        /* Reasoning for the next two lines, taken from the Oculus SDK examples
           code: Without this, [during the next frame, this method] would bind a
           framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
           associated with COLOR_ATTACHMENT0 had been unlocked by calling
           wglDXUnlockObjectsNV(). */
        _framebuffer[eye].detach(GL::Framebuffer::ColorAttachment(0))
                         .detach(GL::Framebuffer::BufferAttachment::Depth);
    }

    /* Set the layers eye poses to the poses chached in the _hmd. */
    _layer->setRenderPoses(*_session.get());

    /* Let the libOVR sdk compositor do its magic! */
    _ovrContext.compositor().submitFrame(*_session.get());

    /* Blit mirror texture to default framebuffer */
    const Vector2i size = _mirrorTexture->imageSize(0);
    GL::Framebuffer::blit(_mirrorFramebuffer,
        GL::defaultFramebuffer,
        {{0, size.y()}, {size.x(), 0}},
        {{}, size},
        GL::FramebufferBlit::Color, GL::FramebufferBlitFilter::Nearest);

    swapBuffers();

    redraw();

    /* Handle tapping on the UI */
    for(const int hand: {0, 1}) {
        const Vector3& indexTip = indexTipPosition[hand];
        const Float tipRadius = 0.008f;

        const Vector3 screenSpace = indexTip - Vector3{0.2f, -0.6f, -0.4f};
        const Vector2i screenPos = Vector2i{
            Int((screenSpace.x() + 0.5f)*1024),
            Int((-screenSpace.y() + 0.5f)*1024)};

        if (hand == 0) {
            _baseUiPlane->inputDefault.setValue(std::to_string(screenPos.x()));
            _baseUiPlane->inputDanger.setValue(std::to_string(screenPos.y()));

            _baseUiPlane->inputSuccess.setValue(std::to_string(screenSpace.x()).substr(0, 6));
            _baseUiPlane->inputWarning.setValue(std::to_string(screenSpace.y()).substr(0, 6));
            _baseUiPlane->inputFlat.setValue(std::to_string(screenSpace.z()).substr(0, 6));
        }

        if(Math::abs(screenSpace.z()) < tipRadius) {
            if(!_isPressed[hand]) {
                _ui->handlePressEvent(screenPos);
                _isPressed[hand] = true;
            } else {
                _ui->handleMoveEvent(screenPos);
            }
        } else if(_isPressed[hand]) {
            _ui->handleReleaseEvent(screenPos);
            _isPressed[hand] = false;
        }
    }
}

void VrGallery::drawBone(const Leap::Bone& bone, bool start, bool end, const Color4& color) {
    const Vector3 boneCenter = Vector3::from(bone.center().toFloatPointer());
    const Vector3 boneDirection = Vector3::from(bone.direction().toFloatPointer());
    const Float boneLength = bone.length();

    /* Draw cylinder */
    {
        const Matrix4 rotX = Matrix4::rotationX(90.0_degf);
        const Matrix4 transformation = Matrix4::translation(boneCenter)*Matrix4::from(bone.basis().toArray4x4())*rotX*Matrix4::scaling({6.0f, boneLength, 6.0f});
        _shader.setDiffuseColor(Color3{1.0f, 1.0f, 1.0f})
               .setTransformationMatrix(transformation)
               .setNormalMatrix(transformation.rotationScaling());
        _cylinder.draw(_shader);
    }

    /* Draw sphere at start of the bone */
    if(start) {
        const Vector3 prevJoint = Vector3::from(bone.prevJoint().toFloatPointer());
        const Matrix4 transformation = Matrix4::translation(prevJoint)*Matrix4::scaling(Vector3{8.0f});
        _shader.setDiffuseColor(color)
               .setTransformationMatrix(transformation)
               .setNormalMatrix(transformation.rotationScaling());
        _sphere.draw(_shader);
    }

    /* Draw sphere at end of the bone */
    if(end) {
        const Vector3 nextJoint = Vector3::from(bone.nextJoint().toFloatPointer());
        const Matrix4 transformation = Matrix4::translation(nextJoint)*Matrix4::scaling(Vector3{8.0f});
        _shader.setDiffuseColor(color)
               .setTransformationMatrix(transformation)
               .setNormalMatrix(transformation.rotationScaling());
        _sphere.draw(_shader);
    }
}

void VrGallery::keyPressEvent(KeyEvent& event) {
    /* Toggle through the performance hud modes */
    if(event.key() == KeyEvent::Key::F11) {
        switch(_curPerfHudMode) {
            case OvrIntegration::PerformanceHudMode::Off:
                _curPerfHudMode = OvrIntegration::PerformanceHudMode::LatencyTiming;
                break;
            case OvrIntegration::PerformanceHudMode::LatencyTiming:
                _curPerfHudMode = OvrIntegration::PerformanceHudMode::AppRenderTiming;
                break;
            case OvrIntegration::PerformanceHudMode::AppRenderTiming:
                _curPerfHudMode = OvrIntegration::PerformanceHudMode::CompRenderTiming;
                break;
            case OvrIntegration::PerformanceHudMode::CompRenderTiming:
                _curPerfHudMode = OvrIntegration::PerformanceHudMode::PerfSummary;
                break;
            case OvrIntegration::PerformanceHudMode::PerfSummary:
                _curPerfHudMode = OvrIntegration::PerformanceHudMode::VersionInfo;
                break;
            case OvrIntegration::PerformanceHudMode::VersionInfo:
                _curPerfHudMode = OvrIntegration::PerformanceHudMode::Off;
                break;
        }

        _session->setPerformanceHudMode(_curPerfHudMode);

    /* Exit */
    } else if(event.key() == KeyEvent::Key::Esc) {
        exit();
    }
}

}

MAGNUM_APPLICATION_MAIN(Magnum::VrGallery)
