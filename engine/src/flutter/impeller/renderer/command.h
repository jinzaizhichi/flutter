// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_COMMAND_H_
#define FLUTTER_IMPELLER_RENDERER_COMMAND_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "impeller/core/buffer_view.h"
#include "impeller/core/formats.h"
#include "impeller/core/sampler.h"
#include "impeller/core/shader_types.h"
#include "impeller/core/texture.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/pipeline.h"

namespace impeller {

template <class T>
class Resource {
 public:
  using ResourceType = T;
  ResourceType resource;

  Resource() {}

  Resource(const ShaderMetadata* metadata, ResourceType p_resource)
      : resource(p_resource), metadata_(metadata) {}

  const ShaderMetadata* GetMetadata() const {
    return dynamic_metadata_ ? dynamic_metadata_.get() : metadata_;
  }

  static Resource MakeDynamic(std::unique_ptr<ShaderMetadata> metadata,
                              ResourceType p_resource) {
    return Resource(std::move(metadata), p_resource);
  }

 private:
  Resource(std::unique_ptr<ShaderMetadata> metadata, ResourceType p_resource)
      : resource(p_resource), dynamic_metadata_(std::move(metadata)) {}

  // Static shader metadata (typically generated by ImpellerC).
  const ShaderMetadata* metadata_ = nullptr;

  // Dynamically generated shader metadata.
  std::unique_ptr<const ShaderMetadata> dynamic_metadata_ = nullptr;
};

using BufferResource = Resource<BufferView>;
using TextureResource = Resource<std::shared_ptr<const Texture>>;

/// @brief combines the texture, sampler and sampler slot information.
struct TextureAndSampler {
  SampledImageSlot slot;
  ShaderStage stage;
  TextureResource texture;
  raw_ptr<const Sampler> sampler;
};

//------------------------------------------------------------------------------
/// @brief      An object used to specify work to the GPU along with references
///             to resources the GPU will used when doing said work.
///
///             To construct a valid command, follow these steps:
///             * Specify a valid pipeline.
///             * Specify vertex information via a call `BindVertices`
///             * Specify any stage bindings.
///             * (Optional) Specify a debug label.
///
///             Command can be created frequently and on demand. The resources
///             referenced in commands views into buffers managed by other
///             allocators and resource managers.
///
struct Command {
  //----------------------------------------------------------------------------
  /// The pipeline to use for this command.
  ///
  PipelineRef pipeline;

  //----------------------------------------------------------------------------
  /// The index buffer binding used by the vertex shader stage.
  BufferView index_buffer;

  /// An offset into render pass storage where bound buffers/texture metadata is
  /// stored.
  Range bound_buffers = Range{0, 0};
  Range bound_textures = Range{0, 0};

  //----------------------------------------------------------------------------
  /// An offset and range of vertex buffers bound for this draw call.
  ///
  /// The vertex buffers are stored on the render pass object.
  Range vertex_buffers;

  //----------------------------------------------------------------------------
  /// The viewport coordinates that the rasterizer linearly maps normalized
  /// device coordinates to.
  /// If unset, the viewport is the size of the render target with a zero
  /// origin, znear=0, and zfar=1.
  ///
  std::optional<Viewport> viewport;

  //----------------------------------------------------------------------------
  /// The scissor rect to use for clipping writes to the render target. The
  /// scissor rect must lie entirely within the render target.
  /// If unset, no scissor is applied.
  ///
  std::optional<IRect32> scissor;

#ifdef IMPELLER_DEBUG
  //----------------------------------------------------------------------------
  /// The debugging label to use for the command.
  ///
  std::string label;
#endif  // IMPELLER_DEBUG

  //----------------------------------------------------------------------------
  /// The offset used when indexing into the vertex buffer.
  ///
  uint64_t base_vertex = 0u;

  //----------------------------------------------------------------------------
  /// The number of elements to draw. When only a vertex buffer is set, this is
  /// the vertex count. When an index buffer is set, this is the index count.
  uint32_t element_count = 0u;

  //----------------------------------------------------------------------------
  /// The number of instances of the given set of vertices to render. Not all
  /// backends support rendering more than one instance at a time.
  ///
  /// @warning      Setting this to more than one will limit the availability of
  ///               backends to use with this command.
  ///
  uint32_t instance_count = 1u;

  //----------------------------------------------------------------------------
  /// The reference value to use in stenciling operations. Stencil configuration
  /// is part of pipeline setup and can be read from the pipelines descriptor.
  ///
  /// @see         `Pipeline`
  /// @see         `PipelineDescriptor`
  ///
  uint32_t stencil_reference = 0u;

  //----------------------------------------------------------------------------
  /// The type of indices in the index buffer. The indices must be tightly
  /// packed in the index buffer.
  IndexType index_type = IndexType::kUnknown;

  bool IsValid() const { return pipeline && pipeline->IsValid(); }
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_COMMAND_H_
