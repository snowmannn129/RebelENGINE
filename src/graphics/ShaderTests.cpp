#include <gtest/gtest.h>
#include "graphics/Shader.h"
#include "graphics/OpenGLShader.h"
#include "graphics/VulkanShader.h"
#include "graphics/GraphicsSystem.h"
#include <shaderc/shaderc.hpp>

namespace rebel::tests {

class ShaderTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize graphics system for testing
        if (!graphics::GraphicsSystem::getInstance().isInitialized()) {
            graphics::GraphicsSystem::getInstance().initialize();
        }
    }

    void TearDown() override {
        if (graphics::GraphicsSystem::getInstance().isInitialized()) {
            graphics::GraphicsSystem::getInstance().shutdown();
        }
    }

    // Test shader sources
    const std::string vertexShaderSource = R"(
        #version 450
        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec2 aTexCoord;
        
        layout(location = 0) out vec2 vTexCoord;
        
        layout(binding = 0) uniform UniformBuffer {
            mat4 model;
            mat4 view;
            mat4 projection;
        } ubo;
        
        void main() {
            vTexCoord = aTexCoord;
            gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPosition, 1.0);
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 450
        layout(location = 0) in vec2 vTexCoord;
        layout(location = 0) out vec4 fragColor;
        
        layout(binding = 1) uniform sampler2D texSampler;
        
        void main() {
            fragColor = texture(texSampler, vTexCoord);
        }
    )";
};

TEST_F(ShaderTests, ShaderCreationTest) {
    // Test vertex shader creation
    auto vertexShader = graphics::Shader::Create(
        "TestVertexShader",
        graphics::ShaderType::Vertex,
        vertexShaderSource
    );
    ASSERT_NE(vertexShader, nullptr);
    EXPECT_TRUE(vertexShader->isCompiled());
    EXPECT_EQ(vertexShader->getType(), graphics::ShaderType::Vertex);

    // Test fragment shader creation
    auto fragmentShader = graphics::Shader::Create(
        "TestFragmentShader",
        graphics::ShaderType::Fragment,
        fragmentShaderSource
    );
    ASSERT_NE(fragmentShader, nullptr);
    EXPECT_TRUE(fragmentShader->isCompiled());
    EXPECT_EQ(fragmentShader->getType(), graphics::ShaderType::Fragment);
}

TEST_F(ShaderTests, ShaderProgramTest) {
    auto vertexShader = graphics::Shader::Create(
        "TestVertexShader",
        graphics::ShaderType::Vertex,
        vertexShaderSource
    );
    auto fragmentShader = graphics::Shader::Create(
        "TestFragmentShader",
        graphics::ShaderType::Fragment,
        fragmentShaderSource
    );

    auto program = graphics::ShaderProgram::Create("TestProgram");
    ASSERT_NE(program, nullptr);

    // Test shader attachment and linking
    EXPECT_NO_THROW(program->attach(vertexShader));
    EXPECT_NO_THROW(program->attach(fragmentShader));
    EXPECT_TRUE(program->link());
    EXPECT_TRUE(program->isLinked());
}

TEST_F(ShaderTests, UniformTest) {
    auto program = graphics::ShaderProgram::Create("UniformTestProgram");
    ASSERT_NE(program, nullptr);

    // Create and attach shaders
    auto vertexShader = graphics::Shader::Create(
        "UniformTestVS",
        graphics::ShaderType::Vertex,
        vertexShaderSource
    );
    auto fragmentShader = graphics::Shader::Create(
        "UniformTestFS",
        graphics::ShaderType::Fragment,
        fragmentShaderSource
    );

    program->attach(vertexShader);
    program->attach(fragmentShader);
    EXPECT_TRUE(program->link());

    // Test uniform setters
    float matrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    EXPECT_NO_THROW({
        program->bind();
        program->setMat4("ubo.model", matrix);
        program->setMat4("ubo.view", matrix);
        program->setMat4("ubo.projection", matrix);
        program->unbind();
    });
}

TEST_F(ShaderTests, SPIRVTest) {
    // Create a shaderc compiler instance
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Set compilation options
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

    // Compile vertex shader to SPIR-V
    auto vertexResult = compiler.CompileGlslToSpv(
        vertexShaderSource,
        shaderc_vertex_shader,
        "test.vert",
        options
    );

    ASSERT_EQ(vertexResult.GetCompilationStatus(), shaderc_compilation_status_success)
        << "SPIR-V compilation failed: " << vertexResult.GetErrorMessage();

    // Convert to vector and create shader
    std::vector<uint32_t> spirv(vertexResult.cbegin(), vertexResult.cend());
    auto shader = graphics::Shader::CreateFromSPIRV(
        "SPIRVTestShader",
        graphics::ShaderType::Vertex,
        spirv
    );

    ASSERT_NE(shader, nullptr);
    EXPECT_TRUE(shader->isCompiled());
}

TEST_F(ShaderTests, ErrorHandlingTest) {
    // Test invalid shader source
    const std::string invalidSource = "invalid shader source";
    EXPECT_THROW(
        graphics::Shader::Create(
            "InvalidShader",
            graphics::ShaderType::Vertex,
            invalidSource
        ),
        rebel::core::Error
    );

    // Test invalid shader type
    EXPECT_THROW(
        graphics::Shader::Create(
            "InvalidType",
            static_cast<graphics::ShaderType>(999),
            vertexShaderSource
        ),
        rebel::core::Error
    );

    // Test program linking without shaders
    auto program = graphics::ShaderProgram::Create("EmptyProgram");
    EXPECT_FALSE(program->link());
}

} // namespace rebel::tests
