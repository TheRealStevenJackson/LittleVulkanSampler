#pragma once

#include "engine/graphics/renderer/VulkanContext.h"
#include "engine/graphics/renderer/VulkanBuffer.h"
#include "engine/graphics/renderer/VulkanImage.h"

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
    void setAlbedoMap(std::unique_ptr<VulkanImage> image);
    void setNormalMap(std::unique_ptr<VulkanImage> image);
    void setMetallicMap(std::unique_ptr<VulkanImage> image);
    void setRoughnessMap(std::unique_ptr<VulkanImage> image);
    void setAoMap(std::unique_ptr<VulkanImage> image);
    
    // Set material properties (used when textures are not available)
    void setAlbedo(float r, float g, float b, float a = 1.0f);
    void setMetallic(float metallic);
    void setRoughness(float roughness);
    void setAo(float ao);
    
    // Update the material UBO buffer
    void updateUBO();
    
    // Getters
    VulkanImage* albedoMap() const { return mAlbedoMap.get(); }
    VulkanImage* normalMap() const { return mNormalMap.get(); }
    VulkanImage* metallicMap() const { return mMetallicMap.get(); }
    VulkanImage* roughnessMap() const { return mRoughnessMap.get(); }
    VulkanImage* aoMap() const { return mAoMap.get(); }
    
    VulkanBuffer& materialUBO() { return *mMaterialUBO; }
    const VulkanBuffer& materialUBO() const { return *mMaterialUBO; }
    
    // Check if textures are being used
    bool hasAlbedoMap() const { return mAlbedoMap != nullptr; }
    bool hasNormalMap() const { return mNormalMap != nullptr; }
    bool hasMetallicMap() const { return mMetallicMap != nullptr; }
    bool hasRoughnessMap() const { return mRoughnessMap != nullptr; }
    bool hasAoMap() const { return mAoMap != nullptr; }

private:
    VulkanContext& mContext;
    
    // Texture maps (optional)
    std::unique_ptr<VulkanImage> mAlbedoMap;
    std::unique_ptr<VulkanImage> mNormalMap;
    std::unique_ptr<VulkanImage> mMetallicMap;
    std::unique_ptr<VulkanImage> mRoughnessMap;
    std::unique_ptr<VulkanImage> mAoMap;
    
    // Material properties (used when textures are not set)
    MaterialUBO mMaterialData;
    
    // Material UBO buffer
    std::unique_ptr<VulkanBuffer> mMaterialUBO;
};
