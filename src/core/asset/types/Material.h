#pragma once

#include "core/asset/types/Texture.h"
#include "platform/graphics/vulkan/VulkanContext.h"
#include "platform/graphics/vulkan/VulkanBuffer.h"
#include "platform/graphics/vulkan/VulkanShaderModule.h"
#include "platform/graphics/vulkan/VulkanDescriptorSet.h"

#include <memory>
#include <optional>
#include <string>

// Material UBO structure matching the PBR shader
struct MaterialUBO {
    alignas(16) float albedo[4];      // vec4
    alignas(4) float metallic;
    alignas(4) float roughness;
    alignas(4) float ao;
    alignas(4) int useAlbedoMap;
    alignas(4) int useNormalMap;
    alignas(4) int useMetallicMap;
    alignas(4) int useRoughnessMap;
    alignas(4) int useAoMap;
};

// Structure for AssetManager to pass material paths to Material constructor
struct MaterialPaths {
    std::string assetPath;      // Base path for the material asset
    std::string albedoPath;     // Path to albedo texture
    std::string normalPath;     // Path to normal map texture
    std::string metallicPath;    // Path to metallic texture
    std::string roughnessPath;  // Path to roughness texture
};

class Material {
public:
    Material(VulkanContext& context);
    Material(VulkanContext& context, const MaterialPaths& paths);
    
    // Delete copy constructor and assignment (RAII - non-copyable)
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    
    // Allow move semantics
    Material(Material&&) noexcept = default;
    Material& operator=(Material&&) noexcept = default;
    
    ~Material() = default;
    
    // Set texture maps (optional - if not set, uses uniform values)
    void setAlbedoMap(std::unique_ptr<Texture> texture);
    void setNormalMap(std::unique_ptr<Texture> texture);
    void setMetallicMap(std::unique_ptr<Texture> texture);
    void setRoughnessMap(std::unique_ptr<Texture> texture);
    void setAoMap(std::unique_ptr<Texture> texture);
    
    // Set material properties (used when textures are not available)
    void setAlbedo(float r, float g, float b, float a = 1.0f);
    void setMetallic(float metallic);
    void setRoughness(float roughness);
    void setAo(float ao);
    
    // Update the material UBO buffer
    void updateUBO();
    
    // Getters
    Texture* albedoMap() const { return mAlbedoMap.get(); }
    Texture* normalMap() const { return mNormalMap.get(); }
    Texture* metallicMap() const { return mMetallicMap.get(); }
    Texture* roughnessMap() const { return mRoughnessMap.get(); }
    Texture* aoMap() const { return mAoMap.get(); }
    
    VulkanBuffer& materialUBO() { return *mMaterialUBO; }
    const VulkanBuffer& materialUBO() const { return *mMaterialUBO; }
    
    // Descriptor sets for texture images
    VulkanDescriptorSet* albedoDescriptorSet() const { return mAlbedoDescriptorSet.get(); }
    VulkanDescriptorSet* normalDescriptorSet() const { return mNormalDescriptorSet.get(); }
    VulkanDescriptorSet* metallicDescriptorSet() const { return mMetallicDescriptorSet.get(); }
    VulkanDescriptorSet* roughnessDescriptorSet() const { return mRoughnessDescriptorSet.get(); }
    VulkanDescriptorSet* aoDescriptorSet() const { return mAoDescriptorSet.get(); }
    
    void setAlbedoDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet);
    void setNormalDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet);
    void setMetallicDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet);
    void setRoughnessDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet);
    void setAoDescriptorSet(std::unique_ptr<VulkanDescriptorSet> descriptorSet);
    
    VulkanShaderModule& vertexShader() { return *mVertexShader; }
    const VulkanShaderModule& vertexShader() const { return *mVertexShader; }
    VulkanShaderModule& fragmentShader() { return *mFragmentShader; }
    const VulkanShaderModule& fragmentShader() const { return *mFragmentShader; }
    
    // Check if textures are being used
    bool hasAlbedoMap() const { return mAlbedoMap != nullptr; }
    bool hasNormalMap() const { return mNormalMap != nullptr; }
    bool hasMetallicMap() const { return mMetallicMap != nullptr; }
    bool hasRoughnessMap() const { return mRoughnessMap != nullptr; }
    bool hasAoMap() const { return mAoMap != nullptr; }

private:
    VulkanContext& mContext;
    
    // Texture maps (optional)
    std::unique_ptr<Texture> mAlbedoMap;
    std::unique_ptr<Texture> mNormalMap;
    std::unique_ptr<Texture> mMetallicMap;
    std::unique_ptr<Texture> mRoughnessMap;
    std::unique_ptr<Texture> mAoMap;
    
    // Descriptor sets for texture images (optional)
    std::unique_ptr<VulkanDescriptorSet> mAlbedoDescriptorSet;
    std::unique_ptr<VulkanDescriptorSet> mNormalDescriptorSet;
    std::unique_ptr<VulkanDescriptorSet> mMetallicDescriptorSet;
    std::unique_ptr<VulkanDescriptorSet> mRoughnessDescriptorSet;
    std::unique_ptr<VulkanDescriptorSet> mAoDescriptorSet;
    
    // Material properties (used when textures are not set)
    MaterialUBO mMaterialData;
    
    // Material UBO buffer
    std::unique_ptr<VulkanBuffer> mMaterialUBO;
    
    // Default shaders
    std::unique_ptr<VulkanShaderModule> mVertexShader;
    std::unique_ptr<VulkanShaderModule> mFragmentShader;
};
